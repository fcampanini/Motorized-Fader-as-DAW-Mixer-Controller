/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

#include <hpl_rtc_base.h>

/*! The buffer size for USART */
#define USART_MAIN_BUFFER_SIZE 16

struct adc_dma_descriptor     ADC_DMA;
struct usart_async_descriptor USART_MAIN;
struct timer_descriptor       Timer;

static uint8_t USART_MAIN_buffer[USART_MAIN_BUFFER_SIZE];

struct timer_descriptor TIMER_ADC;

/**
 * \brief ADC initialization function
 *
 * Enables ADC peripheral, clocks and initializes ADC driver
 */
static void ADC_DMA_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, ADC);
	_gclk_enable_channel(ADC_GCLK_ID, CONF_GCLK_ADC_SRC);

	adc_dma_init(&ADC_DMA, ADC);

	// Disable digital pin circuitry
	gpio_set_pin_direction(PA14, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(PA14, PINMUX_PA14B_ADC_AIN6);
}

/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
void USART_MAIN_CLOCK_init()
{

	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM1);
	_gclk_enable_channel(SERCOM1_GCLK_ID_CORE, CONF_GCLK_SERCOM1_CORE_SRC);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
void USART_MAIN_PORT_init()
{

	gpio_set_pin_function(PA22, PINMUX_PA22C_SERCOM1_PAD0);

	gpio_set_pin_function(PA23, PINMUX_PA23C_SERCOM1_PAD1);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
void USART_MAIN_init(void)
{
	USART_MAIN_CLOCK_init();
	usart_async_init(&USART_MAIN, SERCOM1, USART_MAIN_buffer, USART_MAIN_BUFFER_SIZE, (void *)NULL);
	USART_MAIN_PORT_init();
}

/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void Timer_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBA, RTC);
	_gclk_enable_channel(RTC_GCLK_ID, CONF_GCLK_RTC_SRC);
	timer_init(&Timer, RTC, _rtc_get_timer());
}

void TIMER_ADC_CLOCK_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, TCC0);
	_gclk_enable_channel(TCC0_GCLK_ID, CONF_GCLK_TCC0_SRC);
}

void TIMER_ADC_init(void)
{
	TIMER_ADC_CLOCK_init();
	timer_init(&TIMER_ADC, TCC0, _tcc_get_timer());
}

/**
 * \brief PTC initialization function
 *
 * Enables PTC peripheral, clocks and initializes PTC driver
 */
static void PTC_clock_init(void)
{
	_pm_enable_bus_clock(PM_BUS_APBC, PTC);
	_gclk_enable_channel(PTC_GCLK_ID, CONF_GCLK_PTC_SRC);
}

void system_init(void)
{
	init_mcu();

	ADC_DMA_init();

	USART_MAIN_init();

	Timer_init();

	TIMER_ADC_init();

	PTC_clock_init();
}
