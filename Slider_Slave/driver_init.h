/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_INIT_INCLUDED
#define DRIVER_INIT_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>

#include <hal_adc_dma.h>

#include <hal_usart_async.h>
#include <hal_timer.h>

#include <hal_timer.h>
#include <hpl_tcc.h>

extern struct adc_dma_descriptor     ADC_DMA;
extern struct usart_async_descriptor USART_MAIN;
extern struct timer_descriptor       Timer;

extern struct timer_descriptor TIMER_ADC;

void USART_MAIN_PORT_init(void);
void USART_MAIN_CLOCK_init(void);
void USART_MAIN_init(void);

void TIMER_ADC_CLOCK_init(void);
void TIMER_ADC_init(void);

/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_INIT_INCLUDED
