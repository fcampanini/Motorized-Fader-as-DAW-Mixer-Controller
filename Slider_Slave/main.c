
/*
Copyright 2020 Filippo Campanini

CC BY-NC-SA License

http://creativecommons.org/licenses/by-nc-sa/4.0/

NOTICES
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#include <atmel_start.h>
#include <stdbool.h>
#include "Ring_Buffer/RingBuffer.h"

#define IO_DIRSET_A	PORT->Group[0].DIRSET.reg
#define IO_OUTSET_A	PORT->Group[0].OUTSET.reg
#define IO_OUTCLR_A	PORT->Group[0].OUTCLR.reg
#define IO_OUTTLG_A	PORT->Group[0].OUTTGL.reg

#define LED_PWR		PORT_PA05
#define LED_OPR		PORT_PA08
#define LED_TOUCH	PORT_PA09
#define LED_TEST	PORT_PA04

#define PWM_SPEED				4
#define PWM_FULL_SPEED			9

#define ADC_OFFSET				150
#define ADC_SAMPLE_RATE			1
#define ADC_DMA_BUFFER_SIZE		16

#define CHANNEL_N				0

/*----------------------------QTOUCH----------------------------*/
extern volatile uint8_t measurement_done_touch;
static uint8_t key_status = 0;
volatile bool isTouch;
volatile bool wasTouch;
static void touch_status_display(void);
/*--------------------------------------------------------------*/

/*----------------------------TCC0-------------------------------*/
static struct timer_task TIMER_ADC_task1, TIMER_ADC_task2, TIMER_ADC_task3;
static void TIMER_ADC_ini(void); // TC1 init
static void TIMER_ADC_task1_cb(const struct timer_task \
*const timer_task); // TCC0 task1 OVF handler
static void TIMER_ADC_task2_cb(const struct timer_task \
*const timer_task); // TCC0 task2 OVF handler
static void TIMER_ADC_task3_cb(const struct timer_task \
*const timer_task); // Reset Error state of UART. Done here for lack of free timers usable
/*--------------------------------------------------------------*/


/*----------------------------ADC DMA-------------------------------*/
volatile uint16_t adc_result;
volatile uint16_t old_adc_result;
uint16_t ADC_DMA_buffer[ADC_DMA_BUFFER_SIZE];
static void ADC_DMA_init(void);
static void ADC_DMA_complete_cb(const struct adc_dma_descriptor *const descr);
static void ADC_DMA_error_cb(const struct adc_dma_descriptor *const descr);
/*--------------------------------------------------------------*/

/*-------------------------------PWM----------------------------*/
volatile uint8_t up_speed;
volatile uint8_t down_speed;
volatile uint16_t pot_max_value;
volatile uint16_t pot_min_value;
volatile uint16_t reference_value;
static void TC2_init(void);
/*--------------------------------------------------------------*/

/*-------------------------------UART-----------------------------*/
struct io_descriptor *USART_MAIN_io;
volatile bool usart_is_tx_complete;
volatile bool usart_is_rx_complete;
volatile bool usart_is_error;
uint8_t usart_data_buffer[3];
uint8_t usart_ring_temp[3];
struct Buffer USART_ring_buffer = {{}, 0, 0};
static void USART_MAIN_load(void);
static void USART_MAIN_tx_complete_cb(const struct usart_async_descriptor *const io_descr);
static void USART_MAIN_rx_complete_cb(const struct usart_async_descriptor *const io_descr);
static void USART_MAIN_error_cb(const struct usart_async_descriptor *const io_descr);
/*--------------------------------------------------------------*/

/*-------------------------------MOTOR----------------------------*/
void move_motor_up(uint8_t speed);
void move_motor_down(uint8_t speed);
void break_motor(void);
void calibration(void);
/*--------------------------------------------------------------*/

volatile bool isOperating;

