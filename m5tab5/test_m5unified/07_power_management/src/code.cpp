/*
 * M5Unified Tutorial - 07_power_management
 * 
 * This demo demonstrates power management capabilities of M5Stack devices:
 * - Battery level monitoring and visualization
 * - Power consumption measurement
 * - Sleep mode functionality (light sleep, deep sleep)
 * - Wake-up sources configuration
 * - CPU frequency scaling for power optimization
 * - Display brightness control
 * - Power-saving strategies
 * 
 * Key concepts:
 * - M5.Power.getBatteryLevel() for battery monitoring
 * - M5.Power.deepSleep() for power saving
 * - ESP sleep modes and wake-up timers
 * - Power optimization techniques
 * - Real-time power consumption display
 */

#include <M5Unified.h>
#include <esp_sleep.h>
#include <esp_pm.h>

// Demo modes
enum PowerDemo {
    DEMO_BATTERY_MONITOR,
    DEMO_POWER_CONSUMPTION,
    DEMO_SLEEP_MODES,
    DEMO_CPU_SCALING,
    DEMO_POWER_OPTIMIZATION,
    POWER_DEMO_COUNT
};

// Touch button definitions
struct TouchButton {
    int x, y, w, h;
    String label;
    uint16_t color;
    bool pressed;
};

// Define touch buttons
TouchButton btnPrev = {10, 275, 90, 35, "< PREV", TFT_DARKGREEN, false};
TouchButton btnAction = {115, 275, 90, 35, "ACTION", TFT_DARKGREY, false};
TouchButton btnNext = {220, 275, 90, 35, "NEXT >", TFT_DARKGREEN, false};

PowerDemo currentDemo = DEMO_BATTERY_MONITOR;
const char* powerDemoNames[] = {
    "Battery Monitor",
    "Power Consumption",
    "Sleep Modes",
    "CPU Scaling",
    "Power Optimization"
};

// Power monitoring variables
float batteryVoltage = 0;
float batteryLevel = 0;
bool isCharging = false;
float powerConsumption = 0;
uint32_t uptime = 0;
uint32_t lastPowerUpdate = 0;

// Sleep mode variables
uint32_t sleepDuration = 5000;  // 5 seconds default
bool sleepPending = false;
RTC_DATA_ATTR int bootCount = 0;  // Survives deep sleep

// CPU scaling variables
uint32_t currentCpuFreq = 240;  // MHz
uint32_t cpuFreqOptions[] = {80, 160, 240};
int cpuFreqIndex = 2;  // Start at 240MHz

// Display brightness
int displayBrightness = 255;

// Power optimization settings
bool wifiEnabled = true;
bool bluetoothEnabled = true;
bool displayEnabled = true;
int displayTimeout = 30000;  // 30 seconds
uint32_t lastActivity = 0;

// Forward declarations
void checkWakeupReason();
void displayWelcome();
void displayCurrentDemo();
void drawBatteryMonitorBackground();
void drawPowerConsumptionBackground();
void drawSleepModesBackground();
void drawCpuScalingBackground();
void drawPowerOptimizationBackground();
void updatePowerData();
void handleBatteryMonitorDemo();
void handlePowerConsumptionDemo();
void handleSleepModesDemo();
void handleCpuScalingDemo();
void handlePowerOptimizationDemo();
void drawTouchButton(TouchButton& btn);
bool checkTouchButton(TouchButton& btn, int touchX, int touchY);
void drawAllButtons();
void handleActionButton();

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize power management
    if (!M5.Power.begin()) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Power Management", M5.Display.width()/2, M5.Display.height()/2 - 20);
        M5.Display.drawString("Not Available", M5.Display.width()/2, M5.Display.height()/2 + 20);
        delay(3000);
    }
    
    // Check wake-up reason
    checkWakeupReason();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
    
    lastActivity = millis();
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5 Power", M5.Display.width()/2, 30);
    M5.Display.drawString("Management", M5.Display.width()/2, 70);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Battery & Sleep Demo", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Features:", M5.Display.width()/2, 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Battery Level Monitoring", M5.Display.width()/2, 180);
    M5.Display.drawString("• Power Consumption Tracking", M5.Display.width()/2, 195);
    M5.Display.drawString("• Sleep Mode Management", M5.Display.width()/2, 210);
    M5.Display.drawString("• CPU Frequency Scaling", M5.Display.width()/2, 225);
    M5.Display.drawString("• Power Optimization", M5.Display.width()/2, 240);
    
    if (bootCount > 0) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Boot #" + String(bootCount) + " (Woke from deep sleep)", M5.Display.width()/2, 260);
    }
}

void checkWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
            bootCount++;
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("Wakeup caused by ULP program");
            break;
        default:
            Serial.println("Wakeup was not caused by deep sleep");
            bootCount = 1;
            break;
    }
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Power Management", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString(powerDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(POWER_DEMO_COUNT), M5.Display.width()/2, 60);
    
    // Draw demo-specific background
    switch(currentDemo) {
        case DEMO_BATTERY_MONITOR:
            drawBatteryMonitorBackground();
            break;
        case DEMO_POWER_CONSUMPTION:
            drawPowerConsumptionBackground();
            break;
        case DEMO_SLEEP_MODES:
            drawSleepModesBackground();
            break;
        case DEMO_CPU_SCALING:
            drawCpuScalingBackground();
            break;
        case DEMO_POWER_OPTIMIZATION:
            drawPowerOptimizationBackground();
            break;
    }
    
    // Draw touch buttons
    drawAllButtons();
}

void drawBatteryMonitorBackground() {
    // Draw battery outline
    int battX = 100, battY = 90, battW = 120, battH = 60;
    M5.Display.drawRect(battX, battY, battW, battH, TFT_WHITE);
    M5.Display.fillRect(battX + battW, battY + 15, 10, 30, TFT_WHITE);  // Battery terminal
    
    // Draw voltage and level labels
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Battery Status:", 20, 170);
    M5.Display.drawString("Voltage:", 20, 190);
    M5.Display.drawString("Level:", 20, 210);
    M5.Display.drawString("Charging:", 20, 230);
    M5.Display.drawString("Uptime:", 200, 190);
}

void drawPowerConsumptionBackground() {
    // Draw power consumption graph area
    M5.Display.drawRect(20, 80, 280, 100, TFT_DARKGREY);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Power Consumption Over Time", 25, 70);
    M5.Display.drawString("Current:", 25, 190);
    M5.Display.drawString("Average:", 25, 210);
    M5.Display.drawString("Peak:", 200, 190);
    M5.Display.drawString("Estimated Runtime:", 200, 210);
}

void drawSleepModesBackground() {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Sleep Mode Options:", 20, 80);
    
    M5.Display.drawString("• ACTION button: Deep Sleep", 20, 100);
    M5.Display.drawString("• Auto-sleep timeout: 30s", 20, 115);
    M5.Display.drawString("• Wake on timer expiry", 20, 130);
    
    M5.Display.drawString("Sleep Duration:", 20, 160);
    M5.Display.drawString("Boot Count: " + String(bootCount), 20, 180);
    M5.Display.drawString("Last Activity:", 20, 200);
    
    // Draw sleep duration bar
    M5.Display.drawRect(20, 220, 280, 20, TFT_DARKGREY);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Touch slider to adjust sleep duration", 25, 245);
}

void drawCpuScalingBackground() {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("CPU Frequency Scaling:", 20, 80);
    
    M5.Display.drawString("Current Frequency:", 20, 100);
    M5.Display.drawString("Power Level:", 20, 120);
    M5.Display.drawString("Performance:", 20, 140);
    
    M5.Display.drawString("ACTION button cycles frequency:", 20, 170);
    M5.Display.drawString("• 80MHz  (Low Power)", 20, 190);
    M5.Display.drawString("• 160MHz (Balanced)", 20, 205);
    M5.Display.drawString("• 240MHz (High Performance)", 20, 220);
}

void drawPowerOptimizationBackground() {
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Power Optimization Settings:", 20, 80);
    
    M5.Display.drawString("WiFi:", 20, 100);
    M5.Display.drawString("Bluetooth:", 20, 120);
    M5.Display.drawString("Display Brightness:", 20, 140);
    M5.Display.drawString("Display Timeout:", 20, 160);
    
    M5.Display.drawString("Estimated Battery Life:", 20, 190);
    M5.Display.drawString("Current Settings:", 20, 210);
}

