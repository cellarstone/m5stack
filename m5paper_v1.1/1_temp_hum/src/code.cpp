#include <M5EPD.h>
#include <SensirionI2CScd4x.h>
#include <Preferences.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi Configuration - UPDATE THESE VALUES
const char* ssid = "Cellarstone IoT";
const char* password = "Cllrs123IoT456";

// MQTT Configuration - UPDATE THESE VALUES
const char* mqtt_server = "192.168.2.176";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt_user";
const char* mqtt_password = "mqtt";

// Device identification
const char* device_name = "M5Paper_no_1";
const char* device_id = "m5paper_no_1";

// MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Network status
bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastMqttPublish = 0;
unsigned long lastMqttReconnect = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastMqttCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 30000;  // Check WiFi every 30 seconds
const unsigned long MQTT_CHECK_INTERVAL = 30000;  // Check MQTT every 30 seconds

// M5Paper Port A (Grove) pins - External I2C
const int PortA_SDA = 25;  // Yellow wire - Port A SDA for M5Paper
const int PortA_SCL = 32;  // White wire - Port A SCL for M5Paper

// M5Paper native resolution is 960x540 (landscape)
// We'll use it in portrait by rotating
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;

SensirionI2CScd4x scd4x;
M5EPD_Canvas canvas(&M5.EPD);
Preferences preferences;
bool invertDisplay = false;  // Flag for visual update indicator
bool calibrationMode = false;
unsigned long calibrationStartTime = 0;
const unsigned long CALIBRATION_DURATION = 15000;  // 15 seconds for testing (was 180000 for 3 minutes)

// Sensor data for MQTT
float lastTemperature = 0;
float lastHumidity = 0;
uint16_t lastCO2 = 0;

void setupWiFi() {
    Serial.println("Setting up WiFi...");
    
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
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
    }
}

void checkWiFiConnection() {
    if (millis() - lastWifiCheck < WIFI_CHECK_INTERVAL) return;
    lastWifiCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        mqttConnected = false;  // If WiFi is down, MQTT is also down
        Serial.println("WiFi disconnected! Attempting to reconnect...");
        
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(ssid, password);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("\nWiFi reconnected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nWiFi reconnection failed!");
        }
    } else {
        wifiConnected = true;
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
    device["model"] = "M5Paper";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    }
    delay(50);
    
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
    device["model"] = "M5Paper";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    }
    delay(50);
    
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
    device["model"] = "M5Paper";
    device["manufacturer"] = "M5Stack";
    
    serializeJson(doc, payload);
    if (mqttClient.publish(topic, payload, true)) {
        Serial.printf("Published discovery: %s\n", topic);
    }
}

void checkMQTTConnection() {
    if (!wifiConnected) {
        mqttConnected = false;
        return;
    }
    
    // Periodically check if MQTT is really connected
    if (millis() - lastMqttCheck > MQTT_CHECK_INTERVAL) {
        lastMqttCheck = millis();
        if (!mqttClient.connected()) {
            mqttConnected = false;
            Serial.println("MQTT connection lost!");
        }
    }
}

void connectMQTT() {
    if (!wifiConnected || millis() - lastMqttReconnect < 5000) return;
    
    // Double-check MQTT is not already connected
    if (mqttClient.connected()) {
        mqttConnected = true;
        return;
    }
    
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
    }
}

void publishSensorData() {
    // Check both flag and actual connection state
    if (!mqttConnected || !mqttClient.connected() || millis() - lastMqttPublish < 5000) return;
    
    lastMqttPublish = millis();
    
    JsonDocument doc;
    doc["temperature"] = lastTemperature;
    doc["humidity"] = round(lastHumidity);
    doc["co2"] = lastCO2;
    
    char payload[256];
    serializeJson(doc, payload);
    
    char topic[100];
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s/state", device_id);
    
    if (mqttClient.publish(topic, payload)) {
        Serial.printf("Published to %s: %s\n", topic, payload);
    } else {
        Serial.println("Failed to publish sensor data!");
        // Force reconnection on next loop
        mqttConnected = false;
    }
}

