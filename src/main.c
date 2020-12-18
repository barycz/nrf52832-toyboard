
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

/* Custom Service Variables */
// 12345678-1234-5678-1234-56789abcdef0
static struct bt_uuid_128 vnd_uuid = BT_UUID_INIT_128(
	0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12);

static int signed_value;

static ssize_t read_signed(struct bt_conn *conn, const struct bt_gatt_attr *attr,
	void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(signed_value));
}

static ssize_t write_signed(struct bt_conn *conn, const struct bt_gatt_attr *attr,
	const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(signed_value)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);

	gpio_pin_set(led_blue_dev, 12, *value);

	return len;
}

static const struct bt_uuid_128 vnd_signed_uuid = BT_UUID_INIT_128(
	0xf3, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x13,
	0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x13);

BT_GATT_SERVICE_DEFINE(vnd_svc,
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	BT_GATT_CHARACTERISTIC(&vnd_signed_uuid.uuid, BT_GATT_CHRC_READ |
		BT_GATT_CHRC_WRITE | BT_GATT_CHRC_AUTH,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_signed, write_signed, &signed_value),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
		0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12),
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
	err = gpio_pin_configure(led_red_dev, LED_RED_PIN, GPIO_OUTPUT_INACTIVE | LED_BLUE_FLAGS);

	err = gpio_pin_configure(led_blue_dev, 12, GPIO_OUTPUT_ACTIVE); // DRVAIN1
	err = gpio_pin_configure(led_blue_dev, 13, GPIO_OUTPUT_INACTIVE); // DRVAIN2
	err = gpio_pin_configure(led_blue_dev, 11, GPIO_OUTPUT_ACTIVE); // NDRVSLEEP
	if (err < 0) {
		return;
	}

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	while(1) {
		gpio_pin_set(led_blue_dev, LED_BLUE_PIN, 1);
		k_sleep(K_MSEC(100));
		gpio_pin_set(led_blue_dev, LED_BLUE_PIN, 0);
		k_sleep(K_MSEC(1900));
	}
}