void updatePowerData() {
    // Update power data every second
    if (millis() - lastPowerUpdate > 1000) {
        // Get battery information
        // For ESP32P4/M5TAB5, check if power functions are available
        int batteryMilliVolts = M5.Power.getBatteryVoltage();
        if (batteryMilliVolts != -1) {
            batteryVoltage = batteryMilliVolts / 1000.0;  // Convert mV to V
            batteryLevel = M5.Power.getBatteryLevel();
            isCharging = M5.Power.isCharging();
        } else {
            // Simulate battery data for devices without power IC
            batteryVoltage = 3.7 + (random(-50, 50) / 100.0);
            batteryLevel = constrain(batteryVoltage * 100 - 320, 0, 100);
            isCharging = false;
        }
        
        // Calculate uptime
        uptime = millis() / 1000;
        
        // Estimate power consumption (simplified)
        powerConsumption = 150 + random(-20, 20);  // mA
        if (currentCpuFreq == 240) powerConsumption += 50;
        else if (currentCpuFreq == 80) powerConsumption -= 30;
        
        lastPowerUpdate = millis();
    }
}

void handleBatteryMonitorDemo() {
    // Draw battery level bar
    int battX = 100, battY = 90, battW = 120, battH = 60;
    M5.Display.fillRect(battX + 2, battY + 2, battW - 4, battH - 4, TFT_BLACK);
    
    // Calculate battery fill
    int fillWidth = (batteryLevel / 100.0) * (battW - 4);
    uint16_t battColor = TFT_GREEN;
    if (batteryLevel < 30) battColor = TFT_RED;
    else if (batteryLevel < 60) battColor = TFT_YELLOW;
    
    M5.Display.fillRect(battX + 2, battY + 2, fillWidth, battH - 4, battColor);
    
    // Draw battery percentage
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(String(int(batteryLevel)) + "%", battX + battW/2, battY + battH/2);
    
    // Display detailed info
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString(String(batteryVoltage, 2) + "V", 90, 190);
    M5.Display.drawString(String(int(batteryLevel)) + "%", 70, 210);
    M5.Display.drawString(isCharging ? "Yes" : "No", 90, 230);
    
    // Display uptime
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    M5.Display.drawString(String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s", 250, 190);
    
    // Charging indicator
    if (isCharging) {
        M5.Display.fillTriangle(260, 100, 280, 120, 260, 140, TFT_GREEN);
        M5.Display.fillRect(255, 115, 10, 10, TFT_GREEN);
    }
}

void handlePowerConsumptionDemo() {
    // Simplified power consumption visualization
    static float powerHistory[50] = {0};
    static int historyIndex = 0;
    
    // Update power history
    powerHistory[historyIndex] = powerConsumption;
    historyIndex = (historyIndex + 1) % 50;
    
    // Clear graph area
    M5.Display.fillRect(21, 81, 278, 98, TFT_BLACK);
    
    // Draw power consumption graph
    for (int i = 0; i < 49; i++) {
        int idx1 = (historyIndex + i) % 50;
        int idx2 = (historyIndex + i + 1) % 50;
        
        int y1 = 180 - (powerHistory[idx1] / 300.0) * 98;
        int y2 = 180 - (powerHistory[idx2] / 300.0) * 98;
        int x1 = 22 + i * 5;
        int x2 = 22 + (i + 1) * 5;
        
        M5.Display.drawLine(x1, y1, x2, y2, TFT_CYAN);
        M5.Display.drawPixel(x1, y1, TFT_YELLOW);
    }
    
    // Display current values
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString(String(int(powerConsumption)) + " mA", 80, 190);
    
    // Calculate average
    float avgPower = 0;
    for (int i = 0; i < 50; i++) {
        avgPower += powerHistory[i];
    }
    avgPower /= 50;
    M5.Display.drawString(String(int(avgPower)) + " mA", 80, 210);
    
    // Estimated runtime
    float runtime = (batteryLevel / 100.0) * 3000 / powerConsumption;  // Hours (assuming 3000mAh battery)
    M5.Display.drawString(String(runtime, 1) + " hours", 290, 210);
}

void handleSleepModesDemo() {
    // Handle sleep duration adjustment via touch on the slider bar
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed() && touch.y >= 220 && touch.y <= 240 && touch.x >= 20 && touch.x <= 300) {
            sleepDuration = constrain((touch.x - 20) / 280.0 * 30000, 1000, 30000);  // 1-30 seconds
            
            // Update sleep duration bar
            M5.Display.fillRect(21, 221, 278, 18, TFT_BLACK);
            int fillWidth = (sleepDuration / 30000.0) * 278;
            M5.Display.fillRect(21, 221, fillWidth, 18, TFT_BLUE);
        }
    }
    
    // Display current settings
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString(String(sleepDuration / 1000) + " seconds", 140, 160);
    
    uint32_t inactiveTime = millis() - lastActivity;
    M5.Display.drawString(String(inactiveTime / 1000) + "s ago", 120, 200);
}

