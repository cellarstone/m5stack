/*
 * M5Unified Tutorial - 01_hello_world
 * For M5Stack Tab5
 * 
 * This demo introduces the basic initialization of M5Unified library
 * specifically for the M5Stack Tab5 and demonstrates:
 * - M5 configuration structure
 * - Basic display operations  
 * - Tab5 hardware capabilities
 * - Touch interface testing
 * 
 * M5Stack Tab5 specifications:
 * - ESP32-P4 processor
 * - 7" touch display (800x480)
 * - Speaker support
 * - Touch-only interface (no physical buttons)
 * 
 * Key concepts:
 * - M5.config() creates a configuration structure
 * - M5.begin() initializes the hardware with the config
 * - M5.Display provides unified display access
 * - M5.Touch for touch input handling
 */

#include <M5Unified.h>


void setup() {
    // Create configuration structure
    // This allows customization of initialization parameters
    auto cfg = M5.config();
    
    // Optional: Configure specific settings before initialization
    // cfg.output_power = true;  // Enable external power output
    // cfg.internal_imu = true;  // Enable IMU if available
    // cfg.internal_rtc = true;  // Enable RTC if available
    // cfg.internal_spk = true;  // Enable speaker if available
    // cfg.internal_mic = true;  // Enable microphone if available
    
    // Initialize M5 device with configuration
    M5.begin(cfg);
    
    // Set display rotation (3 = upside down from default)
    M5.Display.setRotation(3);
    
    // Clear screen with black background
    M5.Display.fillScreen(TFT_BLACK);
    
    // Set text color to white
    M5.Display.setTextColor(TFT_WHITE);
    
    // Display title
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);  // Top Center alignment
    M5.Display.drawString("M5Stack Tab5", M5.Display.width()/2, 10);
    M5.Display.drawString("Hello World!", M5.Display.width()/2, 50);
    
    // Display Tab5 information
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TL_DATUM);  // Top Left alignment
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Tab5 Information:", 10, 100);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Display Tab5 specific info
    M5.Display.drawString("Board: M5Stack Tab5", 10, 125);
    M5.Display.drawString("Processor: ESP32-P4", 10, 140);
    M5.Display.drawString("Display: 7\" IPS Touch (800x480)", 10, 155);
    
    
    // Display actual screen dimensions
    M5.Display.drawString("Resolution: " + String(M5.Display.width()) + " x " + String(M5.Display.height()) + " pixels", 10, 170);
    
    // Display hardware capabilities
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Hardware Capabilities:", 10, 195);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Touch capability
    bool hasTouch = M5.Touch.isEnabled();
    M5.Display.drawString("Touch Screen: " + String(hasTouch ? "Enabled" : "Disabled"), 10, 210);
    
    // Speaker capability
    bool hasSpeaker = M5.Speaker.isEnabled();
    M5.Display.drawString("Speaker: " + String(hasSpeaker ? "Enabled" : "Disabled"), 10, 225);
    
    // Note about buttons
    M5.Display.drawString("Physical Buttons: None (Touch only)", 10, 240);
    
    // Display M5Unified features
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("M5Unified Features:", 10, 265);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Show M5Unified capabilities
    M5.Display.drawString("- Hardware abstraction layer", 10, 280);
    M5.Display.drawString("- Built-in M5GFX graphics", 10, 295);
    M5.Display.drawString("- Touch event handling", 10, 310);
    M5.Display.drawString("- Audio output support", 10, 325);
    
    // Instructions
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);  // Bottom Center alignment
    M5.Display.drawString("Touch the screen to test touch input", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Play a startup sound if speaker is available
    if (hasSpeaker) {
        M5.Speaker.tone(1000, 100);
        delay(100);
        M5.Speaker.tone(1500, 100);
        delay(100);
        M5.Speaker.tone(2000, 100);
        delay(100);
        M5.Speaker.stop();
    }
}

void loop() {
    // Update M5 system (handles button and touch events)
    M5.update();
    
    static bool wasPressed = false;
    static unsigned long lastInteraction = 0;
    
    // Check for touch input
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            // Get touch coordinates
            int x = touch.x;
            int y = touch.y;
            
            // Draw circle at touch point
            M5.Display.fillCircle(x, y, 5, TFT_RED);
            
            // Display coordinates
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.setTextDatum(ML_DATUM);
            M5.Display.drawString("Touch: (" + String(x) + ", " + String(y) + ")", 10, M5.Display.height()/2);
            
            // Play touch sound
            if (M5.Speaker.isEnabled()) {
                M5.Speaker.tone(800, 50);
            }
            
            wasPressed = true;
            lastInteraction = millis();
        }
    }
    
    // Note: Tab5 has no physical buttons, only touch input
    // The button check code is removed as Tab5 is touch-only
    
    // Clear interaction feedback after 2 seconds
    if (lastInteraction > 0 && millis() - lastInteraction > 2000) {
        // Clear the interaction area
        M5.Display.fillRect(0, M5.Display.height()/2, M5.Display.width(), 60, TFT_BLACK);
        lastInteraction = 0;
    }
    
    delay(10);
}