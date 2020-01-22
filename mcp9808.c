// Copyright © 2020 Maximilian Wenzkowski

#include <assert.h> // assert()
#include <fcntl.h> // open()
#include <linux/i2c-dev.h> // I2C_SLAVE
#include <stdint.h>
#include <string.h> // strerror()
#include <sys/ioctl.h> // ioctl()
#include <unistd.h> // read(), write()

#include "mcp9808.h"

// todo t_a < t_lower, t_a > t_upper, t_a > t_critical

#define CONFIG_REG 0x01
#define UPPER_ALERT_TEMP_REG 0x02
#define LOWER_ALERT_TEMP_REG 0x03
#define CRITICAL_TEMP_REG 0x04
#define TEMP_REG 0x05
#define MANUFACTURER_ID_REG 0x06
#define DEVICE_ID_REG 0x07
#define RESOLUTION_REG 0x08

#define MANUFACTURER_ID 0x54
#define DEVICE_ID 0x04

#define CONFIG_ALERT_MODE_BIT (1<<0)
#define CONFIG_ALERT_POL_BIT (1<<1)
#define CONFIG_ALERT_SEL_BIT (1<<2)
#define CONFIG_ALERT_ENABLED_BIT (1<<3)
#define CONFIG_ALERT_STATUS_BIT (1<<4)
#define CONFIG_INTERRUPT_CLEAR_BIT (1<<5)
#define CONFIG_WINDOW_LOCKED_BIT (1<<6)
#define CONFIG_CRITICAL_LOCKED_BIT (1<<7)
#define CONFIG_SHUTDOWN_MODE_BIT (1<<8)
#define CONFIG_HYST_BITS 0x600
#define CONFIG_HYST_BITS_START 9


int
mcp9808_open(const char *i2c_bus, unsigned int slave_addr)
{
	int fd;
	uint8_t reg;
	uint8_t data[2];

	fd = open(i2c_bus, O_RDWR);
	if (fd < 0) {
		return -1;
	}

	if (ioctl(fd, I2C_SLAVE, slave_addr) < 0) {
		goto error;
	}
	
	// check manufacturer id
	reg = MANUFACTURER_ID_REG;
	if (write(fd, &reg, 1) < 0) {
		goto error;
	}
	if (read(fd, data, 2) != 2) {
		goto error;
	}
	int id = ( ((int) data[0]) << 8 ) + data[1];
	if (id != MANUFACTURER_ID) {
		goto error;
	}

	// check device id
	reg = DEVICE_ID_REG;
	if (write(fd, &reg, 1) < 0) {
		goto error;
	}
	if (read(fd, data, 2) != 2) {
		goto error;
	}
	// data[0] contains device id and data[1] the revision
	if (data[0] != DEVICE_ID) {
		goto error;
	}

	return fd;

error:
	close(fd);
	return -1;
}

static int
read_word(int fd, uint8_t reg_addr, uint16_t *word)
{
	assert(fd >= 0);
	assert(reg_addr >= CONFIG_REG && reg_addr <= RESOLUTION_REG);

	uint8_t data[2] = {0};

	if (write(fd, &reg_addr, 1) < 0) {
		return -1;
	}
	if (read(fd, data, 2) != 2) {
		return -1;
	}
	*word = ((uint16_t) data[0] << 8) + data[1];
	return 0;
}

static int
write_word(int fd, uint8_t reg_addr, uint16_t word)
{
	assert(fd >= 0);
	assert(reg_addr >= CONFIG_REG && reg_addr <= RESOLUTION_REG);

	uint8_t data[3];
	data[0] = reg_addr;
	data[1] = word >> 8; // first byte of word
	data[2] = word & 0xFF; // second byte of word

	if (write(fd, data, 3) < 3) {
		return -1;
	}
	return 0;
}

int
mcp9808_get_config(int fd, struct mcp9808_config *config)
{
	assert(fd >= 0);
	assert(config);

	uint16_t word;
	if (read_word(fd, CONFIG_REG, &word) < 0 ) {
		return -1;
	}

	config->alert_mode = word & CONFIG_ALERT_MODE_BIT
		? MCP_9808_INTERRUPT_OUTPUT
		: MCP_9808_COMPARATOR_OUTPUT;
	config->alert_polarity = word & CONFIG_ALERT_POL_BIT
		? MCP_9808_ACTIVE_HIGH
		: MCP_9808_ACTIVE_LOW;
	config->alert_select = word & CONFIG_ALERT_SEL_BIT
		? MCP_9808_ALERT_CRITICAL
		: MCP_9808_ALERT_ALL;
	config->alert_enabled = word & CONFIG_ALERT_ENABLED_BIT ? true : false;
	config->temp_window_locked = word & CONFIG_WINDOW_LOCKED_BIT ? true : false;
	config->critical_temp_locked = word & CONFIG_CRITICAL_LOCKED_BIT ? true : false;
	config->shutdown_mode = word & CONFIG_SHUTDOWN_MODE_BIT
		? MCP_9808_SHUTDOWN
		: MCP_9808_CONT_CONVERSION;
	config->hysteresis = (word & CONFIG_HYST_BITS) >> CONFIG_HYST_BITS_START;

	return 0;
}

