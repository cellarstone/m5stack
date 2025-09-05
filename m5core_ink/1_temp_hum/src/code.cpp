#include <M5CoreInk.h>
#include <SensirionI2CScd4x.h>

// M5CoreInk Port A (Grove) pins
const int PortA_SDA = 32;  // Yellow wire
const int PortA_SCL = 33;  // White wire

SensirionI2CScd4x scd4x;
Ink_Sprite InkPageSprite(&M5.M5Ink);

void setup() {
    // Initialize M5CoreInk
    M5.begin();
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== SCD40 Temperature & Humidity Monitor (M5CoreInk) ===");
    
    if (!M5.M5Ink.isInit()) {
        Serial.println("Ink Init Failed");
        while (1) delay(100);
    }
    
    M5.M5Ink.clear();
    delay(1000);
    
    // Create sprite for display
    InkPageSprite.creatSprite(0, 0, 200, 200);
    
    // Show initialization message
    InkPageSprite.clear();
    InkPageSprite.drawString(10, 50, "Init SCD40...", &AsciiFont8x16);
    InkPageSprite.pushSprite();
    
    // Initialize I2C on Port A
    Wire.begin(PortA_SDA, PortA_SCL);
    Serial.printf("I2C initialized on Port A (SDA=%d, SCL=%d)\n", PortA_SDA, PortA_SCL);
    
    // Initialize SCD40 sensor
    uint16_t error;
    char errorMessage[256];
    
    scd4x.begin(Wire);
    
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
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 50, "SCD40 Failed!", &AsciiFont8x16);
        InkPageSprite.drawString(10, 70, "Check wiring", &AsciiFont8x16);
        InkPageSprite.pushSprite();
        
        while(1) delay(1000); // Stop here if init failed
    }
    
    Serial.println("SCD40 initialized successfully!");
    Serial.println("Waiting for first measurement (5 seconds)...");
    
    InkPageSprite.clear();
    InkPageSprite.drawString(10, 50, "SCD40 Ready!", &AsciiFont8x16);
    InkPageSprite.drawString(10, 70, "Waiting...", &AsciiFont8x16);
    InkPageSprite.pushSprite();
    
    // Wait for first measurement
    delay(5000);
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    
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
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 50, "Read Error!", &AsciiFont8x16);
        InkPageSprite.pushSprite();
        
    } else if (co2 == 0) {
        Serial.println("Invalid sample (CO2=0), skipping...");
        
    } else {
        // Valid measurement received
        Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%, CO2: %d ppm\n", 
                     temperature, humidity, co2);
        
        // Update display
        InkPageSprite.clear();
        
        // Title
        InkPageSprite.drawString(10, 10, "SCD40 Sensor", &AsciiFont8x16);
        
        // Temperature
        char tempStr[32];
        snprintf(tempStr, sizeof(tempStr), "Temp: %.1f C", temperature);
        InkPageSprite.drawString(10, 40, tempStr, &AsciiFont8x16);
        
        // Humidity
        char humStr[32];
        snprintf(humStr, sizeof(humStr), "Humi: %.1f %%", humidity);
        InkPageSprite.drawString(10, 70, humStr, &AsciiFont8x16);
        
        // CO2
        char co2Str[32];
        snprintf(co2Str, sizeof(co2Str), "CO2: %d ppm", co2);
        InkPageSprite.drawString(10, 100, co2Str, &AsciiFont8x16);
        
        // Timestamp
        char timeStr[32];
        snprintf(timeStr, sizeof(timeStr), "Time: %lds", millis()/1000);
        InkPageSprite.drawString(10, 150, timeStr, &AsciiFont8x16);
        
        InkPageSprite.pushSprite();
    }
    
    // Wait 1 second before next reading
    delay(1000);
}