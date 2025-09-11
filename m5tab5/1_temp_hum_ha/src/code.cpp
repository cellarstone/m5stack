#include <M5Unified.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SensirionI2CScd4x.h>

// SDIO pins for WiFi communication with ESP32-C6
#define SDIO2_CLK GPIO_NUM_12
#define SDIO2_CMD GPIO_NUM_13
#define SDIO2_D0 GPIO_NUM_11
#define SDIO2_D1 GPIO_NUM_10
#define SDIO2_D2 GPIO_NUM_9
#define SDIO2_D3 GPIO_NUM_8
#define SDIO2_RST GPIO_NUM_15

// WiFi Configuration - UPDATE THESE VALUES
const char* ssid = "Cellarstone IoT";
const char* password = "Cllrs123IoT456";

// MQTT Configuration - UPDATE THESE VALUES
const char* mqtt_server = "192.168.2.176";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt_user";  // UPDATE with your MQTT username
const char* mqtt_password = "mqtt";  // UPDATE with your MQTT password

// Device identification
const char* device_name = "M5Tab5_No_1";
const char* device_id = "m5tab5_no_1";

// MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Button to republish discovery
unsigned long lastButtonCheck = 0;

// SCD40 sensor instance
SensirionI2CScd4x scd4x;

// Working I2C pins
const int GROVE_SDA = 53;
const int GROVE_SCL = 54;

// Sensor data
float temperature = 22.5;
float humidity = 45.0;
uint16_t co2 = 650;
float tempMin = 100, tempMax = -100;
float humMin = 100, humMax = 0;
uint16_t co2Min = 9999, co2Max = 0;

// History for graphs
const int HISTORY_SIZE = 60;
float tempHistory[HISTORY_SIZE] = {0};
float humHistory[HISTORY_SIZE] = {0};
uint16_t co2History[HISTORY_SIZE] = {0};
int historyIndex = 0;
bool historyFull = false;

// Display dimensions
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// Network status
bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastMqttPublish = 0;
unsigned long lastMqttReconnect = 0;

// Colors
#define BG_COLOR TFT_BLACK
#define GRID_COLOR 0x2104
#define TEXT_PRIMARY TFT_WHITE
#define TEXT_SECONDARY 0xB5B6
#define CO2_GOOD 0x07E0
#define CO2_MODERATE 0xFFE0
#define CO2_POOR 0xFBE0
#define CO2_BAD 0xF800
#define TEMP_COLD 0x04FF
#define TEMP_NORMAL 0x07E0
#define TEMP_WARM 0xFBE0
#define TEMP_HOT 0xF800
#define HUM_LOW 0xFBE0
#define HUM_NORMAL 0x07FF
#define HUM_HIGH 0x001F

uint16_t getTemperatureColor(float temp) {
    if (temp < 18) return TEMP_COLD;
    if (temp < 24) return TEMP_NORMAL;
    if (temp < 28) return TEMP_WARM;
    return TEMP_HOT;
}

uint16_t getHumidityColor(float hum) {
    if (hum < 30) return HUM_LOW;
    if (hum < 60) return HUM_NORMAL;
    return HUM_HIGH;
}

uint16_t getCO2Color(uint16_t co2) {
    if (co2 < 800) return CO2_GOOD;
    if (co2 < 1200) return CO2_MODERATE;
    if (co2 < 2000) return CO2_POOR;
    return CO2_BAD;
}

String getCO2Status(uint16_t co2) {
    if (co2 < 800) return "Excellent";
    if (co2 < 1200) return "Good";
    if (co2 < 2000) return "Moderate";
    return "Poor - Ventilate!";
}

void drawGauge(int cx, int cy, int radius, float value, float minVal, float maxVal, uint16_t color, const char* label) {
    // Draw outer circle
    M5.Display.drawCircle(cx, cy, radius, TEXT_SECONDARY);
    M5.Display.drawCircle(cx, cy, radius-1, TEXT_SECONDARY);
    
    // Draw arc based on value
    float angle = map(value * 100, minVal * 100, maxVal * 100, -135, 135);
    int segments = 20;
    for (int i = -135; i <= angle; i += 270/segments) {
        float rad = i * PI / 180;
        int x1 = cx + (radius - 10) * cos(rad);
        int y1 = cy + (radius - 10) * sin(rad);
        int x2 = cx + (radius - 5) * cos(rad);
        int y2 = cy + (radius - 5) * sin(rad);
        M5.Display.drawLine(x1, y1, x2, y2, color);
        M5.Display.drawLine(x1, y1+1, x2, y2+1, color);
    }
    
    // Draw value
    M5.Display.setTextColor(color);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(cx - 35, cy - 15);
    M5.Display.printf("%.1f", value);
    
    // Draw label
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(cx - strlen(label) * 6, cy + 30);
    M5.Display.print(label);
}

