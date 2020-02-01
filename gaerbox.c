// Copyright © 2020 Maximilian Wenzkowski

#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h> // strtof()
#include <string.h> // strerror()

#include "mcp9808.h"

void
printUsage(void)
{
	printf("Aufruf: gärbox <Temperatur>\n"
		"        gärbox aus\n\n"
		"Stellt die gewünschte Temperatur ein.\n"
		"Die Temperatur wird in Grad Celcius in der Form 23.45 angegeben "
		"und muss im Intervall [-40 °C, 125.0 °C] liegen.\n"
		"'gärbox aus' ist eine Kurzform von 'gärbox -40'\n");
}

int
main(int argc, const char *args[])
{
	char *i2c_bus = "/dev/i2c-1";
	unsigned int slave_addr = 0x18; 
	int fd;
	float temp = -40.0f;

	if (argc != 2) {
		printUsage();
		return 1;
	}


	// args[1] != "aus"
	if (strcmp(args[1], "aus") != 0) {
		char *endptr = NULL;
		temp = strtof(args[1], &endptr);

		if (endptr == args[1]) {
			fprintf(stderr, "Fehler: Temperatur ungültig\n");
			return 1;
		} else if (temp < -40.0f || temp > 125.0f) {
			fprintf(stderr, "Fehler: Temperatur außerhalb von [-40 °C, 125.0 °C]\n");
			return 1;
		}
	}


	fd = mcp9808_open(i2c_bus, slave_addr);
	if (fd < 0) {
		fprintf(stderr, "Öffnen des MCP9808 fehlgeschlagen: %s\n", strerror(errno));
		return 1;
	}

	struct mcp9808_config config;
	if (mcp9808_get_config(fd, &config) < 0) {
		fprintf(stderr, "Lesen der Konfiguration fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}

	if (config.critical_temp_locked) {
			fprintf(stderr, "Fehler: Sperrung des kritische Temperaturwerts "
				"aktiv.\n");
			close(fd);
			return 1;
	}
	if (config.temp_window_locked) {
			fprintf(stderr, "Fehler: Sperrung des Temperaturintervalls aktiv.\n");
			close(fd);
			return 1;
	}

	config.alert_mode = MCP_9808_COMPARATOR_OUTPUT;
	config.alert_polarity = MCP_9808_ACTIVE_LOW;
	config.alert_select = MCP_9808_ALERT_CRITICAL;
	config.alert_enabled = true;
	config.shutdown_mode = MCP_9808_CONT_CONVERSION;
	config.hysteresis = MCP_9808_HYST_0;

	if (mcp9808_set_config(fd, config) < 0) {
		fprintf(stderr, "Setzen der Konfiguration fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}

	if (mcp9808_set_critical_temp(fd, temp) < 0) {
		fprintf(stderr,
			"Setzen des kritischen Temperaturwerts fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}

	if (mcp9808_set_resolution(fd, MCP_9808_RES_0_0625) < 0) {
		fprintf(stderr,
			"Setzen des kritischen Temperaturwerts fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return 1;
	}

	printf("Eingestellte Temperatur: %.2f °C\n", temp);
	
	close(fd);
	return 0;
}
