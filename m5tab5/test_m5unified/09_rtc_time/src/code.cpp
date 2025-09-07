/*
 * M5Unified Tutorial - 09_rtc_time
 * 
 * This demo demonstrates Real-Time Clock (RTC) functionality:
 * - RTC initialization and time setting
 * - Digital and analog clock displays
 * - Alarm and timer functionality
 * - Time zone handling
 * - Date/time formatting options
 * - Persistent time keeping
 * 
 * Key concepts:
 * - M5.Rtc.begin() for RTC initialization
 * - M5.Rtc.getTime() and M5.Rtc.setTime()
 * - rtc_time_t structure for time data
 * - Time formatting and display
 * - Alarm callbacks and timers
 */

#include <M5Unified.h>

// Demo modes
enum RTCDemo {
    DEMO_DIGITAL_CLOCK,
    DEMO_ANALOG_CLOCK,
    DEMO_STOPWATCH,
    DEMO_ALARM_TIMER,
    DEMO_TIME_SETTINGS,
    RTC_DEMO_COUNT
};

RTCDemo currentDemo = DEMO_DIGITAL_CLOCK;
const char* rtcDemoNames[] = {
    "Digital Clock",
    "Analog Clock",
    "Stopwatch",
    "Alarm & Timer",
    "Time Settings"
};

// RTC variables
rtc_time_t currentTime;
rtc_date_t currentDate;
bool rtcAvailable = false;

// Stopwatch variables
unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;
bool stopwatchRunning = false;
bool stopwatchReset = true;

// Alarm variables
rtc_time_t alarmTime = {0, 0, 12}; // 12:00:00
bool alarmEnabled = false;
bool alarmTriggered = false;
unsigned long alarmTriggerTime = 0;

// Timer variables
int timerMinutes = 5;
int timerSeconds = 0;
bool timerRunning = false;
unsigned long timerStart = 0;
int timerOriginalMinutes = 5;

// Time setting variables
int settingHour = 12;
int settingMinute = 0;
int settingSecond = 0;
int settingDay = 1;
int settingMonth = 1;
int settingYear = 2024;
int settingField = 0; // 0=hour, 1=minute, 2=second, 3=day, 4=month, 5=year

// Forward declarations
void displayWelcome();
void displayCurrentDemo();

void setup() {
    auto cfg = M5.config();
    cfg.internal_rtc = true;
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize RTC
    rtcAvailable = M5.Rtc.begin();
    
    if (!rtcAvailable) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("RTC Not Available", M5.Display.width()/2, M5.Display.height()/2);
        M5.Display.setTextSize(1);
        M5.Display.drawString("Using system time instead", M5.Display.width()/2, M5.Display.height()/2 + 30);
        delay(2000);
    }
    
    // Set initial time if RTC is available but not set
    if (rtcAvailable) {
        M5.Rtc.getTime(&currentTime);
        if (currentTime.hours == 0 && currentTime.minutes == 0 && currentTime.seconds == 0) {
            // Set default time: 12:00:00
            rtc_time_t defaultTime = {0, 0, 12};
            rtc_date_t defaultDate = {0, 1, 1, 2024}; // Monday, Jan 1, 2024
            M5.Rtc.setTime(&defaultTime);
            M5.Rtc.setDate(&defaultDate);
        }
    }
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5 RTC Demo", M5.Display.width()/2, 40);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Real-Time Clock", M5.Display.width()/2, 90);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Features:", M5.Display.width()/2, 130);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Digital & Analog Clocks", M5.Display.width()/2, 150);
    M5.Display.drawString("• Stopwatch Functionality", M5.Display.width()/2, 165);
    M5.Display.drawString("• Alarms & Timers", M5.Display.width()/2, 180);
    M5.Display.drawString("• Time & Date Setting", M5.Display.width()/2, 195);
    
    M5.Display.setTextColor(rtcAvailable ? TFT_GREEN : TFT_ORANGE);
    M5.Display.drawString(rtcAvailable ? "Hardware RTC Ready" : "Using System Time", M5.Display.width()/2, 220);
}

