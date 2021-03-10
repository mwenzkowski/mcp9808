// Copyright © 2021 Maximilian Wenzkowski

#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h> // strtof(), EXIT_FAILURE, EXIT_SUCCESS
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
	const char *I2C_BUS = "/dev/i2c-1";
	const unsigned int SLAVE_ADDR = 0x18; 

	// file descriptor which is used to communicate with the mcp9808 device
	int fd;

	// the target temperature in °C (-40 °C is the lowest valid temperature)
	float temp = -40.0f;

	if (argc != 2) {
		printUsage();
		return EXIT_FAILURE;
	}


	// If the first argument is not "aus" try to parse the target temperature
	if (strcmp(args[1], "aus") != 0) {
		char *endptr = NULL;
		temp = strtof(args[1], &endptr);

		if (endptr == args[1]) {
			fprintf(stderr, "Fehler: Temperatur ungültig\n");
			return EXIT_FAILURE;
		} else if (temp < -40.0f || temp > 125.0f) {
			fprintf(stderr, "Fehler: Temperatur außerhalb von [-40 °C, 125.0 °C]\n");
			return EXIT_FAILURE;
		}
	}


	fd = mcp9808_open(I2C_BUS, SLAVE_ADDR);
	if (fd < 0) {
		fprintf(stderr, "Öffnen des MCP9808 fehlgeschlagen: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	struct mcp9808_config config;
	if (mcp9808_get_config(fd, &config) < 0) {
		fprintf(stderr, "Lesen der Konfiguration fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return EXIT_FAILURE;
	}

	if (config.critical_temp_locked) {
			fprintf(stderr, "Fehler: Sperrung des kritische Temperaturwerts "
				"aktiv.\n");
			close(fd);
			return EXIT_FAILURE;
	}
	if (config.temp_window_locked) {
			fprintf(stderr, "Fehler: Sperrung des Temperaturintervalls aktiv.\n");
			close(fd);
			return EXIT_FAILURE;
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
		return EXIT_FAILURE;
	}

	if (mcp9808_set_critical_temp(fd, temp) < 0) {
		fprintf(stderr,
			"Setzen des kritischen Temperaturwerts fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return EXIT_FAILURE;
	}

	if (mcp9808_set_resolution(fd, MCP_9808_RES_0_0625) < 0) {
		fprintf(stderr,
			"Setzen des kritischen Temperaturwerts fehlgeschlagen: %s\n",
			strerror(errno));
		close(fd);
		return EXIT_FAILURE;
	}

	printf("Eingestellte Temperatur: %.2f °C\n", temp);
	
	close(fd);
	return EXIT_SUCCESS;
}
