#include <M5CoreInk.h>
#include <SensirionI2CScd4x.h>
#include <Preferences.h>

// M5CoreInk Port A (Grove) pins
const int PortA_SDA = 32;  // Yellow wire
const int PortA_SCL = 33;  // White wire

SensirionI2CScd4x scd4x;
Ink_Sprite InkPageSprite(&M5.M5Ink);
Preferences preferences;
bool invertDisplay = false;  // Flag for visual update indicator
bool calibrationMode = false;
unsigned long calibrationStartTime = 0;
const unsigned long CALIBRATION_DURATION = 15000;  // 15 seconds for testing (was 180000 for 3 minutes)

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
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 80, "Calibration", &AsciiFont8x16);
        InkPageSprite.drawString(10, 100, "FAILED!", &AsciiFont8x16);
        InkPageSprite.pushSprite();
    } else {
        Serial.printf("Calibration successful! Set to 420 ppm, correction: %d\n", frcCorrection);
        
        // Save calibration timestamp
        preferences.begin("scd40", false);
        preferences.putULong("lastCalib", millis());
        preferences.end();
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 80, "Calibration", &AsciiFont8x16);
        InkPageSprite.drawString(10, 100, "SUCCESS!", &AsciiFont8x16);
        InkPageSprite.drawString(10, 120, "420 ppm set", &AsciiFont8x16);
        InkPageSprite.pushSprite();
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
    
    // Check last calibration time
    preferences.begin("scd40", true);
    unsigned long lastCalib = preferences.getULong("lastCalib", 0);
    preferences.end();
    
    if (lastCalib == 0) {
        Serial.println("Note: Sensor has never been calibrated");
        Serial.println("Press top button for 3 seconds to start calibration");
    } else {
        Serial.printf("Last calibration: %lu ms ago\n", millis() - lastCalib);
    }
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    
    // Check button for calibration mode
    M5.update();
    if (M5.BtnUP.wasPressed() && !calibrationMode) {  // Only enter if not already in calibration
        Serial.println("Button UP pressed - entering calibration mode");
        calibrationMode = true;
        calibrationStartTime = millis();
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 50, "CALIBRATION", &AsciiFont8x16);
        InkPageSprite.drawString(10, 70, "Take outside!", &AsciiFont8x16);
        InkPageSprite.drawString(10, 90, "Wait 15 sec", &AsciiFont8x16);  // Updated for testing
        InkPageSprite.drawString(10, 110, "Press DOWN btn", &AsciiFont8x16);
        InkPageSprite.drawString(10, 130, "to calibrate", &AsciiFont8x16);
        InkPageSprite.pushSprite();
    }
    
    // Handle calibration mode
    if (calibrationMode) {
        unsigned long elapsed = millis() - calibrationStartTime;
        
        // Update countdown display
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 30, "CALIBRATION", &AsciiFont8x16);
        InkPageSprite.drawString(10, 50, "Take outside!", &AsciiFont8x16);
        InkPageSprite.drawString(10, 70, "Stabilizing...", &AsciiFont8x16);
        
        char timeStr[32];
        unsigned long remaining = (CALIBRATION_DURATION - elapsed) / 1000;
        snprintf(timeStr, sizeof(timeStr), "Wait: %lu sec", remaining);
        InkPageSprite.drawString(10, 90, timeStr, &AsciiFont8x16);
        
        if (elapsed >= CALIBRATION_DURATION) {
            InkPageSprite.drawString(10, 110, "READY!", &AsciiFont24x48);
            InkPageSprite.drawString(10, 160, "Press DOWN btn", &AsciiFont8x16);
            InkPageSprite.drawString(10, 180, "to calibrate", &AsciiFont8x16);
            
            // Check if DOWN button pressed to confirm calibration
            if (M5.BtnDOWN.wasPressed()) {
                Serial.println("Button DOWN pressed - starting calibration countdown");
                // Show 15 second countdown before calibration
                for (int i = 15; i > 0; i--) {
                    InkPageSprite.clear();
                    InkPageSprite.drawString(10, 50, "CALIBRATING", &AsciiFont8x16);
                    InkPageSprite.drawString(10, 70, "Keep outside!", &AsciiFont8x16);
                    char countStr[32];
                    snprintf(countStr, sizeof(countStr), "%d", i);
                    InkPageSprite.drawString(80, 100, countStr, &AsciiFont24x48);
                    InkPageSprite.drawString(10, 160, "seconds left", &AsciiFont8x16);
                    InkPageSprite.pushSprite();
                    delay(1000);
                }
                performCalibration();
            }
            
            // Allow canceling with UP button
            if (M5.BtnUP.wasPressed()) {
                Serial.println("Calibration cancelled");
                calibrationMode = false;
                InkPageSprite.clear();
                InkPageSprite.drawString(10, 80, "Calibration", &AsciiFont8x16);
                InkPageSprite.drawString(10, 100, "CANCELLED", &AsciiFont8x16);
                InkPageSprite.pushSprite();
                delay(2000);
            }
        }
        
        InkPageSprite.pushSprite();
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
        
        InkPageSprite.clear();
        InkPageSprite.drawString(10, 50, "Read Error!", &AsciiFont8x16);
        InkPageSprite.pushSprite();
        
    } else if (co2 == 0) {
        Serial.println("Invalid sample (CO2=0), skipping...");
        
    } else {
        // Valid measurement received
        Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%, CO2: %d ppm\n", 
                     temperature, humidity, co2);
        
        // Flash inverted display for visual update indicator
        if (invertDisplay) {
            // Show inverted (black background) briefly
            InkPageSprite.clear();
            
            // Draw black rectangles for visual feedback
            InkPageSprite.fillRect(0, 0, 100, 100, 0);     // Temperature area
            InkPageSprite.fillRect(100, 0, 100, 100, 0);   // Humidity area  
            InkPageSprite.fillRect(0, 100, 200, 100, 0);   // CO2 area
            
            InkPageSprite.pushSprite();
            delay(400);  // Show inverted briefly
            invertDisplay = false;
        }
        
        // Normal display (white background, black text)
        InkPageSprite.clear();
        
        // Draw dividing lines for visual separation
        InkPageSprite.drawLine(100, 0, 100, 100, 0);   // Vertical line
        InkPageSprite.drawLine(0, 100, 200, 100, 0);   // Horizontal line
        
        // Temperature - Top left quadrant
        InkPageSprite.drawString(35, 5, "TEMP", &AsciiFont8x16);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.0f", temperature);  // No decimal for larger display
        int tempX = 50 - (strlen(tempStr) * 24) / 2;  // Center in left half
        InkPageSprite.drawString(tempX, 35, tempStr, &AsciiFont24x48);
        InkPageSprite.drawString(70, 50, "C", &AsciiFont8x16);
        
        // Humidity - Top right quadrant  
        InkPageSprite.drawString(130, 5, "HUM", &AsciiFont8x16);
        char humStr[10];
        snprintf(humStr, sizeof(humStr), "%.0f", humidity);  // No decimal for larger display
        int humX = 150 - (strlen(humStr) * 24) / 2;  // Center in right half
        InkPageSprite.drawString(humX, 35, humStr, &AsciiFont24x48);
        InkPageSprite.drawString(170, 50, "%", &AsciiFont8x16);
        
        // CO2 - Bottom half (full width for prominence)
        InkPageSprite.drawString(85, 105, "CO2", &AsciiFont8x16);
        char co2Str[10];
        snprintf(co2Str, sizeof(co2Str), "%d", co2);
        int co2X = 100 - (strlen(co2Str) * 24) / 2;  // Center in full width
        InkPageSprite.drawString(co2X, 130, co2Str, &AsciiFont24x48);
        InkPageSprite.drawString(75, 175, "ppm", &AsciiFont8x16);
        
        InkPageSprite.pushSprite();
        
        // Set flag to show inverted display on next update
        invertDisplay = true;
    }
    
    // Wait 1 second before next reading
    delay(1000);
}