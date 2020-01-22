// Copyright © 2020 Maximilian Wenzkowski

#include "mcp9808.h"
#include <errno.h> // errno
#include <stdio.h>
#include <string.h> // strerror()


int main(int argc, const char *args[])
{
	char *i2c_bus = "/dev/i2c-1";
	unsigned int slave_addr = 0x18; 
	int fd;

	fd = mcp9808_open(i2c_bus, slave_addr);
	if (fd < 0) {
		printf("Öffnen des MCP9808 fehlgeschlagen: %s\n", strerror(errno));
		return 1;
	}

	float temp;

	if (mcp9808_get_temp(fd, &temp) < 0) {
		printf("Temperatur auslesen fehlgeschlagen: %s\n", strerror(errno));
		close(fd);
		return 1;
	}
	printf("Temperatur: %.2f °C\n", temp);
	
	if (mcp9808_get_critical_temp(fd, &temp) < 0) {
		printf("Auslesen des kritischen Temperaturwerts fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}
	printf("Kritische Temperatur: %.2f °C\n", temp);
	
	if (mcp9808_get_upper_alert_temp(fd, &temp) < 0) {
		printf("Auslesen der oberen Temperaturgrenze fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}
	printf("Obere Temperaturgrenze: %.2f °C\n", temp);
	
	if (mcp9808_get_lower_alert_temp(fd, &temp) < 0) {
		printf("Auslesen der unteren Temperaturgrenze fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}
	printf("Untere Temperaturgrenze: %.2f °C\n", temp);

	enum mcp9808_resolution res;
	if (mcp9808_get_resolution(fd, &res) < 0) {
		printf("Lesen der Temperatur-Auflösung fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}
	float res_f = 0;
	switch (res) {
		case MCP_9808_RES_0_5:
			res_f = 0.5f;
			break;
		case MCP_9808_RES_0_25:
			res_f = 0.25f;
			break;
		case MCP_9808_RES_0_125:
			res_f = 0.125f;
			break;
		case MCP_9808_RES_0_0625:
			res_f = 0.0625f;
			break;
	}
	printf("Auflösung: %.4f °C\n", res_f);

	struct mcp9808_config config;
	if (mcp9808_get_config(fd, &config) < 0) {
		printf("Lesen der Konfiguration fehlgeschlagen: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	if (config.alert_mode == MCP_9808_COMPARATOR_OUTPUT) {
		printf("alert ouput mode: Comparator output\n");
	} else {
		printf("alert ouput mode: Interrupt output\n");
	}
	if (config.alert_polarity == MCP_9808_ACTIVE_LOW) {
		printf("alert ouput polarity: Active-low\n");
	} else {
		printf("alert ouput polarity: Active-high\n");
	}
	if (config.alert_select == MCP_9808_ALERT_ALL) {
		printf("alert ouput select: Alert on all temp. boundaries\n");
	} else {
		printf("alert ouput select: Alert only if temp. > critical temp.\n");
	}
	printf("alert enabled: %s\n", config.alert_enabled ? "true" : "false");
	printf("temp. window locked: %s\n",
		config.temp_window_locked ? "true" : "false");
	printf("critical temp. locked: %s\n",
		config.critical_temp_locked ? "true" : "false");
	if (config.shutdown_mode == MCP_9808_CONT_CONVERSION) {
		printf("shutdown mode: Continous conversion\n");
	} else {
		printf("shutdown mode: Shutdown\n");
	}
	float hyst = 0.f;
	switch (config.hysteresis) {
		case MCP_9808_HYST_0: hyst = 0.0f; break;
		case MCP_9808_HYST_1_5: hyst = 1.5f; break;
		case MCP_9808_HYST_3: hyst = 3.0f; break;
		case MCP_9808_HYST_6: hyst = 6.0f; break;
	}
	printf("hysteresis: %.1f °C\n", hyst);

	bool alert_asserted;
	if (mcp9808_get_alert_status(fd, &alert_asserted) < 0) {
		printf("Lesen des alert status fehlgeschlagen: %s\n", strerror(errno));
		close(fd);
		return 1;
	}
	printf("alert asserted: %s\n", alert_asserted ? "true" : "false");

	
	close(fd);
	return 0;
}
