/**
 * \file
 *
 * \brief ADC functionality implementation.
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#include <hal_adc_dma.h>
#include <utils_assert.h>
#include <utils.h>
#include <hal_atomic.h>
#include <hpl_dma.h>
#include <hpl_adc_config.h>

/**
 * \brief Driver version
 */
#define DRIVER_VERSION 0x00000001u

#ifndef CONF_DMA_ADC_CHANNEL
#define CONF_DMA_ADC_CHANNEL 0
#endif

static void adc_dma_complete(struct _dma_resource *resource);
static void adc_dma_error_occured(struct _dma_resource *resource);

/**
 * \brief Initialize ADC
 */
int32_t adc_dma_init(struct adc_dma_descriptor *const descr, void *const hw)
{
	int32_t                 init_status;
	struct _adc_dma_device *device;

	ASSERT(descr && hw);

	device      = &descr->device;
	init_status = _adc_dma_init(device, hw);
	if (init_status) {
		return init_status;
	}

	_dma_set_source_address(CONF_DMA_ADC_CHANNEL, (void *)_adc_get_source_for_dma(device));

	_dma_get_channel_resource(&descr->resource, CONF_DMA_ADC_CHANNEL);
	descr->resource->back                 = descr;
	descr->resource->dma_cb.transfer_done = adc_dma_complete;
	descr->resource->dma_cb.error         = adc_dma_error_occured;

	return ERR_NONE;
}

/**
 * \brief Deinitialize ADC
 */
int32_t adc_dma_deinit(struct adc_dma_descriptor *const descr)
{
	ASSERT(descr);

	_adc_dma_deinit(&descr->device);

	return ERR_NONE;
}

/**
 * \brief Enable ADC
 */
int32_t adc_dma_enable_channel(struct adc_dma_descriptor *const descr, const uint8_t channel)
{
	ASSERT(descr);

	_adc_dma_enable_channel(&descr->device, channel);

	return ERR_NONE;
}

/**
 * \brief Disable ADC
 */
int32_t adc_dma_disable_channel(struct adc_dma_descriptor *const descr, const uint8_t channel)
{
	ASSERT(descr);

	_adc_dma_disable_channel(&descr->device, channel);

	return ERR_NONE;
}

/**
 * \brief Register ADC callback
 */
int32_t adc_dma_register_callback(struct adc_dma_descriptor *const descr, const enum adc_dma_callback_type type,
                                  adc_dma_cb_t cb)
{
	ASSERT(descr);

	switch (type) {
	case ADC_DMA_COMPLETE_CB:
		descr->adc_dma_cb.convert_done = cb;
		break;
	case ADC_DMA_ERROR_CB:
		descr->adc_dma_cb.error = cb;
		break;
	default:
		return ERR_INVALID_ARG;
	}

	_dma_set_irq_state(CONF_DMA_ADC_CHANNEL, (enum _dma_callback_type)type, cb != NULL);

	return ERR_NONE;
}

/*
 * \brief Read data from ADC
 */
int32_t adc_dma_read(struct adc_dma_descriptor *const descr, uint8_t *const buffer, const uint16_t length)
{
	uint8_t data_size;

	ASSERT(descr && buffer && length);

	data_size = _adc_dma_get_data_size(&descr->device);
	ASSERT(!(length % data_size));
	(void)data_size;

	_dma_set_destination_address(CONF_DMA_ADC_CHANNEL, buffer);
	_dma_set_data_amount(CONF_DMA_ADC_CHANNEL, length);
	_dma_enable_transaction(CONF_DMA_ADC_CHANNEL, false);

	return ERR_NONE;
}

/**
 * \brief Start conversion
 */
int32_t adc_dma_start_conversion(struct adc_dma_descriptor *const descr)
{
	ASSERT(descr);

	_adc_dma_convert(&descr->device);
	return ERR_NONE;
}

/**
 * \brief Set ADC reference source
 */