void handleCpuScalingDemo() {
    // Display current frequency
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString(String(currentCpuFreq) + " MHz", 150, 100);
    
    // Display power level
    String powerLevel = "High";
    uint16_t powerColor = TFT_RED;
    if (currentCpuFreq == 160) {
        powerLevel = "Medium";
        powerColor = TFT_YELLOW;
    } else if (currentCpuFreq == 80) {
        powerLevel = "Low";
        powerColor = TFT_GREEN;
    }
    M5.Display.setTextColor(powerColor, TFT_BLACK);
    M5.Display.drawString(powerLevel, 100, 120);
    
    // Display performance indicator
    int performance = map(currentCpuFreq, 80, 240, 25, 100);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(String(performance) + "%", 110, 140);
    
    // Draw frequency bars
    M5.Display.fillRect(160, 100, 100, 80, TFT_BLACK);
    for (int i = 0; i < 3; i++) {
        uint16_t barColor = (i == cpuFreqIndex) ? TFT_CYAN : TFT_DARKGREY;
        int barHeight = (cpuFreqOptions[i] / 240.0) * 70;
        M5.Display.fillRect(170 + i * 25, 180 - barHeight, 20, barHeight, barColor);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(String(cpuFreqOptions[i]), 180 + i * 25, 190);
    }
}

void handlePowerOptimizationDemo() {
    // Handle touch for toggles
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            if (touch.y >= 95 && touch.y <= 110) {
                wifiEnabled = !wifiEnabled;
            } else if (touch.y >= 115 && touch.y <= 130) {
                bluetoothEnabled = !bluetoothEnabled;
            } else if (touch.y >= 135 && touch.y <= 150) {
                displayBrightness = (displayBrightness == 255) ? 128 : 255;
                // Note: Actual brightness control would need proper implementation
            }
        }
    }
    
    // Display toggles
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.setTextColor(wifiEnabled ? TFT_GREEN : TFT_RED, TFT_BLACK);
    M5.Display.drawString(wifiEnabled ? "ON" : "OFF", 70, 100);
    M5.Display.setTextColor(bluetoothEnabled ? TFT_GREEN : TFT_RED, TFT_BLACK);
    M5.Display.drawString(bluetoothEnabled ? "ON" : "OFF", 90, 120);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(String(displayBrightness * 100 / 255) + "%", 150, 140);
    M5.Display.drawString(String(displayTimeout / 1000) + "s", 130, 160);
    
    // Estimate battery life
    float estimatedHours = 24;  // Base estimate
    if (wifiEnabled) estimatedHours *= 0.7;
    if (bluetoothEnabled) estimatedHours *= 0.9;
    if (displayBrightness > 128) estimatedHours *= 0.8;
    if (currentCpuFreq == 240) estimatedHours *= 0.8;
    else if (currentCpuFreq == 80) estimatedHours *= 1.3;
    
    M5.Display.drawString(String(estimatedHours, 1) + " hours", 160, 190);
    
    String settings = "";
    settings += wifiEnabled ? "WiFi " : "";
    settings += bluetoothEnabled ? "BT " : "";
    settings += String(currentCpuFreq) + "MHz";
    M5.Display.drawString(settings, 120, 210);
}

// Touch button helper functions
void drawTouchButton(TouchButton& btn) {
    uint16_t fillColor = btn.pressed ? TFT_WHITE : btn.color;
    uint16_t textColor = btn.pressed ? TFT_BLACK : TFT_WHITE;
    
    M5.Display.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 5, fillColor);
    M5.Display.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 5, TFT_WHITE);
    
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextColor(textColor);
    M5.Display.setTextSize(1);
    M5.Display.drawString(btn.label, btn.x + btn.w/2, btn.y + btn.h/2);
}

bool checkTouchButton(TouchButton& btn, int touchX, int touchY) {
    return (touchX >= btn.x && touchX <= btn.x + btn.w &&
            touchY >= btn.y && touchY <= btn.y + btn.h);
}

