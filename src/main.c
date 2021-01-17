
#include "battery.h"
#include "motor_driver.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <sys/printk.h>
#include <sys/byteorder.h>

#include <stddef.h>
#include <string.h>
#include <errno.h>

static const struct device *led_blue_dev;
static const struct device *led_red_dev;
static struct motor_driver motor_a;
static struct motor_driver motor_b;

// 00000001-201b-286a-f29d-ff952dc319a2
static struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	0xa2, 0x19, 0xc3, 0x2d, 0x95, 0xff, 0x9d, 0xf2,
	0x6a, 0x28, 0x1b, 0x20, 0x01, 0x00, 0x00, 0x00
);

struct motor_driver_bt_data
{
	enum motor_driver_mode mode;
	motor_driver_duty duty;
};

struct motor_drivers_characteristic
{
	struct motor_driver_bt_data data[2];
};

static struct motor_drivers_characteristic driver_value;

static ssize_t read_driver_value(struct bt_conn *conn, const struct bt_gatt_attr *attr,
	void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(driver_value));
}

static ssize_t write_driver_value(struct bt_conn *conn, const struct bt_gatt_attr *attr,
	const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(driver_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	motor_driver_set(&motor_a, driver_value.data[0].mode, driver_value.data[0].duty);
	motor_driver_set(&motor_b, driver_value.data[1].mode, driver_value.data[1].duty);

	return len;
}

// 00000002-201b-286a-f29d-ff952dc319a2
static const struct bt_uuid_128 vnd_drivers_uuid = BT_UUID_INIT_128(
	0xa2, 0x19, 0xc3, 0x2d, 0x95, 0xff, 0x9d, 0xf2,
	0x6a, 0x28, 0x1b, 0x20, 0x02, 0x00, 0x00, 0x00
);

BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	BT_GATT_CHARACTERISTIC(&vnd_drivers_uuid.uuid,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_driver_value, write_driver_value, &driver_value),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
	BT_DATA(BT_DATA_UUID128_ALL, vnd_uuid.val, sizeof(vnd_uuid.val)),
};

#define ADV_PARAMS BT_LE_ADV_PARAM( \
	BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME, \
	BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX, NULL)

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(ADV_PARAMS, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

#define LED_BLUE_NODE DT_ALIAS(led_blue)
#define LED_BLUE DT_GPIO_LABEL(LED_BLUE_NODE, gpios)
#define LED_BLUE_PIN DT_GPIO_PIN(LED_BLUE_NODE, gpios)
#define LED_BLUE_FLAGS DT_GPIO_FLAGS(LED_BLUE_NODE, gpios)

#define LED_RED_NODE DT_ALIAS(led_red)
#define LED_RED DT_GPIO_LABEL(LED_RED_NODE, gpios)
#define LED_RED_PIN DT_GPIO_PIN(LED_RED_NODE, gpios)
#define LED_RED_FLAGS DT_GPIO_FLAGS(LED_RED_NODE, gpios)

#define PWM0_NODE DT_NODELABEL(pwm0)
#define PWM0 DT_LABEL(PWM0_NODE)
#define MOTOR_PIN_NSLEEP 11
#define MOTOR_PIN_A1 12
#define MOTOR_PIN_A2 13
#define MOTOR_PIN_B1 14
#define MOTOR_PIN_B2 15

void main(void)
{
	int err;

	printk("Starting toyboard\n");

	led_blue_dev = device_get_binding(LED_BLUE);
	if (led_blue_dev == NULL) {
		return;
	}

	led_red_dev = device_get_binding(LED_BLUE);
	if (led_red_dev == NULL) {
		return;
	}

	err = gpio_pin_configure(led_blue_dev, LED_BLUE_PIN, GPIO_OUTPUT_INACTIVE | LED_BLUE_FLAGS);
	err = gpio_pin_configure(led_red_dev, LED_RED_PIN, GPIO_OUTPUT_INACTIVE | LED_RED_FLAGS);
	if (err < 0) {
		return;
	}

	err = motor_driver_init(&motor_a, PWM0, MOTOR_PIN_A1, MOTOR_PIN_A2, LED_BLUE, MOTOR_PIN_NSLEEP); // TODO right device
	if (err < 0) {
		return;
	}

	err = motor_driver_init(&motor_b, PWM0, MOTOR_PIN_B1, MOTOR_PIN_B2, LED_BLUE, MOTOR_PIN_NSLEEP); // TODO right device
	if (err < 0) {
		return;
	}

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	while(1) {
		uint8_t batt_level = battery_get_level();
		bt_bas_set_battery_level(batt_level);
		gpio_pin_t pin = LED_BLUE_PIN;
		if (batt_level < 10) {
			pin = LED_RED_PIN;
			motor_driver_set(&motor_a, motor_driver_off, 0);
			motor_driver_set(&motor_b, motor_driver_off, 0);
		}

		gpio_pin_set(led_blue_dev, pin, 1);
		k_sleep(K_MSEC(100));
		gpio_pin_set(led_blue_dev, pin, 0);
		k_sleep(K_MSEC(2900));
	}
}
