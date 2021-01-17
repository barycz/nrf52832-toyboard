
#include "motor_driver.h"

#include <drivers/gpio.h>
#include <drivers/pwm.h>

int motor_driver_init(struct motor_driver* driver, const char* pwm_dev_name, uint32_t pin1, uint32_t pin2, const char* enable_gpio_name, uint32_t enable_pin)
{
	driver->pin1 = pin1;
	driver->pin2 = pin2;
	driver->enable_pin = enable_pin;
	driver->period_ns = 10000;

	driver->pwm = device_get_binding(pwm_dev_name);
	if (driver->pwm == NULL) {
		return -ENODEV;
	}

	driver->gpio = device_get_binding(enable_gpio_name);
	if (driver->gpio == NULL) {
		return -ENODEV;
	}

	int err;
	err = gpio_pin_configure(driver->gpio, driver->enable_pin, GPIO_OUTPUT_INACTIVE);
	if (err < 0) {
		return err;
	}

	err = motor_driver_set(driver, motor_driver_off, 0);
	return err;
}

uint32_t motor_driver_get_pulse1(struct motor_driver* driver, enum motor_driver_mode mode, uint32_t duty_pulse)
{
	switch (mode) {
		case motor_driver_off:
		case motor_driver_coast:
			return driver->period_ns;
		case motor_driver_reverse:
			return duty_pulse;
		default:
			return 0;
	}
}

uint32_t motor_driver_get_pulse2(struct motor_driver* driver, enum motor_driver_mode mode, uint32_t duty_pulse)
{
	switch (mode) {
		case motor_driver_off:
		case motor_driver_coast:
			return driver->period_ns;
		case motor_driver_forward:
			return duty_pulse;
		default:
			return 0;
	}
}

int motor_driver_set(struct motor_driver* driver, enum motor_driver_mode mode, motor_driver_duty duty)
{
	if (driver->pwm == NULL) {
		return -ENODEV;
	}

	if (driver->gpio == NULL) {
		return -ENODEV;
	}

	if (mode == motor_driver_off) {
		int err = gpio_pin_set(driver->gpio, driver->enable_pin, 0);
		if (err < 0) {
			return err;
		}
	}

	const uint32_t duty_pulse = (driver->period_ns * duty) / ((motor_driver_duty)-1);
	const uint32_t pulse1 = motor_driver_get_pulse1(driver, mode, duty_pulse);
	const uint32_t pulse2 = motor_driver_get_pulse2(driver, mode, duty_pulse);

	int err;
	err = pwm_pin_set_nsec(driver->pwm, driver->pin1, driver->period_ns, pulse1, 0);
	if (err < 0) {
		return err;
	}
	err = pwm_pin_set_nsec(driver->pwm, driver->pin2, driver->period_ns, pulse2, 0);
	if (err < 0) {
		return err;
	}

	if (mode != motor_driver_off) {
		int err = gpio_pin_set(driver->gpio, driver->enable_pin, 1);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}