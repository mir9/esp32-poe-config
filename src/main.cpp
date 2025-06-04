#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

#define I2C_SDA 18
#define I2C_SCL 17

#define ETH_CS   14
#define ETH_RST  9
#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK  13


Adafruit_BME280 bme;
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
WebServer server(80);
Preferences prefs;

struct Config {
    String eth_ip = "192.168.1.50";
    String eth_gateway = "192.168.1.1";
    String eth_subnet = "255.255.255.0";
    bool mqtt_enabled = false;
    String mqtt_server = "192.168.1.100";
    uint16_t mqtt_port = 1883;
    String mqtt_topic = "esp32/data";
    uint32_t sampling_time_ms = 5000;
    String tag_temp = "Temperature1";
    String tag_press = "Pressure1";
    String tag_hum = "Humidity1";
} config;

void loadConfig() {
    prefs.begin("cfg", true);
    config.eth_ip = prefs.getString("ip", config.eth_ip);
    config.eth_gateway = prefs.getString("gw", config.eth_gateway);
    config.eth_subnet = prefs.getString("sn", config.eth_subnet);
    config.mqtt_enabled = prefs.getBool("mqtt_en", config.mqtt_enabled);
    config.mqtt_server = prefs.getString("mqtt_server", config.mqtt_server);
    config.mqtt_port = prefs.getUShort("mqtt_port", config.mqtt_port);
    config.mqtt_topic = prefs.getString("mqtt_topic", config.mqtt_topic);
    config.sampling_time_ms = prefs.getUInt("sample", config.sampling_time_ms);
    config.tag_temp = prefs.getString("tag_temp", config.tag_temp);
    config.tag_press = prefs.getString("tag_press", config.tag_press);
    config.tag_hum = prefs.getString("tag_hum", config.tag_hum);
    prefs.end();
}

void saveConfig() {
    prefs.begin("cfg", false);
    prefs.putString("ip", config.eth_ip);
    prefs.putString("gw", config.eth_gateway);
    prefs.putString("sn", config.eth_subnet);
    prefs.putBool("mqtt_en", config.mqtt_enabled);
    prefs.putString("mqtt_server", config.mqtt_server);
    prefs.putUShort("mqtt_port", config.mqtt_port);
    prefs.putString("mqtt_topic", config.mqtt_topic);
    prefs.putUInt("sample", config.sampling_time_ms);
    prefs.putString("tag_temp", config.tag_temp);
    prefs.putString("tag_press", config.tag_press);
    prefs.putString("tag_hum", config.tag_hum);
    prefs.end();
}

void handleRoot() {
    String page = "<html><body><h1>ESP32 Config</h1><form method='POST' action='/save'>";
    page += "Ethernet IP: <input name='ip' value='" + config.eth_ip + "'><br>";
    page += "Gateway: <input name='gw' value='" + config.eth_gateway + "'><br>";
    page += "Subnet: <input name='sn' value='" + config.eth_subnet + "'><br>";
    page += "MQTT Enabled: <input type='checkbox' name='mqtt_en'" + String(config.mqtt_enabled?" checked":"") + "><br>";
    page += "MQTT Server: <input name='mqtt_server' value='" + config.mqtt_server + "'><br>";
    page += "MQTT Port: <input name='mqtt_port' value='" + String(config.mqtt_port) + "'><br>";
    page += "MQTT Topic: <input name='mqtt_topic' value='" + config.mqtt_topic + "'><br>";
    page += "Sampling Time(ms): <input name='sample' value='" + String(config.sampling_time_ms) + "'><br>";
    page += "Temp Tag: <input name='tag_temp' value='" + config.tag_temp + "'><br>";
    page += "Press Tag: <input name='tag_press' value='" + config.tag_press + "'><br>";
    page += "Hum Tag: <input name='tag_hum' value='" + config.tag_hum + "'><br>";
    page += "<input type='submit' value='Save'></form></body></html>";
    server.send(200, "text/html", page);
}

