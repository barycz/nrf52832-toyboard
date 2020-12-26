
#include "motor_driver.h"

#include <drivers/gpio.h>

#define DRIVER_OUTPUT_FIRST_BIT 12
#define DRIVER_OUTPUT_MASK 0x0f
#define DRIVER_NSLEEP 11

int motor_driver_init(struct motor_driver* driver, const char* dev_name)
{
	driver->gpio = device_get_binding(dev_name);
	if (driver->gpio == NULL) {
		return -ENODEV;
	}

	int err;
	err = gpio_pin_configure(driver->gpio, DRIVER_OUTPUT_FIRST_BIT, GPIO_OUTPUT_ACTIVE); // DRVAIN1
	err = gpio_pin_configure(driver->gpio, DRIVER_OUTPUT_FIRST_BIT + 1, GPIO_OUTPUT_INACTIVE); // DRVAIN2
	err = gpio_pin_configure(driver->gpio, DRIVER_OUTPUT_FIRST_BIT + 2, GPIO_OUTPUT_ACTIVE); // DRVBIN1
	err = gpio_pin_configure(driver->gpio, DRIVER_OUTPUT_FIRST_BIT + 3, GPIO_OUTPUT_INACTIVE); // DRVBIN2
	err = gpio_pin_configure(driver->gpio, DRIVER_NSLEEP, GPIO_OUTPUT_ACTIVE); // NDRVSLEEP
	if (err < 0) {
		return err;
	}

	motor_driver_set_mode(driver, motor_driver_on);
	return err;
}

void motor_driver_set_mode(struct motor_driver* driver, enum motor_driver_mode mode)
{
	gpio_port_set_masked(driver->gpio,
		DRIVER_OUTPUT_MASK << DRIVER_OUTPUT_FIRST_BIT,
		mode << DRIVER_OUTPUT_FIRST_BIT);

	gpio_pin_set(driver->gpio, DRIVER_NSLEEP, mode != motor_driver_off);
}