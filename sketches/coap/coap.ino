// Based on examples:
//     "DHT20.ino" by Rob Tillaart
//     "WiFiClient" from ESP32 Dev Module board Manager
//     "coaptest.ino" by Hirotaka Niisato
//     "esp32.ino" by Hirotaka Niisato

#include <DHT20.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>


DHT20 DHT;
WiFiUDP udp;
Coap coap(udp);

const uint16_t updatePeriodMillis = 1000;
const uint16_t publishPeriodSecs = 60;
uint16_t lastPublishSecs = publishPeriodSecs;

int status = DHT20_OK;
float temperature = 0.0f;
float humidity = 0.0f;

const unsigned long serialBaud = 115200;
const uint8_t serialHeaderFreq = 10;
uint8_t serialBeforeHeader = 0;

const char *wifiSSID = "<ssid>";			// removed for security reasons
const char *wifiPassword = "<password>";		// removed for security reasons

const char *coapServer = "<server-IP>";			// removed for security reasons
const uint16_t coapPort = 5683;
const char *coapResourceBase = "api/v1/<access-token>";	// removed for security reasons
uint16_t coapMessageID = 0;


void setup() {
    // Connect to Serial
    Serial.begin(serialBaud);
    while (!Serial) {
        delay(100);
    }

    Serial.println("Serial Connected");

    // Connect to DHT20
    connectDHT20();

    // Connect to WiFi
    connectWiFi();

    // Set up the CoAP client
    setupCoAP();

    delay(updatePeriodMillis);
}

void loop() {
    // Read data and print them to serial
    read();
    print();

    // Publish data
    if (millis() / 1000 - lastPublishSecs >= publishPeriodSecs) {
        publish();
        lastPublishSecs = millis() / 1000;
    }

    delay(updatePeriodMillis);
}

void connectDHT20() {
    Serial.println("DHT20 Connecting...");
    
    Wire.begin();
    DHT.begin();
    while (!DHT.isConnected()) {
        delay(100);
    }

    Serial.println("DHT20 Connected");
}

void connectWiFi() {
    Serial.println("WiFi Connecting...");

    WiFi.begin(wifiSSID, wifiPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    Serial.println("WiFi Connected");
}

void setupCoAP() {
    coap.start();

    Serial.println("CoAP Set Up");
}

void read() {
    status = DHT.read();
    temperature = DHT.getTemperature();
    humidity = DHT.getHumidity();
}

void print() {
    // Print header if necessary
    if (serialBeforeHeader == 0) {
        serialBeforeHeader = serialHeaderFreq;

        Serial.println();
        Serial.println("Temp (°C)\tHumidity (%)\tStatus");
    }

    serialBeforeHeader--;

    // Print data
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");

    switch (status) {
        case DHT20_OK:
            Serial.print("OK");
            break;
        case DHT20_ERROR_CHECKSUM:
            Serial.print("Checksum error");
            break;
        case DHT20_ERROR_CONNECT:
            Serial.print("Connect error");
            break;
        case DHT20_MISSING_BYTES:
            Serial.print("Missing bytes");
            break;
        case DHT20_ERROR_BYTES_ALL_ZERO:
            Serial.print("All bytes read zero");
            break;
        case DHT20_ERROR_READ_TIMEOUT:
            Serial.print("Read time out");
            break;
        case DHT20_ERROR_LASTREAD:
            Serial.print("Error read too fast");
            break;
        default:
            Serial.print("Unknown error");
            break;
    }
    
    Serial.print("\n");
}

void publish() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    char resource[COAP_BUF_MAX_SIZE];
    char payload[COAP_BUF_MAX_SIZE];
    
    Serial.println("Data Sending..");

    uint32_t tokenBase = esp_random();
    uint8_t token[2] = {tokenBase & 0xFF, (tokenBase >> 8) & 0xFF};
    sprintf(resource, "%s/telemetry", coapResourceBase);
    sprintf(payload, "{\"temperature\": %.1f, \"humidity\": %.1f}", temperature, humidity);
    coap.send(coapServer, COAP_DEFAULT_PORT, resource, COAP_NONCON, COAP_POST, token, 2,
              (uint8_t *) payload, strlen(payload), COAP_APPLICATION_JSON, coapMessageID);
    coapMessageID++;

    tokenBase = esp_random();
    token[0] = tokenBase & 0xFF;
    token[1] = (tokenBase >> 8) & 0xFF;
    sprintf(resource, "%s/attributes", coapResourceBase);
    sprintf(payload, "{\"status\": %d}", (status != DHT20_OK));
    coap.send(coapServer, COAP_DEFAULT_PORT, resource, COAP_NONCON, COAP_POST, token, 2,
              (uint8_t *) payload, strlen(payload), COAP_APPLICATION_JSON, coapMessageID);
    coapMessageID++;

    Serial.println("Data Sent");
}
