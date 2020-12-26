/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include "battery.h"

#include <zephyr.h>
#include <init.h>
#include <drivers/adc.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define VBATT DT_PATH(vbatt)

#define BATTERY_MIN_MILLIVOLTS 7420
#define BATTERY_MAX_MILLIVOLTS 8300
#define BATTERY_RANGE_MILLIVOLTS (BATTERY_MAX_MILLIVOLTS - BATTERY_MIN_MILLIVOLTS)

struct battery {
	const struct device *adc;
	struct adc_channel_cfg adc_cfg;
	struct adc_sequence adc_seq;
	int16_t raw;
	bool initialized;
};

static int battery_init(struct battery* batt)
{
	batt->adc = device_get_binding(DT_IO_CHANNELS_LABEL(VBATT));
	if (batt->adc == NULL) {
		return -ENOENT;
	}

	batt->adc_seq = (struct adc_sequence){
		.channels = BIT(0),
		.buffer = &batt->raw,
		.buffer_size = sizeof(batt->raw),
		.resolution = 14,
		.oversampling = 4,
		.calibrate = true,
	};

	batt->adc_cfg = (struct adc_channel_cfg){
		.gain = ADC_GAIN_1_6,
		.reference = ADC_REF_INTERNAL,
		.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
		.input_positive = SAADC_CH_PSELP_PSELP_AnalogInput0 + DT_IO_CHANNELS_INPUT(VBATT),
	};

	int rc = adc_channel_setup(batt->adc, &batt->adc_cfg);
	if (rc == 0) {
		batt->initialized = true;
	}

	return rc;
}

int battery_read_millivolts(struct battery* batt)
{
	int rc = -ENOENT;

	if (batt->initialized == false) {
		return rc;
	}

	rc = adc_read(batt->adc, &batt->adc_seq);
	batt->adc_seq.calibrate = false;
	if (rc) {
		return rc;
	}

	int32_t val = batt->raw;

	adc_raw_to_millivolts(
		adc_ref_internal(batt->adc),
		batt->adc_cfg.gain,
		batt->adc_seq.resolution,
		&val);

	rc = val * (uint64_t)DT_PROP(VBATT, full_ohms) / DT_PROP(VBATT, output_ohms);

	return rc;
}

static struct battery s_battery;

static int battery_setup(const struct device *arg)
{
	int rc = battery_init(&s_battery);
	return rc;
}

SYS_INIT(battery_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

uint8_t battery_get_level()
{
	int adc = battery_read_millivolts(&s_battery);
	if (adc < 0) {
		return 0; // error
	}

	if (adc >= BATTERY_MAX_MILLIVOLTS) {
		return 100u;
	} else if(adc <= BATTERY_MIN_MILLIVOLTS) {
		return 0u;
	} else {
		return 100 * (adc - BATTERY_MIN_MILLIVOLTS) / BATTERY_RANGE_MILLIVOLTS;
	}
}