void drawGraph(int x, int y, int w, int h, float* data, int dataSize, uint16_t color, float minVal, float maxVal) {
    // Draw border
    M5.Display.drawRect(x, y, w, h, GRID_COLOR);
    
    // Draw grid lines
    for (int i = 1; i < 4; i++) {
        int yPos = y + (h * i / 4);
        M5.Display.drawLine(x, yPos, x + w, yPos, GRID_COLOR);
    }
    
    // Draw data
    int dataPoints = historyFull ? HISTORY_SIZE : historyIndex;
    if (dataPoints > 1) {
        for (int i = 1; i < dataPoints; i++) {
            int x1 = x + (i-1) * w / HISTORY_SIZE;
            int x2 = x + i * w / HISTORY_SIZE;
            
            float val1 = data[i-1];
            float val2 = data[i];
            
            int y1 = y + h - (int)((val1 - minVal) * h / (maxVal - minVal));
            int y2 = y + h - (int)((val2 - minVal) * h / (maxVal - minVal));
            
            if (y1 < y) y1 = y;
            if (y1 > y + h) y1 = y + h;
            if (y2 < y) y2 = y;
            if (y2 > y + h) y2 = y + h;
            
            M5.Display.drawLine(x1, y1, x2, y2, color);
            M5.Display.drawLine(x1, y1+1, x2, y2+1, color);
        }
    }
}

void updateHistory() {
    tempHistory[historyIndex] = temperature;
    humHistory[historyIndex] = humidity;
    co2History[historyIndex] = co2;
    
    historyIndex++;
    if (historyIndex >= HISTORY_SIZE) {
        historyIndex = 0;
        historyFull = true;
    }
    
    // Update min/max
    if (temperature < tempMin) tempMin = temperature;
    if (temperature > tempMax) tempMax = temperature;
    if (humidity < humMin) humMin = humidity;
    if (humidity > humMax) humMax = humidity;
    if (co2 < co2Min) co2Min = co2;
    if (co2 > co2Max) co2Max = co2;
}

void setupWiFi() {
    Serial.println("Setting up WiFi...");
    
    // Configure SDIO pins for ESP32-C6 communication
    WiFi.setPins(SDIO2_CLK, SDIO2_CMD, SDIO2_D0, SDIO2_D1, SDIO2_D2, SDIO2_D3, SDIO2_RST);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

void publishDiscovery() {
    if (!mqttConnected) return;
    
    JsonDocument doc;
    char topic[200];
    char payload[1024];
    char stateTopic[100];
    char availTopic[100];
    
    // Prepare common topics
    snprintf(stateTopic, sizeof(stateTopic), "homeassistant/sensor/%s/state", device_id);
    snprintf(availTopic, sizeof(availTopic), "homeassistant/sensor/%s/availability", device_id);
    
    // Temperature sensor discovery
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s_temperature/config", device_id);
    doc.clear();
    doc["name"] = "Temperature";
    doc["device_class"] = "temperature";
    doc["unit_of_measurement"] = "°C";
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availTopic;
    doc["value_template"] = "{{ value_json.temperature }}";
    doc["unique_id"] = String(device_id) + "_temperature";
    
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = device_id;
    device["name"] = device_name;
    device["model"] = "M5Tab5";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    } else {
        Serial.printf("FAILED to publish discovery: %s\n", topic);
    }
    delay(50);  // Small delay between messages
    
    // Humidity sensor discovery
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s_humidity/config", device_id);
    doc.clear();
    doc["name"] = "Humidity";
    doc["device_class"] = "humidity";
    doc["unit_of_measurement"] = "%";
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availTopic;
    doc["value_template"] = "{{ value_json.humidity }}";
    doc["unique_id"] = String(device_id) + "_humidity";
    
    device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = device_id;
    device["name"] = device_name;
    device["model"] = "M5Tab5";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    } else {
        Serial.printf("FAILED to publish discovery: %s\n", topic);
    }
    delay(50);  // Small delay between messages
    
    // CO2 sensor discovery
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s_co2/config", device_id);
    doc.clear();
    doc["name"] = "CO2";
    doc["device_class"] = "carbon_dioxide";
    doc["unit_of_measurement"] = "ppm";
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availTopic;
    doc["value_template"] = "{{ value_json.co2 }}";
    doc["unique_id"] = String(device_id) + "_co2";
    
    device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = device_id;
    device["name"] = device_name;
    device["model"] = "M5Tab5";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    } else {
        Serial.printf("FAILED to publish discovery: %s\n", topic);
    }
}