int
mcp9808_set_config(int fd, struct mcp9808_config config)
{
	uint16_t word = 0;
	if (config.alert_mode == MCP_9808_INTERRUPT_OUTPUT) {
		word |= CONFIG_ALERT_MODE_BIT;
	}
	if (config.alert_polarity == MCP_9808_ACTIVE_HIGH) {
		word |= CONFIG_ALERT_POL_BIT;
	}
	if (config.alert_select == MCP_9808_ALERT_CRITICAL) {
		word |= CONFIG_ALERT_SEL_BIT;
	}
	if (config.alert_enabled) {
		word |= CONFIG_ALERT_ENABLED_BIT;
	}
	if (config.temp_window_locked) {
		word |= CONFIG_WINDOW_LOCKED_BIT;
	}
	if (config.critical_temp_locked) {
		word |= CONFIG_CRITICAL_LOCKED_BIT;
	}
	if (config.shutdown_mode == MCP_9808_SHUTDOWN) {
		word |= CONFIG_SHUTDOWN_MODE_BIT;
	}
	word |= ((uint16_t) config.hysteresis) << CONFIG_HYST_BITS_START;

	if (write_word(fd, CONFIG_REG, word) < 0) {
		return -1;
	}
	return 0;
}

int
mcp9808_get_alert_status(int fd, bool *alert_asserted)
{
	assert(fd >= 0);
	assert(alert_asserted);

	uint16_t word;
	if (read_word(fd, CONFIG_REG, &word) < 0 ) {
		return -1;
	}
	*alert_asserted = word & CONFIG_ALERT_STATUS_BIT ? true : false;
	return 0;
}

int
mcp9808_clear_interrupt(int fd)
{
	assert(fd >= 0);

	uint16_t word;
	if (read_word(fd, CONFIG_REG, &word) < 0 ) {
		return -1;
	}
	word &= ~(CONFIG_INTERRUPT_CLEAR_BIT);
	if (write_word(fd, CONFIG_REG, word) < 0) {
		return -1;
	}
	return 0;
}

static int
get_temp(int fd, int reg_addr, float *temp)
{
	assert(fd >= 0);
	assert(temp);
	assert(reg_addr >= UPPER_ALERT_TEMP_REG && reg_addr <= TEMP_REG);

	uint16_t word;
	if (read_word(fd, reg_addr, &word) < 0) {
		return -1;
	}

	uint8_t upper = word >> 8; // first byte
	uint8_t lower = word & 0xFF; // second byte

	upper &= 0x1F; // clear flag bits
	*temp = (upper & 0x0F) * 16 + (float) lower / 16.0f;

	//check for signum bit, i.e. if temperature is < 0.0 °C
	if (upper & 0x10) {
		*temp -= 256.0f;
	}

	return 0;
}

static int
set_temp(int fd, int reg_addr, float temp)
{
	assert(fd >= 0);
	assert(reg_addr >= UPPER_ALERT_TEMP_REG && reg_addr <= CRITICAL_TEMP_REG);
	assert(temp >= -40.0f && temp <= 125.0f);


	uint16_t word = 0;
	if (temp < 0.0f) {
			temp += 256.0f;
			word = 1<<12; // set signum bit
	}
	word += temp * 16.0f;

	if (write_word(fd, reg_addr, word) < 0) {
		return -1;
	}

	return 0;
}

int
mcp9808_get_temp(int fd, float *temp)
{
	return get_temp(fd, TEMP_REG, temp);
}

int
mcp9808_get_critical_temp(int fd, float *temp)
{
	return get_temp(fd, CRITICAL_TEMP_REG, temp);
}

int
mcp9808_set_critical_temp(int fd, float temp)
{
	return set_temp(fd, CRITICAL_TEMP_REG, temp);
}

int
mcp9808_get_upper_alert_temp(int fd, float *temp)
{
	return get_temp(fd, UPPER_ALERT_TEMP_REG, temp);
}

int
mcp9808_set_upper_alert_temp(int fd, float temp)
{
	return set_temp(fd, UPPER_ALERT_TEMP_REG, temp);
}

int
mcp9808_get_lower_alert_temp(int fd, float *temp)
{
	return get_temp(fd, LOWER_ALERT_TEMP_REG, temp);
}

int
mcp9808_set_lower_alert_temp(int fd, float temp)
{
	return set_temp(fd, LOWER_ALERT_TEMP_REG, temp);
}

int
mcp9808_get_resolution(int fd, enum mcp9808_resolution *res)
{
	assert(fd >= 0);
	assert(res);
	uint8_t reg = RESOLUTION_REG;
	uint8_t resolution;

	if (write(fd, &reg, 1) < 0) {
		return -1;
	}
	if (read(fd, &resolution, 1) < 0) {
		return -1;
	}
	*res = resolution;

	return 0;
}

int
mcp9808_set_resolution(int fd, enum mcp9808_resolution res)
{
	assert(fd >= 0);
	assert(res >= MCP_9808_RES_0_5 && res <= MCP_9808_RES_0_0625);

	uint8_t data[2];
	data[0] = RESOLUTION_REG;
	data[1] = res;

	if (write(fd, data, 2) < 2) {
		// less than 2 bytes written or error
		return -1;
	}
	return 0;
}