int32_t adc_dma_set_reference(struct adc_dma_descriptor *const descr, const adc_reference_t reference)
{
	ASSERT(descr);

	_adc_dma_set_reference_source(&descr->device, reference);
	return ERR_NONE;
}

/**
 * \brief Set ADC resolution
 */
int32_t adc_dma_set_resolution(struct adc_dma_descriptor *const descr, const adc_resolution_t resolution)
{
	ASSERT(descr);

	_adc_dma_set_resolution(&descr->device, resolution);
	return ERR_NONE;
}

/**
 * \brief Set ADC input source of a channel
 */
int32_t adc_dma_set_inputs(struct adc_dma_descriptor *const descr, const adc_pos_input_t pos_input,
                           const adc_neg_input_t neg_input, const uint8_t channel)
{
	ASSERT(descr);

	_adc_dma_set_inputs(&descr->device, pos_input, neg_input, channel);
	return ERR_NONE;
}

/**
 * \brief Set ADC thresholds
 */
int32_t adc_dma_set_thresholds(struct adc_dma_descriptor *const descr, const adc_threshold_t low_threshold,
                               const adc_threshold_t up_threshold)
{
	ASSERT(descr);

	_adc_dma_set_thresholds(&descr->device, low_threshold, up_threshold);
	return ERR_NONE;
}

/**
 * \brief Set ADC gain
 */
int32_t adc_dma_set_channel_gain(struct adc_dma_descriptor *const descr, const uint8_t channel, const adc_gain_t gain)
{
	ASSERT(descr);

	_adc_dma_set_channel_gain(&descr->device, channel, gain);
	return ERR_NONE;
}

/**
 * \brief Set ADC conversion mode
 */
int32_t adc_dma_set_conversion_mode(struct adc_dma_descriptor *const descr, const enum adc_conversion_mode mode)
{
	ASSERT(descr);

	_adc_dma_set_conversion_mode(&descr->device, mode);
	return ERR_NONE;
}

/**
 * \brief Set ADC differential mode
 */
int32_t adc_dma_set_channel_differential_mode(struct adc_dma_descriptor *const descr, const uint8_t channel,
                                              const enum adc_differential_mode mode)
{
	ASSERT(descr);

	_adc_dma_set_channel_differential_mode(&descr->device, channel, mode);
	return ERR_NONE;
}

/**
 * \brief Set ADC window mode
 */
int32_t adc_dma_set_window_mode(struct adc_dma_descriptor *const descr, const adc_window_mode_t mode)
{
	ASSERT(descr);

	_adc_dma_set_window_mode(&descr->device, mode);
	return ERR_NONE;
}

/**
 * \brief Retrieve threshold state
 */
int32_t adc_dma_get_threshold_state(const struct adc_dma_descriptor *const descr, adc_threshold_status_t *const state)
{
	ASSERT(descr && state);

	_adc_dma_get_threshold_state(&descr->device, state);
	return ERR_NONE;
}

/**
 * \brief Check if conversion is complete
 */
int32_t adc_dma_is_conversion_complete(const struct adc_dma_descriptor *const descr)
{
	ASSERT(descr);

	return _adc_dma_is_conversion_done(&descr->device);
}

/**
 * \brief Retrieve the current driver version
 */
uint32_t adc_dma_get_version(void)
{
	return DRIVER_VERSION;
}

/**
 * \internal Process conversion completion
 *
 * \param[in] device The pointer to ADC device structure
 * \param[in] data Converted data
 */
static void adc_dma_complete(struct _dma_resource *resource)
{
	struct adc_dma_descriptor *const descr = (struct adc_dma_descriptor *)resource->back;

	if (descr->adc_dma_cb.convert_done) {
		descr->adc_dma_cb.convert_done(descr);
	}
}

static void adc_dma_error_occured(struct _dma_resource *resource)
{
	struct adc_dma_descriptor *const descr = (struct adc_dma_descriptor *)resource->back;

	if (descr->adc_dma_cb.error) {
		descr->adc_dma_cb.error(descr);
	}
}