void getCurrentTime() {
    if (rtcAvailable) {
        M5.Rtc.getTime(&currentTime);
        M5.Rtc.getDate(&currentDate);
    } else {
        // Use system time as fallback
        unsigned long totalSeconds = millis() / 1000;
        currentTime.seconds = totalSeconds % 60;
        currentTime.minutes = (totalSeconds / 60) % 60;
        currentTime.hours = (totalSeconds / 3600) % 24;
        
        // Simple date calculation (not accurate, just for demo)
        currentDate.date = 1 + (totalSeconds / 86400) % 30;
        currentDate.month = 1 + (totalSeconds / 2592000) % 12;
        currentDate.year = 2024;
        currentDate.weekDay = (totalSeconds / 86400) % 7;
    }
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("RTC Demo", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString(rtcDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(RTC_DEMO_COUNT), M5.Display.width()/2, 60);
    
    // RTC status indicator
    M5.Display.setTextDatum(TR_DATUM);
    M5.Display.setTextColor(rtcAvailable ? TFT_GREEN : TFT_ORANGE);
    M5.Display.drawString(rtcAvailable ? "RTC" : "SYS", M5.Display.width() - 5, 10);
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    
    switch(currentDemo) {
        case DEMO_STOPWATCH:
            M5.Display.drawString("[A] Prev [B] Start/Stop/Reset [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
            break;
        case DEMO_TIME_SETTINGS:
            M5.Display.drawString("[A] Prev [B] Field/Set [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
            break;
        default:
            M5.Display.drawString("[A] Prev [B] Action [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
            break;
    }
}

void drawDigitalClock() {
    getCurrentTime();
    
    // Main time display
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
    M5.Display.drawString(timeStr, M5.Display.width()/2, 120);
    
    // Date display
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    
    const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    char dateStr[30];
    sprintf(dateStr, "%s, %s %d, %d", 
            weekdays[currentDate.weekDay], 
            months[currentDate.month - 1], 
            currentDate.date, 
            currentDate.year);
    
    M5.Display.drawString(dateStr, M5.Display.width()/2, 170);
    
    // 12/24 hour format toggle hint
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Touch to toggle 12/24 hour format", M5.Display.width()/2, 200);
    
    // Show in 12-hour format if touched
    static bool show12Hour = false;
    if (M5.Touch.wasPressed()) {
        show12Hour = !show12Hour;
    }
    
    if (show12Hour) {
        int hour12 = currentTime.hours;
        const char* ampm = "AM";
        if (hour12 == 0) hour12 = 12;
        else if (hour12 > 12) {
            hour12 -= 12;
            ampm = "PM";
        } else if (hour12 == 12) {
            ampm = "PM";
        }
        
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(TFT_YELLOW);
        char time12Str[15];
        sprintf(time12Str, "%d:%02d:%02d %s", hour12, currentTime.minutes, currentTime.seconds, ampm);
        M5.Display.drawString(time12Str, M5.Display.width()/2, 220);
    }
}

void drawAnalogClock() {
    getCurrentTime();
    
    int centerX = M5.Display.width() / 2;
    int centerY = 140;
    int radius = 70;
    
    // Clear clock area
    M5.Display.fillCircle(centerX, centerY, radius + 5, TFT_BLACK);
    
    // Draw clock face
    M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
    M5.Display.drawCircle(centerX, centerY, radius - 1, TFT_WHITE);
    
    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        float angle = (i * 30 - 90) * PI / 180.0;
        int x1 = centerX + (radius - 10) * cos(angle);
        int y1 = centerY + (radius - 10) * sin(angle);
        int x2 = centerX + (radius - 3) * cos(angle);
        int y2 = centerY + (radius - 3) * sin(angle);
        M5.Display.drawLine(x1, y1, x2, y2, TFT_WHITE);
    }
    
    // Draw minute markers
    for (int i = 0; i < 60; i++) {
        if (i % 5 != 0) {  // Skip hour markers
            float angle = (i * 6 - 90) * PI / 180.0;
            int x1 = centerX + (radius - 5) * cos(angle);
            int y1 = centerY + (radius - 5) * sin(angle);
            int x2 = centerX + (radius - 2) * cos(angle);
            int y2 = centerY + (radius - 2) * sin(angle);
            M5.Display.drawPixel(x1, y1, TFT_DARKGREY);
        }
    }
    
    // Draw hour hand
    float hourAngle = ((currentTime.hours % 12) * 30 + currentTime.minutes * 0.5 - 90) * PI / 180.0;
    int hourX = centerX + (radius - 25) * cos(hourAngle);
    int hourY = centerY + (radius - 25) * sin(hourAngle);
    M5.Display.drawLine(centerX, centerY, hourX, hourY, TFT_RED);
    M5.Display.drawLine(centerX + 1, centerY, hourX + 1, hourY, TFT_RED);
    
    // Draw minute hand
    float minuteAngle = (currentTime.minutes * 6 - 90) * PI / 180.0;
    int minuteX = centerX + (radius - 15) * cos(minuteAngle);
    int minuteY = centerY + (radius - 15) * sin(minuteAngle);
    M5.Display.drawLine(centerX, centerY, minuteX, minuteY, TFT_GREEN);
    
    // Draw second hand
    float secondAngle = (currentTime.seconds * 6 - 90) * PI / 180.0;
    int secondX = centerX + (radius - 10) * cos(secondAngle);
    int secondY = centerY + (radius - 10) * sin(secondAngle);
    M5.Display.drawLine(centerX, centerY, secondX, secondY, TFT_YELLOW);
    
    // Draw center dot
    M5.Display.fillCircle(centerX, centerY, 3, TFT_WHITE);
    
    // Digital time display
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextDatum(TC_DATUM);
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", currentTime.hours, currentTime.minutes, currentTime.seconds);
    M5.Display.drawString(timeStr, centerX, 230);
}

void drawStopwatch() {
    // Calculate current elapsed time
    unsigned long currentElapsed = stopwatchElapsed;
    if (stopwatchRunning) {
        currentElapsed += millis() - stopwatchStart;
    }
    
    // Convert to minutes, seconds, centiseconds
    unsigned long totalCentiseconds = currentElapsed / 10;
    int centiseconds = totalCentiseconds % 100;
    int seconds = (totalCentiseconds / 100) % 60;
    int minutes = (totalCentiseconds / 6000) % 60;
    int hours = totalCentiseconds / 360000;
    
    // Main stopwatch display
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(stopwatchRunning ? TFT_GREEN : TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    
    char stopwatchStr[15];
    if (hours > 0) {
        sprintf(stopwatchStr, "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        sprintf(stopwatchStr, "%02d:%02d.%02d", minutes, seconds, centiseconds);
    }
    M5.Display.drawString(stopwatchStr, M5.Display.width()/2, 130);
    
    // Status
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    String status = stopwatchRunning ? "RUNNING" : (stopwatchReset ? "READY" : "STOPPED");
    M5.Display.drawString(status, M5.Display.width()/2, 170);
    
    // Instructions
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Button B: " + String(stopwatchReset ? "START" : (stopwatchRunning ? "STOP" : "RESET")), 
                         M5.Display.width()/2, 200);
    
    // Lap time placeholder (could be extended)
    if (!stopwatchReset && !stopwatchRunning) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("Touch for lap time", M5.Display.width()/2, 220);
    }
}

void drawAlarmTimer() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Alarm section
    M5.Display.drawString("ALARM:", 20, 80);
    M5.Display.setTextColor(alarmEnabled ? TFT_GREEN : TFT_RED);
    M5.Display.drawString(alarmEnabled ? "ON" : "OFF", 80, 80);
    
    M5.Display.setTextColor(TFT_WHITE);
    char alarmStr[10];
    sprintf(alarmStr, "%02d:%02d:%02d", alarmTime.hours, alarmTime.minutes, alarmTime.seconds);
    M5.Display.drawString("Time: " + String(alarmStr), 20, 100);
    
    // Timer section
    M5.Display.drawString("TIMER:", 20, 130);
    M5.Display.setTextColor(timerRunning ? TFT_GREEN : TFT_RED);
    M5.Display.drawString(timerRunning ? "RUNNING" : "STOPPED", 80, 130);
    
    // Calculate remaining time
    int remainingMinutes = timerMinutes;
    int remainingSeconds = timerSeconds;
    
    if (timerRunning) {
        unsigned long elapsed = (millis() - timerStart) / 1000;
        int totalSeconds = timerOriginalMinutes * 60;
        int remaining = totalSeconds - elapsed;
        
        if (remaining <= 0) {
            remaining = 0;
            remainingMinutes = 0;
            remainingSeconds = 0;
            if (timerRunning) {
                timerRunning = false;
                // Timer finished - could trigger alarm
            }
        } else {
            remainingMinutes = remaining / 60;
            remainingSeconds = remaining % 60;
        }
    }
    
    M5.Display.setTextColor(TFT_WHITE);
    char timerStr[10];
    sprintf(timerStr, "%02d:%02d", remainingMinutes, remainingSeconds);
    M5.Display.drawString("Time: " + String(timerStr), 20, 150);
    
    // Instructions
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Touch top: Toggle alarm", 20, 180);
    M5.Display.drawString("Touch middle: Start/stop timer", 20, 195);
    M5.Display.drawString("Touch bottom: Set timer", 20, 210);
    
    // Alarm triggered indicator
    if (alarmTriggered && millis() - alarmTriggerTime < 5000) {
        M5.Display.fillRect(200, 80, 100, 30, TFT_RED);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("ALARM!", 250, 95);
    }
}

void drawTimeSettings() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Time & Date Settings:", 20, 80);
    
    // Current system time
    getCurrentTime();
    M5.Display.setTextColor(TFT_CYAN);
    char currentStr[30];
    sprintf(currentStr, "Current: %02d:%02d:%02d %02d/%02d/%04d", 
            currentTime.hours, currentTime.minutes, currentTime.seconds,
            currentDate.month, currentDate.date, currentDate.year);
    M5.Display.drawString(currentStr, 20, 100);
    
    // Setting fields
    const char* fieldNames[] = {"Hour", "Minute", "Second", "Day", "Month", "Year"};
    int* fieldValues[] = {&settingHour, &settingMinute, &settingSecond, &settingDay, &settingMonth, &settingYear};
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("New Settings:", 20, 130);
    
    for (int i = 0; i < 6; i++) {
        uint16_t color = (i == settingField) ? TFT_YELLOW : TFT_WHITE;
        M5.Display.setTextColor(color);
        M5.Display.drawString(String(fieldNames[i]) + ": " + String(*fieldValues[i]), 20, 150 + i * 15);
    }
    
    // Instructions
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Button B: Next field / Apply settings", 20, 240);
    M5.Display.drawString("Touch +/- to adjust selected field", 20, 255);
    
    // Draw +/- buttons
    M5.Display.fillRect(200, 150, 30, 20, TFT_GREEN);
    M5.Display.fillRect(240, 150, 30, 20, TFT_RED);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("+", 215, 160);
    M5.Display.drawString("-", 255, 160);
}

void handleStopwatch() {
    if (M5.BtnB.wasPressed()) {
        if (stopwatchReset) {
            // Start stopwatch
            stopwatchStart = millis();
            stopwatchRunning = true;
            stopwatchReset = false;
        } else if (stopwatchRunning) {
            // Stop stopwatch
            stopwatchElapsed += millis() - stopwatchStart;
            stopwatchRunning = false;
        } else {
            // Reset stopwatch
            stopwatchElapsed = 0;
            stopwatchReset = true;
        }
    }
}

void handleAlarmTimer() {
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            int touchY = touch.y;
            
            if (touchY >= 75 && touchY <= 105) {
                // Toggle alarm
                alarmEnabled = !alarmEnabled;
            } else if (touchY >= 125 && touchY <= 155) {
                // Start/stop timer
                if (!timerRunning) {
                    timerStart = millis();
                    timerOriginalMinutes = timerMinutes;
                    timerRunning = true;
                } else {
                    timerRunning = false;
                }
            } else if (touchY >= 175 && touchY <= 195) {
                // Set timer (cycle through common times)
                if (timerMinutes == 5) timerMinutes = 10;
                else if (timerMinutes == 10) timerMinutes = 15;
                else if (timerMinutes == 15) timerMinutes = 30;
                else if (timerMinutes == 30) timerMinutes = 60;
                else timerMinutes = 5;
            }
        }
    }
    
    // Check alarm
    if (alarmEnabled) {
        getCurrentTime();
        if (currentTime.hours == alarmTime.hours && 
            currentTime.minutes == alarmTime.minutes && 
            currentTime.seconds == alarmTime.seconds) {
            alarmTriggered = true;
            alarmTriggerTime = millis();
        }
    }
}