/*--------------------------------------------------------------------------------------MAIN Function and Infinite Loop----------------------------------------------------------------------------------*/

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	__disable_irq();

	atmel_start_init();
	TIMER_ADC_ini();
	ADC_DMA_init();
	TC2_init();
	USART_MAIN_load();

	NVIC->ISER[0] |= (1 << 14); // Enable the interrupt IRQ (14) for the TC2;

	PORT->Group[0].PINCFG[6].reg |= 0x01; // Set PA6 as Alternative MUX function
	PORT->Group[0].PMUX[3].reg = 0x04; // Set PA6 as WO[0] for TC2

	PORT->Group[0].PINCFG[7].reg |= 0x01; // Set PA7 as Alternative MUX function
	PORT->Group[0].PMUX[3].reg |= (0x40 & 0xF0); // Set PA7 as WO[1] for TC2

	PORT->Group[0].PINCFG[24].reg |= (1 << 2); // enable pull up resistor (as pr datasheet indication)
	PORT->Group[0].PINCFG[25].reg |= (1 << 2); // enable pull up resistor (as pr datasheet indication)

	__enable_irq();

	IO_DIRSET_A = LED_PWR; // Set direction for POWER LED
	IO_OUTSET_A = LED_PWR; // Set HIGH for POWER LED
	IO_DIRSET_A = LED_OPR; // Set direction for OPERATION LED
	IO_DIRSET_A = LED_TOUCH; // Set direction for TOUCH LED
	IO_DIRSET_A = LED_TEST; // Set direction for LED test

	down_speed = 0;
	up_speed = 0;
	reference_value = 11800;

	calibration();

	usart_is_tx_complete = true;
	usart_is_rx_complete = true;

	while (1)
	{

		//---------------------------------------------------------Data receive over UART---------------------------------
		uint8_t testBuffer;
		enum BufferStatus status;
		
		status = bufferRead(&USART_ring_buffer, &testBuffer);
		
		if( status == BUFFER_OK && (testBuffer & 0xF0) == 0xE0)
		{
			usart_ring_temp[0] = testBuffer;
			
			for(int i = 1; i < 3; i++)
			{
				//Read data from master
				bufferRead(&USART_ring_buffer, &usart_ring_temp[i]);
			}
			
			//Set position
			if(!isTouch)
			{
				reference_value = ((usart_ring_temp[2] << 7) | (usart_ring_temp[1] & 0x7F));
			}
		}
		
		// Move fader
		if((reference_value > (adc_result + ADC_OFFSET)) && (reference_value < pot_max_value) && !isTouch )
		{
			move_motor_up(PWM_SPEED);
			while((reference_value - ADC_OFFSET) > adc_result){isOperating = true;}
			break_motor();
			reference_value = adc_result;
		}
		
		if((reference_value < (adc_result - ADC_OFFSET)) && (reference_value > pot_min_value) && !isTouch)
		{
			move_motor_down(PWM_SPEED);
			while((reference_value + ADC_OFFSET) < adc_result){isOperating = true;}
			break_motor();
			reference_value = adc_result;
		}

		//--------------------------------------------------------Data send over the UART-----------------------------------------------

		touch_process();
		if (measurement_done_touch == 1)
		{
			touch_status_display();
		}
		
		if(isTouch)
		{
			if(!wasTouch)
			{
				wasTouch = true;
				
				usart_data_buffer[0] = 0x90;
				usart_data_buffer[1] = (0x18 + CHANNEL_N); //LSB
				usart_data_buffer[2] = 0x7F; //MSB
				io_write(USART_MAIN_io, usart_data_buffer, 3);
			}
			
			if(adc_result >= (old_adc_result + 60) || adc_result <= (old_adc_result - 60))
			{
				reference_value = adc_result;
				old_adc_result = reference_value;

				if (adc_result >= (pot_max_value - 20)) //compensate at top value
				{
					adc_result = 16383;
				}
				
				if(adc_result <= (pot_min_value + 20)) //compensate at bottom value
				{
					adc_result = 0;
				}
				
				usart_data_buffer[0] = (0xE0 + CHANNEL_N);
				usart_data_buffer[2] = ((adc_result >> 7) & 0x7F); //LSB
				usart_data_buffer[1] = (adc_result & 0x7F); //MSB
				io_write(USART_MAIN_io, usart_data_buffer, 3);
			}
			
		}
		else if(!isTouch)
		{
			if(wasTouch)
			{
				wasTouch = false;
				
				usart_data_buffer[0] = 0x90;
				usart_data_buffer[1] = (0x18 + CHANNEL_N); //LSB
				usart_data_buffer[2] = 0x00; //MSB
				io_write(USART_MAIN_io, usart_data_buffer, 3);
			}
		}
	}
}

/*--------------------------------------------------------------------------------------Initialization and Call-backs event Handlers----------------------------------------------------------------------------------------*/

static void TIMER_ADC_ini()
{
	TIMER_ADC_task1.interval = ADC_SAMPLE_RATE;
	TIMER_ADC_task1.cb       = TIMER_ADC_task1_cb;
	TIMER_ADC_task1.mode     = TIMER_TASK_REPEAT;

	TIMER_ADC_task2.interval = 50;
	TIMER_ADC_task2.cb       = TIMER_ADC_task2_cb;
	TIMER_ADC_task2.mode     = TIMER_TASK_REPEAT;
	
	TIMER_ADC_task3.interval = 5000;
	TIMER_ADC_task3.cb       = TIMER_ADC_task3_cb;
	TIMER_ADC_task3.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_ADC, &TIMER_ADC_task1);
	timer_add_task(&TIMER_ADC, &TIMER_ADC_task2);
	timer_add_task(&TIMER_ADC, &TIMER_ADC_task3);
	timer_start(&TIMER_ADC);
}

static void ADC_DMA_init(void)
{
	adc_dma_register_callback(&ADC_DMA, ADC_DMA_COMPLETE_CB, ADC_DMA_complete_cb);
	adc_dma_register_callback(&ADC_DMA, ADC_DMA_ERROR_CB, ADC_DMA_error_cb);
	adc_dma_enable_channel(&ADC_DMA, 0);
}