void performCalibration() {
    uint16_t error;
    char errorMessage[256];
    uint16_t frcCorrection;
    
    // Stop periodic measurement before calibration
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error stopping measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    delay(500);
    
    // Perform forced recalibration to 420 ppm (current outdoor CO2 level)
    error = scd4x.performForcedRecalibration(420, frcCorrection);
    
    if (error) {
        Serial.print("Calibration failed: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("Calibration", 100, 200);
        canvas.drawString("FAILED!", 100, 250);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    } else {
        Serial.printf("Calibration successful! Set to 420 ppm, correction: %d\n", frcCorrection);
        
        // Save calibration timestamp
        preferences.begin("scd40", false);
        preferences.putULong("lastCalib", millis());
        preferences.end();
        
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("Calibration", 100, 200);
        canvas.drawString("SUCCESS!", 100, 250);
        canvas.drawString("420 ppm set", 100, 300);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    }
    
    delay(2000);
    
    // Restart periodic measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error restarting measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    
    calibrationMode = false;
}

void setup() {
    // Initialize M5Paper
    M5.begin();
    M5.EPD.SetRotation(0);  // Native landscape orientation
    M5.EPD.Clear(true);
    
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== SCD40 Temperature & Humidity Monitor (M5Paper v1.1) ===");
    
    // Create canvas in native resolution
    canvas.createCanvas(960, 540);
    canvas.setTextSize(3);
    
    // Show initialization message
    canvas.fillCanvas(0);
    canvas.drawString("Initializing SCD40...", 50, 200);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
    // Initialize I2C on Port A - M5Paper uses Wire1 for external I2C
    Wire1.begin(PortA_SDA, PortA_SCL);
    Serial.printf("I2C initialized on Port A using Wire1 (SDA=%d, SCL=%d)\n", PortA_SDA, PortA_SCL);
    
    // Scan for I2C devices on external bus
    Serial.println("Scanning for I2C devices on Port A...");
    for (int address = 1; address < 127; address++) {
        Wire1.beginTransmission(address);
        int error = Wire1.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
        }
    }
    Serial.println("I2C scan complete.");
    
    // Initialize SCD40 sensor
    uint16_t error;
    char errorMessage[256];
    
    scd4x.begin(Wire1);  // Use Wire1 for external I2C
    
    // Stop any potentially running measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error stopping measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    delay(500);
    
    // Disable Automatic Self-Calibration for better accuracy
    error = scd4x.setAutomaticSelfCalibration(0);
    if (error) {
        Serial.print("Error disabling ASC: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    
    // Start periodic measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error starting measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("SCD40 Failed!", 50, 200);
        canvas.drawString("Check wiring", 50, 260);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
        while(1) delay(1000); // Stop here if init failed
    }
    
    Serial.println("SCD40 initialized successfully!");
    Serial.println("Waiting for first measurement (5 seconds)...");
    
    canvas.fillCanvas(0);
    canvas.setTextSize(4);
    canvas.drawString("SCD40 Ready!", 50, 200);
    canvas.drawString("Waiting for data...", 50, 260);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
    // Wait for first measurement
    delay(5000);
    
    // Check last calibration time
    preferences.begin("scd40", true);
    unsigned long lastCalib = preferences.getULong("lastCalib", 0);
    preferences.end();
    
    if (lastCalib == 0) {
        Serial.println("Note: Sensor has never been calibrated");
        Serial.println("Press center button for 3 seconds to start calibration");
    } else {
        Serial.printf("Last calibration: %lu ms ago\n", millis() - lastCalib);
    }
    
    // Setup WiFi and MQTT
    canvas.fillCanvas(0);
    canvas.setTextSize(3);
    canvas.drawString("Connecting to WiFi...", 50, 200);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
    setupWiFi();
    
    if (wifiConnected) {
        canvas.fillCanvas(0);
        canvas.setTextSize(3);
        canvas.drawString("WiFi Connected!", 50, 200);
        char ipStr[32];
        snprintf(ipStr, sizeof(ipStr), "IP: %s", WiFi.localIP().toString().c_str());
        canvas.drawString(ipStr, 50, 250);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
        // Setup MQTT
        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setBufferSize(1024);
        connectMQTT();
        
        if (mqttConnected) {
            canvas.drawString("MQTT: Connected to Home Assistant", 50, 300);
        } else {
            canvas.drawString("MQTT: Failed to connect", 50, 300);
        }
    } else {
        canvas.fillCanvas(0);
        canvas.setTextSize(3);
        canvas.drawString("WiFi: Connection failed", 50, 200);
        canvas.drawString("Running in offline mode", 50, 250);
    }
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    delay(3000);
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    
    // Check WiFi connection periodically
    checkWiFiConnection();
    
    // Check and handle MQTT connection
    if (wifiConnected) {
        checkMQTTConnection();  // Check if MQTT is really connected
        
        if (!mqttConnected || !mqttClient.connected()) {
            mqttConnected = false;
            connectMQTT();
        } else {
            mqttClient.loop();
        }
    }
    
    // Check wheel control for calibration mode
    // M5Paper has a wheel: rotate UP/Left (BtnL/G37), push (BtnP/G38), rotate DOWN/Right (BtnR/G39)
    M5.update();
    if (M5.BtnL.wasPressed() && !calibrationMode) {  // Wheel UP/Left for calibration
        Serial.println("Wheel UP pressed - entering calibration mode");
        calibrationMode = true;
        calibrationStartTime = millis();
        
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("CALIBRATION MODE", 250, 100);
        canvas.drawString("Take device outside!", 230, 180);
        canvas.drawString("Wait 15 seconds", 280, 260);
        canvas.drawString("Rotate wheel DOWN", 260, 340);
        canvas.drawString("to calibrate", 320, 400);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    }
    
    // Handle calibration mode
    if (calibrationMode) {
        unsigned long elapsed = millis() - calibrationStartTime;
        
        // Update countdown display
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("CALIBRATION MODE", 250, 80);
        canvas.drawString("Take outside!", 300, 160);
        canvas.drawString("Stabilizing...", 320, 240);
        
        char timeStr[32];
        unsigned long remaining = (CALIBRATION_DURATION - elapsed) / 1000;
        snprintf(timeStr, sizeof(timeStr), "Wait: %lu seconds", remaining);
        canvas.drawString(timeStr, 280, 320);
        
        if (elapsed >= CALIBRATION_DURATION) {
            canvas.setTextSize(6);
            canvas.drawString("READY!", 360, 380);
            canvas.setTextSize(3);
            canvas.drawString("Rotate wheel DOWN to calibrate", 220, 450);
            canvas.drawString("or PUSH wheel to cancel", 260, 490);
            
            // Check if wheel rotated DOWN/Right to confirm calibration
            if (M5.BtnR.wasPressed()) {
                Serial.println("Wheel DOWN pressed - starting calibration countdown");
                // Show 15 second countdown before calibration
                for (int i = 15; i > 0; i--) {
                    canvas.fillCanvas(0);
                    canvas.setTextSize(4);
                    canvas.drawString("CALIBRATING", 320, 150);
                    canvas.drawString("Keep outside!", 300, 230);
                    canvas.setTextSize(8);
                    char countStr[32];
                    snprintf(countStr, sizeof(countStr), "%d", i);
                    canvas.drawString(countStr, 440, 300);
                    canvas.setTextSize(3);
                    canvas.drawString("seconds left", 360, 400);
                    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
                    delay(1000);
                }
                performCalibration();
            }
            
            // Allow canceling by pushing the wheel
            if (M5.BtnP.wasPressed()) {
                Serial.println("Calibration cancelled");
                calibrationMode = false;
                canvas.fillCanvas(0);
                canvas.setTextSize(4);
                canvas.drawString("Calibration", 300, 200);
                canvas.drawString("CANCELLED", 300, 260);
                canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
                delay(2000);
            }
        }
        
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        delay(1000);
        return;  // Skip normal measurement during calibration
    }
    
    // Check if data is ready
    bool isDataReady = false;
    error = scd4x.getDataReadyFlag(isDataReady);
    
    if (error) {
        Serial.print("Error checking data ready: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        delay(1000);
        return;
    }
    
    if (!isDataReady) {
        // Data not ready yet, wait a bit
        delay(100);
        return;
    }
    
    // Read measurement
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    
    error = scd4x.readMeasurement(co2, temperature, humidity);
    
    if (error) {
        Serial.print("Error reading measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        
        canvas.fillCanvas(0);
        canvas.setTextSize(4);
        canvas.drawString("Read Error!", 50, 200);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
    } else if (co2 == 0) {
        Serial.println("Invalid sample (CO2=0), skipping...");
        
    } else {
        // Valid measurement received
        Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%, CO2: %d ppm\n", 
                     temperature, humidity, co2);
        
        // Store values for MQTT
        lastTemperature = temperature;
        lastHumidity = humidity;
        lastCO2 = co2;
        
        // Publish to MQTT if connected
        publishSensorData();
        
        // Force a full display clear on first valid reading to remove any artifacts
        static bool firstReading = true;
        if (firstReading) {
            M5.EPD.Clear(true);
            delay(100);
            firstReading = false;
        }
        
        // Clear canvas completely with white
        canvas.fillCanvas(0);
        canvas.fillRect(0, 0, 960, 540, 0);  // Ensure full clear
        
        // Draw dividing lines for visual separation in landscape
        canvas.drawLine(280, 0, 280, 540, 15);   // Vertical line - smaller left section
        canvas.drawLine(560, 0, 560, 540, 15);   // Vertical line - smaller middle section
        
        // Temperature - Left section (smaller)
        canvas.setTextSize(3);
        canvas.drawString("TEMPERATURE", 40, 100);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f", temperature);
        canvas.setTextSize(7);  // Bigger text for temperature
        canvas.drawString(tempStr, 50, 200);
        canvas.setTextSize(3);
        canvas.drawString("°C", 210, 270);
        
        // Humidity - Middle section (smaller)
        canvas.setTextSize(3);
        canvas.drawString("HUMIDITY", 340, 100);
        char humStr[10];
        snprintf(humStr, sizeof(humStr), "%d", (int)round(humidity));
        canvas.setTextSize(7);  // Bigger text for humidity
        canvas.drawString(humStr, 340, 200);
        canvas.setTextSize(3);
        canvas.drawString("%", 500, 270);
        
        // CO2 - Right section (bigger - primary focus)
        canvas.setTextSize(5);
        canvas.drawString("CO2", 690, 90);  // Better aligned with temp/humidity titles
        char co2Str[10];
        snprintf(co2Str, sizeof(co2Str), "%d", co2);
        canvas.setTextSize(14);  // Extra large text for CO2
        int co2X = 760 - (strlen(co2Str) * 45) / 2;  // Properly center in right section (560-960)
        canvas.drawString(co2Str, co2X, 195);  // Move down to better align with other values
        canvas.setTextSize(3);  // Smaller text like other units
        canvas.drawString("ppm", 860, 270);  // Aligned right like °C and %
        
        // Add instruction at bottom
        canvas.setTextSize(2);
        canvas.drawString("Rotate wheel UP for calibration", 350, 500);
        
        // Network status indicator at bottom right
        if (mqttConnected) {
            canvas.setTextSize(2);
            canvas.drawString("HA: Connected", 750, 510);
            canvas.fillCircle(730, 516, 5, 15);  // Connected dot
        } else if (wifiConnected) {
            canvas.setTextSize(2);
            canvas.drawString("WiFi Only", 780, 510);
        } else {
            canvas.setTextSize(2);
            canvas.drawString("Offline", 820, 510);
        }
        
        // Push the updated canvas to display
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
        // Set flag to show inverted display on next update
        invertDisplay = true;
    }
    
    // Wait 5 seconds before next reading (E-ink displays are slower to update)
    delay(5000);
}