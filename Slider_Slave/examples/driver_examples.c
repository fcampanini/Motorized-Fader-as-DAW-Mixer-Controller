/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

/*! The buffer size for ADC */
#define ADC_DMA_BUFFER_SIZE 16
static uint8_t ADC_DMA_buffer[ADC_DMA_BUFFER_SIZE];

static void convert_cb_ADC_DMA(const struct adc_dma_descriptor *const descr)
{
}

/**
 * Example of using ADC_DMA to generate waveform.
 */
void ADC_DMA_example(void)
{
	/* Enable ADC freerun mode in order to make example work */
	adc_dma_register_callback(&ADC_DMA, ADC_DMA_COMPLETE_CB, convert_cb_ADC_DMA);
	adc_dma_enable_channel(&ADC_DMA, 0);
	adc_dma_read(&ADC_DMA, ADC_DMA_buffer, ADC_DMA_BUFFER_SIZE);
}

/**
 * Example of using USART_MAIN to write "Hello World" using the IO abstraction.
 *
 * Since the driver is asynchronous we need to use statically allocated memory for string
 * because driver initiates transfer and then returns before the transmission is completed.
 *
 * Once transfer has been completed the tx_cb function will be called.
 */

static uint8_t example_USART_MAIN[12] = "Hello World!";

static void tx_cb_USART_MAIN(const struct usart_async_descriptor *const io_descr)
{
	/* Transfer completed */
}

void USART_MAIN_example(void)
{
	struct io_descriptor *io;

	usart_async_register_callback(&USART_MAIN, USART_ASYNC_TXC_CB, tx_cb_USART_MAIN);
	/*usart_async_register_callback(&USART_MAIN, USART_ASYNC_RXC_CB, rx_cb);
	usart_async_register_callback(&USART_MAIN, USART_ASYNC_ERROR_CB, err_cb);*/
	usart_async_get_io_descriptor(&USART_MAIN, &io);
	usart_async_enable(&USART_MAIN);

	io_write(io, example_USART_MAIN, 12);
}

static struct timer_task Timer_task1, Timer_task2;
/**
 * Example of using Timer.
 */
static void Timer_task1_cb(const struct timer_task *const timer_task)
{
}

static void Timer_task2_cb(const struct timer_task *const timer_task)
{
}

void Timer_example(void)
{
	Timer_task1.interval = 100;
	Timer_task1.cb       = Timer_task1_cb;
	Timer_task1.mode     = TIMER_TASK_REPEAT;
	Timer_task2.interval = 200;
	Timer_task2.cb       = Timer_task2_cb;
	Timer_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&Timer, &Timer_task1);
	timer_add_task(&Timer, &Timer_task2);
	timer_start(&Timer);
}

/**
 * Example of using TIMER_ADC.
 */
struct timer_task TIMER_ADC_task1, TIMER_ADC_task2;

static void TIMER_ADC_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_ADC_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_ADC_example(void)
{
	TIMER_ADC_task1.interval = 100;
	TIMER_ADC_task1.cb       = TIMER_ADC_task1_cb;
	TIMER_ADC_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_ADC_task2.interval = 200;
	TIMER_ADC_task2.cb       = TIMER_ADC_task2_cb;
	TIMER_ADC_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_ADC, &TIMER_ADC_task1);
	timer_add_task(&TIMER_ADC, &TIMER_ADC_task2);
	timer_start(&TIMER_ADC);
}
