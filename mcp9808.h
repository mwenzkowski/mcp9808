// Copyright © 2020 Maximilian Wenzkowski

#ifndef MCP9808_H
#define MCP9808_H

#include <stdbool.h>
#include <unistd.h> // close()

enum mcp9808_resolution {
	MCP_9808_RES_0_5    = 0, // 0.5 °C
	MCP_9808_RES_0_25   = 1, // 0.25 °C
	MCP_9808_RES_0_125  = 2, // 0.125 °C
	MCP_9808_RES_0_0625 = 3  // 0.0625 °C
};

enum mcp9808_alert_mode {
	MCP_9808_COMPARATOR_OUTPUT, // (power-up default)
	MCP_9808_INTERRUPT_OUTPUT
};

enum mcp9808_alert_polarity {
	MCP_9808_ACTIVE_LOW, // (power-up default; pull-up resistor required)
	MCP_9808_ACTIVE_HIGH
};

enum mcp9808_alert_select {
	// Alert output for T_UPPER, T_LOWER and T_CRIT (power-up default)
	MCP_9808_ALERT_ALL,
	// Alert output for T_A > T_CRIT only (T_UPPER and T_LOWER temperature
	// boundaries are disabled)
	MCP_9808_ALERT_CRITICAL
};

enum mcp9808_shutdown_mode {
	MCP_9808_CONT_CONVERSION, // (power-up default)
	MCP_9808_SHUTDOWN
};

enum mcp9808_hysteresis {
	MCP_9808_HYST_0   = 0, //  0.0 °C
	MCP_9808_HYST_1_5 = 1, // +1.5 °C
	MCP_9808_HYST_3   = 2, // +3.0 °C
	MCP_9808_HYST_6   = 3  // +6.0 °C
};

struct mcp9808_config {
	
	enum mcp9808_alert_mode alert_mode;
	enum mcp9808_alert_polarity alert_polarity;
	enum mcp9808_alert_select alert_select;
	bool alert_enabled;
	bool temp_window_locked; // uppper & lower alert temp locked
	bool critical_temp_locked; // critical alert temp locked
	enum mcp9808_shutdown_mode shutdown_mode;
	enum mcp9808_hysteresis hysteresis;
};

int
mcp9808_open(const char *i2c_bus, unsigned int slave_addr);

int
mcp9808_get_config(int fd, struct mcp9808_config *config);

int
mcp9808_set_config(int fd, struct mcp9808_config config);

int
mcp9808_get_alert_status(int fd, bool *alert_asserted);

int
mcp9808_clear_interrupt(int fd);

int
mcp9808_get_temp(int fd, float *temp);

int
mcp9808_get_critical_temp(int fd, float *temp);

int
mcp9808_set_critical_temp(int fd, float temp);

int
mcp9808_get_upper_alert_temp(int fd, float *temp);

int
mcp9808_set_upper_alert_temp(int fd, float temp);

int
mcp9808_get_lower_alert_temp(int fd, float *temp);

int
mcp9808_set_lower_alert_temp(int fd, float temp);

int
mcp9808_get_resolution(int fd, enum mcp9808_resolution *res);

int
mcp9808_set_resolution(int fd, enum mcp9808_resolution res);
#endif