void connectMQTT() {
    if (!wifiConnected || millis() - lastMqttReconnect < 5000) return;
    
    lastMqttReconnect = millis();
    
    Serial.printf("Connecting to MQTT broker %s:%d...", mqtt_server, mqtt_port);
    String clientId = String(device_id) + "_" + String(random(0xffff), HEX);
    
    // Set last will message
    char willTopic[100];
    snprintf(willTopic, sizeof(willTopic), "homeassistant/sensor/%s/availability", device_id);
    
    bool connected = false;
    if (strlen(mqtt_user) > 0) {
        Serial.printf(" with auth (user: %s)...", mqtt_user);
        connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password, willTopic, 0, true, "offline");
    } else {
        Serial.print(" without auth...");
        connected = mqttClient.connect(clientId.c_str(), willTopic, 0, true, "offline");
    }
    
    if (connected) {
        mqttConnected = true;
        Serial.println(" connected!");
        
        // Publish availability
        mqttClient.publish(willTopic, "online", true);
        
        // Small delay to ensure connection is stable
        delay(100);
        
        // Publish discovery messages
        publishDiscovery();
    } else {
        int state = mqttClient.state();
        Serial.printf(" failed, rc=%d\n", state);
        switch(state) {
            case -4: Serial.println("  MQTT_CONNECTION_TIMEOUT - server didn't respond"); break;
            case -3: Serial.println("  MQTT_CONNECTION_LOST - network connection broken"); break;
            case -2: Serial.println("  MQTT_CONNECT_FAILED - network connection failed"); break;
            case -1: Serial.println("  MQTT_DISCONNECTED - client is disconnected"); break;
            case 1: Serial.println("  MQTT_CONNECT_BAD_PROTOCOL - server doesn't support MQTT 3.1.1"); break;
            case 2: Serial.println("  MQTT_CONNECT_BAD_CLIENT_ID - server rejected client ID"); break;
            case 3: Serial.println("  MQTT_CONNECT_UNAVAILABLE - server unavailable"); break;
            case 4: Serial.println("  MQTT_CONNECT_BAD_CREDENTIALS - username/password rejected"); break;
            case 5: Serial.println("  MQTT_CONNECT_UNAUTHORIZED - not authorized (check if broker requires auth)"); break;
        }
    }
}

void publishSensorData() {
    if (!mqttConnected || millis() - lastMqttPublish < 5000) return;
    
    lastMqttPublish = millis();
    
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["co2"] = co2;
    
    char payload[256];
    serializeJson(doc, payload);
    
    char topic[100];
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s/state", device_id);
    
    if (mqttClient.publish(topic, payload)) {
        Serial.printf("Published to %s: %s\n", topic, payload);
    } else {
        Serial.println("Failed to publish sensor data!");
    }
}

