#include <M5EPD.h>
#include <SensirionI2CScd4x.h>
#include <Preferences.h>

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
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    
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
        snprintf(humStr, sizeof(humStr), "%.1f", humidity);
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
        
        // Push the updated canvas to display
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
        // Set flag to show inverted display on next update
        invertDisplay = true;
    }
    
    // Wait 5 seconds before next reading (E-ink displays are slower to update)
    delay(5000);
}