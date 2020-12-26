#ifndef APPLICATION_MOTOR_DRIVER_H_
#define APPLICATION_MOTOR_DRIVER_H_

#include <stdint.h>

enum motor_driver_mode
{
	motor_driver_off = 0,
	motor_driver_on,
};

struct motor_driver
{
	const struct device* gpio;
};

int motor_driver_init(struct motor_driver* driver, const char* dev_name);
void motor_driver_set_mode(struct motor_driver* driver, enum motor_driver_mode mode);

#endif /* APPLICATION_MOTOR_DRIVER_H_ */