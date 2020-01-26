#include <assert.h> // assert()
#include <curl/curl.h>
#include <errno.h> // Variable errno
#include <inttypes.h> // UINT64_C(), PRIu64
#include <stdbool.h> // true, false
#include <stdint.h> // uint64_t
#include <stdio.h> // printf(), fprintf()
#include <string.h> // strerror()
#include <time.h> // clock_gettime()
#include <unistd.h> // sleep()

#include "mcp9808.h"

#define BUF_LEN 512

const char INFLUXDB_URL[] = "http://localhost:8086/write?db=pi1&precision=ms";

char curl_data[BUF_LEN];


CURL *
create_influxdb_curl_handle()
{
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		return NULL;
	}
	curl_easy_setopt(curl, CURLOPT_URL, INFLUXDB_URL);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curl_data);

	return curl;
}

bool
insert_temp(CURL *curl, float temp, struct timespec timestamp)
{
	assert(curl);

	// Zeitstempel in Millisekunden umrechnen
	uint64_t time_ms = (uint64_t) timestamp.tv_sec * UINT64_C(1000)
		+ (uint64_t) timestamp.tv_nsec / UINT64_C(1000000);

	long len = snprintf(curl_data, BUF_LEN, "temp value=%f %" PRIu64, temp, time_ms);
	assert(len < BUF_LEN);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr,
			"Fehler: Senden der Temperatur an die Datenbank"
			" fehlgeschlagen (%s)\n", curl_easy_strerror(res));
		return false;
	}
	return true;
}


int main()
{
	// Zeilenweise Pufferung einstellen, für den Fall das stdout/stderr
	// nicht mit einem Terminal verbunden sind. Dies ist z.B. der Fall wenn
	// dieses Programm als Dienst automatisch beim Booten gestartet wird.
	setlinebuf(stdout);
	setlinebuf(stderr);

	if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
		fprintf(stderr, "Initialisierung von curl fehlgeschlagen!\n");
		return 1;
	}

	char *i2c_bus = "/dev/i2c-1";
	unsigned int slave_addr = 0x18; 

	for (;;) {

		CURL *curl = create_influxdb_curl_handle();
		if (curl == NULL) {
			fprintf(stderr, "Erstellen des curl-Handles fehlgeschlagen!\n");
			sleep(5);
			continue;
		}

		int fd = mcp9808_open(i2c_bus, slave_addr);
		if (fd < 0) {
			fprintf(stderr, "Öffnen des MCP9808 fehlgeschlagen: %s\n",
				strerror(errno));
			sleep(5);
			continue;
		}
			
		float temp=0;
		struct timespec timestamp;

		for (;;) {

			clock_gettime(CLOCK_REALTIME, &timestamp);

			if (mcp9808_get_temp(fd, &temp) < 0) {
				fprintf(stderr, "Temperatur auslesen fehlgeschlagen: %s\n",
					strerror(errno));
				break;
			}

			if (!insert_temp(curl, temp, timestamp)) {
				break;
			}
			sleep(5);
		}

		sleep(5);
		curl_easy_cleanup(curl);
		close(fd);
	}
}
