#ifndef APPLICATION_MOTOR_DRIVER_H_
#define APPLICATION_MOTOR_DRIVER_H_

#include <stdint.h>

enum motor_driver_mode
{
	motor_driver_off = 0,
	motor_driver_coast,
	motor_driver_break,
	motor_driver_forward,
	motor_driver_reverse,
};

typedef uint8_t motor_driver_duty;

struct motor_driver
{
	const struct device* gpio; // to enable external driver
	const struct device* pwm;
	uint32_t pin1;
	uint32_t pin2;
	uint32_t enable_pin;
	uint32_t period_ns;
};

int motor_driver_init(struct motor_driver* driver, const char* pwm_dev_name, uint32_t pin1, uint32_t pin2, const char* enable_gpio_name, uint32_t enable_pin);
int motor_driver_set(struct motor_driver* driver, enum motor_driver_mode mode, motor_driver_duty duty);

#endif /* APPLICATION_MOTOR_DRIVER_H_ */