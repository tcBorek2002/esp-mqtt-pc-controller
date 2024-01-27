#include <WiFi.h>
#include <PubSubClient.h>
#include <WakeOnLan.h>
#include <WiFiUdp.h>
#include <ESP32Ping.h>
#include <Ticker.h>

// ESP Location name
#define LOCATION "name"
// WiFi credentials
char ssid[] = "ssid";
char pass[] = "password";
// MQTT Broker settings
const char *mqtt_broker = "mqtt.emphisia.nl";
const char *mqtt_topic_command = "pc/" LOCATION "/command";
const char *mqtt_topic_pc_status = "pc/" LOCATION "/status";
const char *mqtt_topic_esp_status = "esp/" LOCATION "/status";
const char *mqtt_username = "username";
const char *mqtt_password = "password";
const int mqtt_port = 1883;

// Don't change these
bool previousPingState = false;
bool firstRun = true;

const char *MACAddress = "macaddress"; // MAC address for WoL
IPAddress ip (192, 168, 1, x); // The PC ip to ping

Ticker timerPing;
Ticker timerCheckin;

// WiFi and MQTT client initialization
WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);
WiFiUDP UDP;

WakeOnLan WOL(UDP); // Pass WiFiUDP class

// Function Declarations
void connectToMQTT();

void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup() {
    Serial.begin(115200);
    Serial.println("BORTECH PRODUCT");
    connectToWiFi();

    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setKeepAlive(60);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTT();
    timerPing.attach(10, pingPc); // Ping pc every 10 seconds
    timerCheckin.attach(3600, sendCheckIn); // Send checkin msg every hour
}

void connectToWiFi() {
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());
}

void connectToMQTT() {
    while (!mqtt_client.connected()) {
        String client_id = "esp32-" LOCATION "-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
          if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
            mqtt_client.subscribe(mqtt_topic_command);
            mqtt_client.publish(mqtt_topic_esp_status, "signin");  // Publish message upon connection
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" Retrying in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    if(strcmp(topic, mqtt_topic_command) == 0) {
      String command = "";
      Serial.print("Received the following command: ");
      for (unsigned int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        command += (char) payload[i];
        if(i == length -1) {
          Serial.println("");
        }
      }
      if(command == "start") {
        Serial.println("Starting pc, sending WoL magic packet...");
        WOL.sendMagicPacket(MACAddress);
      }
    }
    else {
      Serial.print("Message: ");
      for (unsigned int i = 0; i < length; i++) {
          Serial.print((char) payload[i]);
      }
    }

    Serial.println("\n-----------------------");
}

void sendCheckIn() {
  mqtt_client.publish(mqtt_topic_esp_status, "checkin");
}

void pingPc()
{
  Serial.println("Pinging IP ...");
  bool currentPingState = Ping.ping(ip);

  if (currentPingState != previousPingState || firstRun) {
    // Ping state has changed
    Serial.println("Ping state changed.");

    if (currentPingState) {
      Serial.println("IP found. Sending online message");
      mqtt_client.publish(mqtt_topic_pc_status, "online");
    } else {
      Serial.println("IP NOT found. Sending offline message");
      mqtt_client.publish(mqtt_topic_pc_status, "offline");
    }

    previousPingState = currentPingState;  // Update the previous ping state
  }
  else {
    Serial.println("Ping state has not changed. Not sending message.");
  }
  firstRun = false;
  Serial.println("\n-----------------------");
}


void loop() {
    if (!mqtt_client.connected()) {
        connectToMQTT();
    }
    mqtt_client.loop();
}