void updateDisplay() {
    // Clear main area
    M5.Display.fillRect(0, 100, SCREEN_WIDTH, SCREEN_HEIGHT - 100, BG_COLOR);
    
    // Draw gauges
    int gaugeY = 200;
    int gaugeRadius = 90;
    
    // Temperature gauge
    drawGauge(250, gaugeY, gaugeRadius, temperature, 0, 40, getTemperatureColor(temperature), "Temp °C");
    
    // Humidity gauge
    drawGauge(500, gaugeY, gaugeRadius, humidity, 0, 100, getHumidityColor(humidity), "Humidity %");
    
    // CO2 display - larger box
    int co2X = 800;
    M5.Display.fillRoundRect(co2X - 100, gaugeY - 90, 250, 180, 15, getCO2Color(co2));
    M5.Display.fillRoundRect(co2X - 95, gaugeY - 85, 240, 170, 12, BG_COLOR);
    
    M5.Display.setTextColor(getCO2Color(co2));
    M5.Display.setTextSize(6);
    M5.Display.setCursor(co2X - 70, gaugeY - 40);
    M5.Display.printf("%d", co2);
    
    M5.Display.setTextSize(3);
    M5.Display.setCursor(co2X - 30, gaugeY + 20);
    M5.Display.print("ppm");
    
    M5.Display.setTextSize(2);
    M5.Display.setCursor(co2X - 60, gaugeY + 60);
    M5.Display.print(getCO2Status(co2));
    
    // Min/Max values
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setTextSize(2);
    
    M5.Display.setCursor(160, 320);
    M5.Display.printf("Min: %.1f°C | Max: %.1f°C", tempMin, tempMax);
    
    M5.Display.setCursor(420, 320);
    M5.Display.printf("Min: %.0f%% | Max: %.0f%%", humMin, humMax);
    
    M5.Display.setCursor(720, 320);
    M5.Display.printf("Min: %d | Max: %d ppm", co2Min, co2Max);
    
    // Graphs
    int graphY = 380;
    int graphHeight = 120;
    int graphWidth = 350;
    
    // Temperature graph
    M5.Display.setTextColor(getTemperatureColor(temperature));
    M5.Display.setTextSize(2);
    M5.Display.setCursor(50, graphY - 25);
    M5.Display.print("Temperature (5 min)");
    drawGraph(50, graphY, graphWidth, graphHeight, tempHistory, HISTORY_SIZE, getTemperatureColor(temperature), 15, 35);
    
    // Humidity graph
    M5.Display.setTextColor(getHumidityColor(humidity));
    M5.Display.setCursor(450, graphY - 25);
    M5.Display.print("Humidity (5 min)");
    drawGraph(450, graphY, graphWidth, graphHeight, humHistory, HISTORY_SIZE, getHumidityColor(humidity), 0, 100);
    
    // CO2 graph
    M5.Display.setTextColor(getCO2Color(co2));
    M5.Display.setCursor(850, graphY - 25);
    M5.Display.print("CO2 (5 min)");
    float co2Float[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; i++) co2Float[i] = co2History[i];
    drawGraph(850, graphY, graphWidth, graphHeight, co2Float, HISTORY_SIZE, getCO2Color(co2), 400, 2000);
    
    // Recommendations
    int statusY = 540;
    M5.Display.fillRect(0, statusY, SCREEN_WIDTH, 100, 0x0841);
    
    M5.Display.setTextColor(TEXT_PRIMARY);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(100, statusY + 20);
    
    if (co2 > 1200) {
        M5.Display.setTextColor(CO2_BAD);
        M5.Display.print("Open windows for fresh air!");
    } else if (humidity < 30) {
        M5.Display.setTextColor(HUM_LOW);
        M5.Display.print("Air is dry - add moisture");
    } else if (humidity > 60) {
        M5.Display.setTextColor(HUM_HIGH);
        M5.Display.print("High humidity - ventilate");
    } else if (temperature < 18) {
        M5.Display.setTextColor(TEMP_COLD);
        M5.Display.print("Too cold - increase heating");
    } else if (temperature > 28) {
        M5.Display.setTextColor(TEMP_HOT);
        M5.Display.print("Too warm - cooling needed");
    } else {
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.print("Conditions are optimal!");
    }
    
    // Status info
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(50, statusY + 60);
    M5.Display.printf("Next update in %d seconds", 5 - ((millis() % 5000) / 1000));
    
    // Button hint
    if (mqttConnected) {
        M5.Display.setCursor(350, statusY + 60);
        M5.Display.setTextColor(TEXT_SECONDARY);
        M5.Display.print("[Press screen to resend discovery]");
    }
    
    // Network status
    M5.Display.setCursor(700, statusY + 60);
    if (wifiConnected) {
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.print("WiFi: ");
        M5.Display.print(WiFi.localIP());
        if (mqttConnected) {
            M5.Display.print(" | MQTT: Connected");
        } else {
            M5.Display.setTextColor(TFT_ORANGE);
            M5.Display.print(" | MQTT: Disconnected");
        }
    } else {
        M5.Display.setTextColor(TFT_ORANGE);
        M5.Display.print("WiFi: Disconnected");
    }
}