static void TC2_init()
{
	PM->APBCMASK.reg |= (1 <<  7); // Enable peripheral BUS for TC2

	GCLK->CLKCTRL.reg = 0;
	GCLK->CLKCTRL.reg = (1 << 14) | (0x04 << 8) | 0x12; // enable generic clock, select 4 as generic clock source and assign TC2 id

	TC2->COUNT8.CTRLA.reg |= (1 << 0); // Sofware reset and TC2 disable
	while((TC2->COUNT8.CTRLA.reg & 0x1) == 1){} // wait until TC2 is reset and disabled
	TC2->COUNT8.CTRLA.reg |= (1 << 2) | (1 << 6); // Set 8bit mode, NPWM
	TC2->COUNT8.PER.reg = 10; // 1.000.000/ 10 = 100KHz period
	TC2->COUNT8.CC[0].reg = 0;
	TC2->COUNT8.CC[1].reg = 0;
	TC2->COUNT8.INTENSET.reg = 0x01;
	TC2->COUNT8.CTRLA.reg |= (1 << 1); // enable the counter
}

static void USART_MAIN_load(void)
{
	usart_async_register_callback(&USART_MAIN, USART_ASYNC_TXC_CB, USART_MAIN_tx_complete_cb);
	usart_async_register_callback(&USART_MAIN, USART_ASYNC_RXC_CB, USART_MAIN_rx_complete_cb);
	usart_async_register_callback(&USART_MAIN, USART_ASYNC_ERROR_CB, USART_MAIN_error_cb);
	usart_async_get_io_descriptor(&USART_MAIN, &USART_MAIN_io);
	usart_async_enable(&USART_MAIN);
}

static void TIMER_ADC_task1_cb(const struct timer_task *const timer_task)
{
	adc_dma_read(&ADC_DMA, ADC_DMA_buffer, ADC_DMA_BUFFER_SIZE);
}

static void TIMER_ADC_task2_cb(const struct timer_task *const timer_task)
{
	if(isOperating && ((old_adc_result + 100) < adc_result ||  (old_adc_result - 100) > adc_result))
	{
		IO_OUTTLG_A = LED_OPR;
	}
	else
	{
		IO_OUTCLR_A = LED_OPR;
	}

	isOperating = false;
}

static void TIMER_ADC_task3_cb(const struct timer_task *const timer_task)
{
	usart_is_error = false;
	IO_OUTCLR_A = LED_TEST;
}

static void ADC_DMA_complete_cb(const struct adc_dma_descriptor *const descr)
{
	uint32_t tmp = 0;

	for( int n = 0; n < ADC_DMA_BUFFER_SIZE; n++)
	{
		tmp += ADC_DMA_buffer[n];
	}

	adc_result = tmp/ADC_DMA_BUFFER_SIZE;
}

static void ADC_DMA_error_cb(const struct adc_dma_descriptor *const descr)
{

}

void TC2_Handler()
{
	if(TC2->COUNT8.INTFLAG.reg & 0x01)
	{
		TC2->COUNT8.CC[0].reg = down_speed;
		TC2->COUNT8.CC[1].reg = up_speed;
		TC2->COUNT8.INTFLAG.reg = 1; // clear overflow flag
	}
}

static void USART_MAIN_tx_complete_cb(const struct usart_async_descriptor *const io_descr)
{
	usart_is_tx_complete = true;
}

static void USART_MAIN_rx_complete_cb(const struct usart_async_descriptor *const io_descr)
{
	usart_is_rx_complete = true;
	uint8_t tmp;
	io_read(USART_MAIN_io, &tmp, 1);
	bufferWrite(&USART_ring_buffer, tmp);
	delay_us(320);
}

static void USART_MAIN_error_cb(const struct usart_async_descriptor *const io_descr)
{
	usart_is_error = true;
	IO_OUTSET_A = LED_TEST;
}


/*--------------------------------------------------------------------------------------Other private functions--------------------------------------------------------------------------------------*/

void move_motor_up(uint8_t speed)
{
	down_speed = 0;
	up_speed = speed;
}

void move_motor_down(uint8_t speed)
{
	down_speed = speed;
	up_speed = 0;
}

void break_motor()
{
	down_speed = 0;
	up_speed = 0;
}

void calibration()
{
	move_motor_up(PWM_FULL_SPEED);
	delay_ms(500);
	pot_max_value = adc_result;

	move_motor_down(PWM_FULL_SPEED);
	delay_ms(500);
	pot_min_value = adc_result;

	break_motor();
}

void touch_status_display(void)
{
	key_status = get_sensor_state(0) & 0x80;

	if (0u != key_status)
	{
		// LED_ON
		IO_OUTSET_A = LED_TOUCH;
		isTouch = true;
		isOperating = true;
	}
	else
	{
		// LED_OFF
		IO_OUTCLR_A = LED_TOUCH;
		isTouch = false;
	}
}