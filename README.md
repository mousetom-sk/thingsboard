# Thingsboard &ndash; Prototype Application

The client-side implementation of and the outputs from our prototype application for telemetry collection using ThingsBoard.

## Contents

There are two folders in this repository: 
- **sketches**, which contains the respective client-side INO implementations for MQTT and CoAP data injection, and
- **traces**, which holds extracts from the MQTT and CoAP message exchanges between the device and the server captured with Wireshark.

Note that the actual credentials and IP addresses were removed from the codes for security reasons.

## Dependencies

The implementation provided in this repository was tested with a ThingsBoard Community Edition server in version 3.8.1.

The sketches depend on the following third-party libraries for Arduino IDE:
- esp32 board manager (version 3.0.7),
- DHT20 by Rob Tillaart (version 0.3.1),
- WiFi and WiFiUdp from esp32,
- PubSubClient by Nick O’Leary (version 2.8),
- CoAP simple library by Hirotaka Niisato (version 1.3.28).