void setup() {
    // Initialize M5Stack
    auto cfg = M5.config();
    cfg.external_spk = false;
    M5.begin(cfg);
    
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BG_COLOR);
    
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== M5Tab5 Environmental Monitor ===");
    Serial.println("ESP32-P4 + ESP32-C6 WiFi Version");
    
    // Title
    M5.Display.setTextColor(TEXT_PRIMARY);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(50, 30);
    M5.Display.print("Environmental Monitor");
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setCursor(750, 40);
    M5.Display.print("(Home Assistant Ready)");
    
    // Initialize I2C
    Wire.begin(GROVE_SDA, GROVE_SCL);
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setCursor(50, 80);
    M5.Display.print("Initializing SCD40 sensor...");
    
    // Initialize SCD40
    scd4x.begin(Wire);
    
    uint16_t error;
    error = scd4x.stopPeriodicMeasurement();
    delay(500);
    
    uint16_t serial0, serial1, serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (!error) {
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.setCursor(400, 80);
        M5.Display.printf("SCD40: %04x%04x%04x", serial0, serial1, serial2);
        Serial.printf("SCD40 Serial: %04x%04x%04x\n", serial0, serial1, serial2);
    } else {
        M5.Display.setTextColor(CO2_BAD);
        M5.Display.setCursor(400, 80);
        M5.Display.print("SCD40: Not found");
    }
    
    error = scd4x.startPeriodicMeasurement();
    
    // Setup WiFi
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setCursor(50, 120);
    M5.Display.print("Connecting to WiFi...");
    setupWiFi();
    
    if (wifiConnected) {
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.setCursor(300, 120);
        M5.Display.print("Connected: ");
        M5.Display.print(WiFi.localIP());
        
        // Setup MQTT with larger buffer for discovery messages
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setBufferSize(1024);  // Increase buffer size for discovery payloads
        connectMQTT();
        
        if (mqttConnected) {
            M5.Display.setTextColor(CO2_GOOD);
            M5.Display.setCursor(600, 120);
            M5.Display.print("MQTT: Connected");
        }
    } else {
        M5.Display.setTextColor(TFT_ORANGE);
        M5.Display.setCursor(300, 120);
        M5.Display.print("Offline Mode");
    }
    
    delay(2000);
    
    // Initial display
    updateDisplay();
}

void loop() {
    static unsigned long lastUpdate = 0;
    
    M5.update();
    
    // Check for button press to republish discovery
    if (M5.BtnA.wasPressed() && mqttConnected) {
        Serial.println("Button pressed - republishing discovery messages...");
        publishDiscovery();
        M5.Display.fillRect(400, 650, 400, 50, BG_COLOR);
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(400, 660);
        M5.Display.print("Discovery sent!");
        delay(1000);
    }
    
    // Handle MQTT connection
    if (wifiConnected) {
        if (!mqttConnected) {
            connectMQTT();
        } else {
            mqttClient.loop();
        }
    }
    
    // Update every 5 seconds
    if (millis() - lastUpdate > 5000) {
        lastUpdate = millis();
        
        // Read sensor
        uint16_t error;
        bool isDataReady = false;
        
        error = scd4x.getDataReadyFlag(isDataReady);
        if (!error && isDataReady) {
            float newTemp, newHum;
            uint16_t newCO2;
            
            error = scd4x.readMeasurement(newCO2, newTemp, newHum);
            if (!error && newCO2 > 0) {
                temperature = newTemp;
                humidity = newHum;
                co2 = newCO2;
                
                updateHistory();
                
                Serial.printf("T=%.1f°C, H=%.1f%%, CO2=%d ppm\n", temperature, humidity, co2);
                
                // Publish to MQTT if connected
                publishSensorData();
            }
        }
        
        // Update display
        updateDisplay();
    }
    
    delay(100);
}