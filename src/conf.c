
#include "conf.h"

#include <zephyr.h>
#include <fs/nvs.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>

#include <string.h>

#define CONF_ID 0

static struct nvs_fs conf_nvs_fs;

void conf_get_default(struct conf* conf)
{
	memset(conf, 0, sizeof(*conf));

	conf->version = 1;
}

int conf_init()
{
	const struct device* flash_dev = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (flash_dev == NULL) {
		return -ENODEV;
	}

	struct flash_pages_info info;
	conf_nvs_fs.offset = FLASH_AREA_OFFSET(storage);
	int err = flash_get_page_info_by_offs(flash_dev, conf_nvs_fs.offset, &info);
	if (err) {
		return err;
	}
	conf_nvs_fs.sector_size = info.size;
	conf_nvs_fs.sector_count = 3U;

	err = nvs_init(&conf_nvs_fs, DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (err) {
		return err;
	}

	return 0;
}

int conf_read(struct conf* conf)
{
	int err = nvs_read(&conf_nvs_fs, CONF_ID, conf, sizeof(*conf));
	return err;
}

int conf_write(struct conf* conf)
{
	int err = nvs_write(&conf_nvs_fs, CONF_ID, conf, sizeof(*conf));
	return err;
}
