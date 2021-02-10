#ifndef APPLICATION_CONF_H_
#define APPLICATION_CONF_H_

#include <stdint.h>

#define CONF_USER_COLOR_RGBA_LEN 4
#define CONF_USER_NAME_LEN 16

struct conf_user_data
{
	uint8_t color[CONF_USER_COLOR_RGBA_LEN];
	char name[CONF_USER_NAME_LEN];
};

struct conf
{
	uint32_t version;

	struct conf_user_data user_data;
};

int conf_init();
void conf_get_default(struct conf* conf);
int conf_read(struct conf* conf);
int conf_write(struct conf* conf);

#endif