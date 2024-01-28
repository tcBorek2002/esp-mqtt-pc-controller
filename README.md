# esp-mqtt-pc-controller
Code for an ESP32 that connects to an MQTT broker. Can be used to turn on the PC using WoL and pings the PC to communicate whether the PC is turned on. Makes use of WakeOnLan.

# Getting started
## Libraries
Install the following libraries:   
* PubSubClient by Nick
* WakeOnLan by a7md0
* [ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping)
* Ticker by Stefan Staub
## Config
Set the following variables in the main.ino file:   
* LOCATION: Any name to give to the esp client
* ssid[]: WiFi SSID
* pass[]: WiFi Password
* *mqtt_broker: Hostname of the MQTT broker
* *mqtt_username: Username for MQTT
* *mqtt_password: Password for MQTT
* mqtt_port: Port to use for MQTT
* *MACAddress: MAC Address to send WoL package to
* ip: IP Address to ping. Note the format. Example: (192, 168, 1, 10)
### Upload the code and enjoy
## Understanding the broker topics
Location is the name you have set in the configuration.   
The topics used are:
* **esp/{location}/status**: Used to indicate ESP status. ESP will send "signin" on startup or connection and ESP will send "checkin" every hour.
* **esp/{location}/poll**: Used to poll the ESP. ESP will send "poll-checkin" on the esp status topic. It will also send the latest PC status. This can be done to check whether the ESP is still online and to manually get the latest PC status.
* **pc/{location}/command**: Used to send the WoL package. Send "start" to this topic to send the WoL package.
* **pc/{location}/status**: Used to indicate PC status. Can be "online" or "offline". The ESP will ping the PC every 10 seconds. Note that an update is sent to the broker only when the state is changed.