void drawAllButtons() {
    drawTouchButton(btnPrev);
    drawTouchButton(btnAction);
    drawTouchButton(btnNext);
}

void loop() {
    M5.update();
    
    // Update power data
    updatePowerData();
    
    // Handle touch input for navigation buttons
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        
        // Update last activity on any touch
        if (touch.wasPressed()) {
            lastActivity = millis();
        }
        
        // Check button presses
        if (touch.wasPressed()) {
            bool needRedraw = false;
            
            // Check Previous button
            if (checkTouchButton(btnPrev, touch.x, touch.y)) {
                btnPrev.pressed = true;
                drawTouchButton(btnPrev);
                currentDemo = (PowerDemo)((currentDemo - 1 + POWER_DEMO_COUNT) % POWER_DEMO_COUNT);
                delay(100);
                displayCurrentDemo();
                needRedraw = false;
            }
            // Check Next button
            else if (checkTouchButton(btnNext, touch.x, touch.y)) {
                btnNext.pressed = true;
                drawTouchButton(btnNext);
                currentDemo = (PowerDemo)((currentDemo + 1) % POWER_DEMO_COUNT);
                delay(100);
                displayCurrentDemo();
                needRedraw = false;
            }
            // Check Action button
            else if (checkTouchButton(btnAction, touch.x, touch.y)) {
                btnAction.pressed = true;
                drawTouchButton(btnAction);
                delay(100);
                
                // Handle demo-specific actions
                handleActionButton();
                needRedraw = true;
            }
            
            // Reset button states
            if (needRedraw) {
                btnPrev.pressed = false;
                btnAction.pressed = false;
                btnNext.pressed = false;
                drawAllButtons();
            }
        }
    }
    
    // Handle current demo
    switch(currentDemo) {
        case DEMO_BATTERY_MONITOR:
            handleBatteryMonitorDemo();
            break;
        case DEMO_POWER_CONSUMPTION:
            handlePowerConsumptionDemo();
            break;
        case DEMO_SLEEP_MODES:
            handleSleepModesDemo();
            break;
        case DEMO_CPU_SCALING:
            handleCpuScalingDemo();
            break;
        case DEMO_POWER_OPTIMIZATION:
            handlePowerOptimizationDemo();
            break;
    }
    
    // Auto-sleep if inactive for too long (only in sleep demo)
    if (currentDemo == DEMO_SLEEP_MODES && millis() - lastActivity > displayTimeout) {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.setTextSize(1);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Auto-sleep due to inactivity", M5.Display.width()/2, M5.Display.height()/2);
        delay(1000);
        
        esp_sleep_enable_timer_wakeup(10000000);  // 10 seconds
        esp_light_sleep_start();
        
        displayCurrentDemo();
        lastActivity = millis();
    }
    
    delay(100);
}

void handleActionButton() {
    // Handle demo-specific action button presses
        switch(currentDemo) {
            case DEMO_SLEEP_MODES:
                // Deep sleep
                M5.Display.fillScreen(TFT_BLACK);
                M5.Display.setTextColor(TFT_RED);
                M5.Display.setTextSize(2);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("Deep Sleep", M5.Display.width()/2, M5.Display.height()/2 - 20);
                M5.Display.drawString("for " + String(sleepDuration / 1000) + "s", M5.Display.width()/2, M5.Display.height()/2 + 20);
                M5.Display.setTextSize(1);
                M5.Display.drawString("Will restart device...", M5.Display.width()/2, M5.Display.height()/2 + 40);
                delay(2000);
                
                esp_sleep_enable_timer_wakeup(sleepDuration * 1000);
                esp_deep_sleep_start();
                break;
                
            case DEMO_CPU_SCALING:
                // Same as touch for CPU scaling
                cpuFreqIndex = (cpuFreqIndex + 1) % 3;
                currentCpuFreq = cpuFreqOptions[cpuFreqIndex];
                setCpuFrequencyMhz(currentCpuFreq);
                break;
                
            default:
                // Generic action feedback
                M5.Display.fillRect(M5.Display.width()/2 - 50, M5.Display.height()/2 - 10, 100, 20, TFT_BLUE);
                M5.Display.setTextColor(TFT_WHITE);
                M5.Display.setTextSize(1);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("Action!", M5.Display.width()/2, M5.Display.height()/2);
                delay(300);
                displayCurrentDemo();
                break;
        }
}