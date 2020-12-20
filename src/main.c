
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

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

#define DRIVER_OUTPUT_FIRST_BIT 12
#define DRIVER_OUTPUT_MASK 0x0f

// 00000001-201b-286a-f29d-ff952dc319a2
static struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	0xa2, 0x19, 0xc3, 0x2d, 0x95, 0xff, 0x9d, 0xf2,
	0x6a, 0x28, 0x1b, 0x20, 0x01, 0x00, 0x00, 0x00
);

static uint8_t driver_value;

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

	gpio_port_set_masked(led_blue_dev,
		DRIVER_OUTPUT_MASK << DRIVER_OUTPUT_FIRST_BIT,
		driver_value << DRIVER_OUTPUT_FIRST_BIT);

	return len;
}

// 00000002-201b-286a-f29d-ff952dc319a2
static const struct bt_uuid_128 vnd_signed_uuid = BT_UUID_INIT_128(
	0xa2, 0x19, 0xc3, 0x2d, 0x95, 0xff, 0x9d, 0xf2,
	0x6a, 0x28, 0x1b, 0x20, 0x02, 0x00, 0x00, 0x00
);

BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	BT_GATT_CHARACTERISTIC(&vnd_signed_uuid.uuid,
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_driver_value, write_driver_value, &driver_value),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_UUID128_ALL, vnd_uuid.val, sizeof(vnd_uuid.val)),
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
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

void main(void)
{
	int err;

	printk("Starting Beacon Demo\n");

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

	err = gpio_pin_configure(led_blue_dev, DRIVER_OUTPUT_FIRST_BIT, GPIO_OUTPUT_ACTIVE); // DRVAIN1
	err = gpio_pin_configure(led_blue_dev, DRIVER_OUTPUT_FIRST_BIT + 1, GPIO_OUTPUT_INACTIVE); // DRVAIN2
	err = gpio_pin_configure(led_blue_dev, DRIVER_OUTPUT_FIRST_BIT + 2, GPIO_OUTPUT_ACTIVE); // DRVBIN1
	err = gpio_pin_configure(led_blue_dev, DRIVER_OUTPUT_FIRST_BIT + 3, GPIO_OUTPUT_INACTIVE); // DRVBIN2
	err = gpio_pin_configure(led_blue_dev, 11, GPIO_OUTPUT_ACTIVE); // NDRVSLEEP
	if (err < 0) {
		return;
	}

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	while(1) {
		gpio_pin_set(led_blue_dev, LED_BLUE_PIN, 1);
		k_sleep(K_MSEC(100));
		gpio_pin_set(led_blue_dev, LED_BLUE_PIN, 0);
		k_sleep(K_MSEC(1900));
	}
}
