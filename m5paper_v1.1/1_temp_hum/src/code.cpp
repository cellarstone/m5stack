#include <M5EPD.h>
#include <SensirionI2CScd4x.h>

// Use M5EPD constants for Port A pins
const int PortA_SDA = M5EPD_PORTA_Y_PIN;  // Yellow wire
const int PortA_SCL = M5EPD_PORTA_W_PIN;  // White wire

SensirionI2CScd4x scd4x;
M5EPD_Canvas canvas(&M5.EPD);

void setup() {
    // Initialize M5Paper with specific settings
    M5.begin(false,  // touchEnable
             false,  // SDEnable
             true,   // SerialEnable
             false,  // BatteryADCEnable
             false   // I2CEnable - we'll initialize it manually
    );

    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== SCD40 Temperature & Humidity Monitor ===");
    
    M5.EPD.SetRotation(90);
    M5.EPD.Clear(true);
    
    // Create canvas for display
    canvas.createCanvas(540, 960);
    
    // Show initialization message
    canvas.fillCanvas(0);
    canvas.setTextSize(3);
    canvas.drawString("Initializing SCD40...", 50, 400);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
    // Initialize I2C on Port A using Wire1
    Wire1.begin(PortA_SDA, PortA_SCL);
    Serial.printf("I2C initialized on Port A (SDA=%d, SCL=%d)\n", PortA_SDA, PortA_SCL);
    
    // Initialize SCD40 sensor
    uint16_t error;
    char errorMessage[256];
    
    scd4x.begin(Wire1);
    
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
        canvas.setTextSize(3);
        canvas.drawString("SCD40 Init Failed!", 50, 400);
        canvas.drawString("Check connections", 50, 500);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
        while(1) delay(1000); // Stop here if init failed
    }
    
    Serial.println("SCD40 initialized successfully!");
    Serial.println("Waiting for first measurement (5 seconds)...");
    
    canvas.fillCanvas(0);
    canvas.setTextSize(3);
    canvas.drawString("SCD40 Ready!", 50, 400);
    canvas.drawString("Waiting for data...", 50, 500);
    canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
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
        
        canvas.fillCanvas(0);
        canvas.setTextSize(3);
        canvas.drawString("Read Error!", 50, 400);
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
        
    } else if (co2 == 0) {
        Serial.println("Invalid sample (CO2=0), skipping...");
        
    } else {
        // Valid measurement received
        Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%, CO2: %d ppm\n", 
                     temperature, humidity, co2);
        
        // Update display
        canvas.fillCanvas(0);
        
        // Temperature
        canvas.setTextSize(3);
        canvas.drawString("Temperature", 50, 100);
        canvas.setTextSize(5);
        canvas.drawString(String(temperature, 1) + " C", 50, 170);
        
        // Humidity
        canvas.setTextSize(3);
        canvas.drawString("Humidity", 50, 350);
        canvas.setTextSize(5);
        canvas.drawString(String(humidity, 1) + " %", 50, 420);
        
        // CO2
        canvas.setTextSize(3);
        canvas.drawString("CO2", 50, 600);
        canvas.setTextSize(5);
        canvas.drawString(String(co2) + " ppm", 50, 670);
        
        // Timestamp (optional)
        canvas.setTextSize(2);
        canvas.drawString("Updated: " + String(millis()/1000) + "s", 50, 850);
        
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    }
    
    // Wait 1 second before next reading
    delay(1000);
}