void handleSave() {
    if(server.hasArg("ip")) config.eth_ip = server.arg("ip");
    if(server.hasArg("gw")) config.eth_gateway = server.arg("gw");
    if(server.hasArg("sn")) config.eth_subnet = server.arg("sn");
    config.mqtt_enabled = server.hasArg("mqtt_en");
    if(server.hasArg("mqtt_server")) config.mqtt_server = server.arg("mqtt_server");
    if(server.hasArg("mqtt_port")) config.mqtt_port = server.arg("mqtt_port").toInt();
    if(server.hasArg("mqtt_topic")) config.mqtt_topic = server.arg("mqtt_topic");
    if(server.hasArg("sample")) config.sampling_time_ms = server.arg("sample").toInt();
    if(server.hasArg("tag_temp")) config.tag_temp = server.arg("tag_temp");
    if(server.hasArg("tag_press")) config.tag_press = server.arg("tag_press");
    if(server.hasArg("tag_hum")) config.tag_hum = server.arg("tag_hum");
    saveConfig();
    server.sendHeader("Location", "/");
    server.send(303);
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload, len);
    if(err) return;
    if(doc.containsKey("ip")) config.eth_ip = doc["ip"].as<String>();
    if(doc.containsKey("gw")) config.eth_gateway = doc["gw"].as<String>();
    if(doc.containsKey("sn")) config.eth_subnet = doc["sn"].as<String>();
    if(doc.containsKey("mqtt_enabled")) config.mqtt_enabled = doc["mqtt_enabled"].as<bool>();
    if(doc.containsKey("mqtt_server")) config.mqtt_server = doc["mqtt_server"].as<String>();
    if(doc.containsKey("mqtt_port")) config.mqtt_port = doc["mqtt_port"].as<uint16_t>();
    if(doc.containsKey("mqtt_topic")) config.mqtt_topic = doc["mqtt_topic"].as<String>();
    if(doc.containsKey("sample")) config.sampling_time_ms = doc["sample"].as<uint32_t>();
    if(doc.containsKey("tag_temp")) config.tag_temp = doc["tag_temp"].as<String>();
    if(doc.containsKey("tag_press")) config.tag_press = doc["tag_press"].as<String>();
    if(doc.containsKey("tag_hum")) config.tag_hum = doc["tag_hum"].as<String>();
    saveConfig();
}

void connectMQTT() {
    if(!config.mqtt_enabled) return;
    mqttClient.setServer(config.mqtt_server.c_str(), config.mqtt_port);
    mqttClient.setCallback(mqttCallback);
    while(!mqttClient.connected()) {
        mqttClient.connect("esp32");
        delay(500);
    }
    mqttClient.subscribe("esp32/config");
}

unsigned long lastPublish = 0;

void setup() {
    Serial.begin(115200);
    loadConfig();

    // Start I2C and BME280
    Wire.begin(I2C_SDA, I2C_SCL);
    bme.begin(0x76);

    // Start Ethernet (W5500)
    pinMode(ETH_RST, OUTPUT);
    digitalWrite(ETH_RST, LOW);
    delay(100);
    digitalWrite(ETH_RST, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, ETH_CS);
    Ethernet.init(ETH_CS);
    byte mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    IPAddress ip, gw, sn;
    ip.fromString(config.eth_ip);
    gw.fromString(config.eth_gateway);
    sn.fromString(config.eth_subnet);
    Ethernet.begin(mac, ip, gw, gw, sn);

    // Start WiFi AP for config
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_CONFIG", "password");

    // Web server routes
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();

    connectMQTT();
}

void loop() {
    server.handleClient();
    if(config.mqtt_enabled && !mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();

    unsigned long now = millis();
    if(now - lastPublish > config.sampling_time_ms) {
        lastPublish = now;
        float t = bme.readTemperature();
        float p = bme.readPressure() / 100.0F;
        float h = bme.readHumidity();
        StaticJsonDocument<128> doc;
        doc[config.tag_temp] = t;
        doc[config.tag_press] = p;
        doc[config.tag_hum] = h;
        char buf[128];
        size_t n = serializeJson(doc, buf);
        if(config.mqtt_enabled) {
            mqttClient.publish(config.mqtt_topic.c_str(), buf, n);
        }
    }
}

