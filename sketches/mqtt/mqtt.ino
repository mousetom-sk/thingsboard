// Based on examples:
//     "DHT20.ino" by Rob Tillaart
//     "WiFiClient" from ESP32 Dev Module board Manager
//     "mqtt_esp8266.ino" by Nick O'Leary

#include <DHT20.h>
#include <WiFi.h>
#include <PubSubClient.h>


DHT20 DHT;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const uint16_t updatePeriodMillis = 1000;
const uint16_t publishPeriodSecs = 60;
uint16_t lastPublishSecs = publishPeriodSecs;

int status = DHT20_OK;
float temperature = 0.0f;
float humidity = 0.0f;

const unsigned long serialBaud = 115200;
const uint8_t serialHeaderFreq = 10;
uint8_t serialBeforeHeader = 0;

const char *wifiSSID = "<ssid>";		// removed for security reasons
const char *wifiPassword = "<password>";	// removed for security reasons

const char *mqttServer = "<server-IP>";		// removed for security reasons
const uint16_t mqttPort = 1883;
const char *mqttTopicBase = "v1/devices/me";
const char *mqttClientID = "ESP32_DHT20";
const char *mqttUser = "<access-token>";	// removed for security reasons


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

    // Connect to ThingsBoard
    connectThingsBoard();

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

void connectThingsBoard() {
    Serial.println("ThingsBoard via MQTT Connecting...");

    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.connect(mqttClientID, mqttUser, "");

    while (!mqttClient.connected()) {
        delay(500);
    }

    Serial.println("ThingsBoard via MQTT Connected");
}

void reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    connectThingsBoard();
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
    if (!mqttClient.connected()) {
        reconnect();
    }

    char topic[MQTT_MAX_PACKET_SIZE];
    char payload[MQTT_MAX_PACKET_SIZE];
    
    Serial.println("Data Sending..");

    sprintf(topic, "%s/telemetry", mqttTopicBase);
    sprintf(payload, "{\"temperature\": %.1f, \"humidity\": %.1f}", temperature, humidity);
    mqttClient.publish(topic, payload);

    sprintf(topic, "%s/attributes", mqttTopicBase);
    sprintf(payload, "{\"status\": %d}", (status != DHT20_OK));
    mqttClient.publish(topic, payload);

    Serial.println("Data Sent");
}
