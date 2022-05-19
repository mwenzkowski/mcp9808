# Tools für den Temperatursensor MCP9808 von Microchip

Der [MCP9808](https://www.microchip.com/en-us/product/MCP9808)
([Datenblatt (EN)](https://ww1.microchip.com/downloads/en/DeviceDoc/MCP9808-0.5C-Maximum-Accuracy-Digital-Temperature-Sensor-Data-Sheet-DS20005095B.pdf))
ist ein Temperatursensor der über den seriellen Datenbus
[I²C](https://de.wikipedia.org/wiki/I%C2%B2C) angesprochen wird. Ich verwende
den Sensor zusammen mit einem
[Raspberry Pi](https://de.wikipedia.org/wiki/Raspberry_Pi) unter Linux.

Dieses Projekt enthält drei Tools:

1. `mcp9808_info`: Ein Kommandozeilen-Programm das die aktuell gemessene Temperatur und die Konfiguration des Sensors anzeigt.
2. `mcp9808_logger`: Ein Programm das als Hintergrundienst alle 5 Sekunden die
   Temperatur des Sensors abfragt und über die HTTP-API in die Datenbank
   [InfluxDB](https://www.influxdata.com/products/influxdb-overview/), eine
   Datenbank speziell für Zeitreihen, einträgt.
3. `gaerbox`: Dieses Programm konfiguriert den Sensor um ihn als ein Thermostat
   zu verwenden. Dazu benutze ich folgenden Aufbau: Eine Styroporbox mit einer
   Heizmatte auf dem Boden der Box. Der Temperatursensor ist mit einem Relais
   verbunden, welches die Heizmatte an- oder ausschaltet. Der Sensor befindet
   sich unter dem Deckel der Box. Diese "Gärbox" benutze ich für die Sauer- und
   Brotteig-Führung beim Brot backen.

Alle Tools nutzen die Dateien `mcp9808.h` und `mcp9808.c` als eigene Bibliothek
um den Temperatursensor zu steuern. Diese abstrahieren die Details der
Kommunikation mit dem Sensor über den I²C-Bus und können auch für andere
Projekte genutzt werden.