void handleTimeSettings() {
    // Handle field selection
    if (M5.BtnB.wasPressed()) {
        settingField++;
        if (settingField >= 6) {
            // Apply settings
            if (rtcAvailable) {
                rtc_time_t newTime = {settingSecond, settingMinute, settingHour};
                rtc_date_t newDate = {0, settingDay, settingMonth, settingYear};
                M5.Rtc.setTime(&newTime);
                M5.Rtc.setDate(&newDate);
            }
            settingField = 0;
        }
    }
    
    // Handle value adjustment
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            int touchX = touch.x;
            int* currentValue = nullptr;
            int minVal = 0, maxVal = 59;
            
            switch(settingField) {
                case 0: // Hour
                    currentValue = &settingHour;
                    maxVal = 23;
                    break;
                case 1: // Minute
                    currentValue = &settingMinute;
                    break;
                case 2: // Second
                    currentValue = &settingSecond;
                    break;
                case 3: // Day
                    currentValue = &settingDay;
                    minVal = 1; maxVal = 31;
                    break;
                case 4: // Month
                    currentValue = &settingMonth;
                    minVal = 1; maxVal = 12;
                    break;
                case 5: // Year
                    currentValue = &settingYear;
                    minVal = 2020; maxVal = 2030;
                    break;
            }
            
            if (currentValue) {
                if (touchX >= 200 && touchX <= 230) {
                    // + button
                    (*currentValue)++;
                    if (*currentValue > maxVal) *currentValue = minVal;
                } else if (touchX >= 240 && touchX <= 270) {
                    // - button
                    (*currentValue)--;
                    if (*currentValue < minVal) *currentValue = maxVal;
                }
            }
        }
    }
}

void loop() {
    M5.update();
    
    // Handle demo navigation
    if (M5.BtnA.wasPressed()) {
        currentDemo = (RTCDemo)((currentDemo - 1 + RTC_DEMO_COUNT) % RTC_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (RTCDemo)((currentDemo + 1) % RTC_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Handle current demo
    switch(currentDemo) {
        case DEMO_DIGITAL_CLOCK:
            drawDigitalClock();
            break;
        case DEMO_ANALOG_CLOCK:
            drawAnalogClock();
            break;
        case DEMO_STOPWATCH:
            drawStopwatch();
            handleStopwatch();
            break;
        case DEMO_ALARM_TIMER:
            drawAlarmTimer();
            handleAlarmTimer();
            break;
        case DEMO_TIME_SETTINGS:
            drawTimeSettings();
            handleTimeSettings();
            break;
    }
    
    delay(100);
}