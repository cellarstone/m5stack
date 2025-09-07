/*
 * M5Unified Tutorial - 06_imu_sensors
 * 
 * This demo demonstrates IMU sensor capabilities of M5Stack devices:
 * - Accelerometer data reading and visualization
 * - Gyroscope data reading and angular velocity display
 * - Magnetometer/compass functionality
 * - Motion detection and gesture recognition
 * - Orientation calculation (pitch, roll, yaw)
 * - Data logging and calibration
 * 
 * Key concepts:
 * - M5.Imu.begin() for IMU initialization
 * - M5.Imu.getAccel() for acceleration data
 * - M5.Imu.getGyro() for gyroscope data
 * - M5.Imu.getAhrsData() for orientation
 * - Real-time sensor visualization
 * - Motion-based interactions
 */

#include <M5Unified.h>
#include <math.h>

// Demo modes
enum IMUDemo {
    DEMO_ACCELEROMETER,
    DEMO_GYROSCOPE,
    DEMO_MAGNETOMETER,
    DEMO_ORIENTATION,
    DEMO_MOTION_DETECT,
    IMU_DEMO_COUNT
};

IMUDemo currentDemo = DEMO_ACCELEROMETER;
const char* imuDemoNames[] = {
    "Accelerometer",
    "Gyroscope",
    "Magnetometer",
    "Orientation",
    "Motion Detection"
};

// IMU data structures
float accX = 0, accY = 0, accZ = 0;
float gyroX = 0, gyroY = 0, gyroZ = 0;
float magX = 0, magY = 0, magZ = 0;
float pitch = 0, roll = 0, yaw = 0;
float temp = 0;

// Motion detection
float accMagnitude = 0;
float prevAccMagnitude = 0;
bool motionDetected = false;
float motionThreshold = 0.5;  // g-force threshold
unsigned long lastMotionTime = 0;

// Data history for graphs
#define HISTORY_SIZE 100
float accHistory[HISTORY_SIZE];
float gyroHistory[HISTORY_SIZE];
int historyIndex = 0;

// Colors for different axes
uint16_t axisColors[3] = {TFT_RED, TFT_GREEN, TFT_BLUE};
const char* axisNames[3] = {"X", "Y", "Z"};

// Calibration data
bool isCalibrating = false;
int calibrationCount = 0;
float accOffsetX = 0, accOffsetY = 0, accOffsetZ = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

// Virtual button structure for navigation
struct NavButton {
    int x, y, w, h;
    const char* label;
    uint16_t color;
    bool isPressed;
};

// Navigation buttons
NavButton navButtons[3];

// Forward declarations
void displayWelcome();
void calibrateIMU();
void displayCurrentDemo();
void drawAccelerometerBackground();
void drawGyroscopeBackground();
void drawMagnetometerBackground();
void drawOrientationBackground();
void drawMotionDetectBackground();
void initNavButtons();
void drawNavButtons();
bool checkNavButtons();

void setup() {
    auto cfg = M5.config();
    cfg.internal_imu = true;  // Enable IMU
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize IMU
    if (!M5.Imu.begin()) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("IMU Sensor", M5.Display.width()/2, M5.Display.height()/2 - 20);
        M5.Display.drawString("Not Available", M5.Display.width()/2, M5.Display.height()/2 + 20);
        while(true) {
            M5.update();
            delay(100);
        }
    }
    
    // Initialize history arrays
    for (int i = 0; i < HISTORY_SIZE; i++) {
        accHistory[i] = 0;
        gyroHistory[i] = 0;
    }
    
    // Initialize navigation buttons
    initNavButtons();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Calibration prompt
    calibrateIMU();
    
    // Start with first demo
    displayCurrentDemo();
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5 IMU Sensors", M5.Display.width()/2, 30);
    M5.Display.drawString("Demo", M5.Display.width()/2, 70);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Motion Sensors Tutorial", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Features:", M5.Display.width()/2, 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• 3-Axis Accelerometer", M5.Display.width()/2, 180);
    M5.Display.drawString("• 3-Axis Gyroscope", M5.Display.width()/2, 195);
    M5.Display.drawString("• 3-Axis Magnetometer", M5.Display.width()/2, 210);
    M5.Display.drawString("• Orientation Tracking", M5.Display.width()/2, 225);
    M5.Display.drawString("• Motion Detection", M5.Display.width()/2, 240);
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Move and rotate the device", M5.Display.width()/2, 260);
}

void calibrateIMU() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("IMU Calibration", M5.Display.width()/2, 50);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Place device on flat surface", M5.Display.width()/2, 100);
    M5.Display.drawString("Keep it still during calibration", M5.Display.width()/2, 120);
    M5.Display.drawString("Touch screen to start", M5.Display.width()/2, 160);
    
    // Wait for touch to start calibration
    while(true) {
        M5.update();
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) break;
        delay(10);
    }
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Calibrating...", M5.Display.width()/2, 200);
    
    // Perform calibration
    float accSumX = 0, accSumY = 0, accSumZ = 0;
    float gyroSumX = 0, gyroSumY = 0, gyroSumZ = 0;
    int samples = 100;
    
    for (int i = 0; i < samples; i++) {
        if (M5.Imu.getAccel(&accX, &accY, &accZ)) {
            accSumX += accX;
            accSumY += accY;
            accSumZ += accZ;
        }
        if (M5.Imu.getGyro(&gyroX, &gyroY, &gyroZ)) {
            gyroSumX += gyroX;
            gyroSumY += gyroY;
            gyroSumZ += gyroZ;
        }
        
        // Show progress
        int progress = (i * 100) / samples;
        M5.Display.fillRect(50, 220, 200, 20, TFT_DARKGREY);
        M5.Display.fillRect(50, 220, (progress * 200) / 100, 20, TFT_GREEN);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(String(progress) + "%", 150, 230);
        
        delay(50);
    }
    
    // Calculate offsets
    accOffsetX = accSumX / samples;
    accOffsetY = accSumY / samples;
    accOffsetZ = (accSumZ / samples) - 1.0;  // Remove gravity (1g in Z)
    gyroOffsetX = gyroSumX / samples;
    gyroOffsetY = gyroSumY / samples;
    gyroOffsetZ = gyroSumZ / samples;
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Calibration Complete!", M5.Display.width()/2, 260);
    delay(1000);
}

void initNavButtons() {
    int buttonWidth = (M5.Display.width() - 40) / 3;
    int buttonHeight = 50;
    int buttonY = M5.Display.height() - 60;
    
    // Button A - Previous
    navButtons[0] = {
        10, buttonY, buttonWidth, buttonHeight,
        "< Prev", TFT_DARKGREY, false
    };
    
    // Button B - Calibrate
    navButtons[1] = {
        20 + buttonWidth, buttonY, buttonWidth, buttonHeight,
        "Calibrate", TFT_DARKGREY, false
    };
    
    // Button C - Next
    navButtons[2] = {
        30 + buttonWidth * 2, buttonY, buttonWidth, buttonHeight,
        "Next >", TFT_DARKGREY, false
    };
}

void drawNavButtons() {
    for (int i = 0; i < 3; i++) {
        uint16_t color = navButtons[i].isPressed ? TFT_BLUE : navButtons[i].color;
        M5.Display.fillRoundRect(navButtons[i].x, navButtons[i].y,
                                navButtons[i].w, navButtons[i].h, 8, color);
        M5.Display.drawRoundRect(navButtons[i].x, navButtons[i].y,
                                navButtons[i].w, navButtons[i].h, 8, TFT_WHITE);
        
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(1);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(navButtons[i].label,
                             navButtons[i].x + navButtons[i].w/2,
                             navButtons[i].y + navButtons[i].h/2);
    }
}

bool checkNavButtons() {
    auto touch = M5.Touch.getDetail();
    bool buttonPressed = false;
    
    // Reset button states
    for (int i = 0; i < 3; i++) {
        navButtons[i].isPressed = false;
    }
    
    if (touch.isPressed()) {
        for (int i = 0; i < 3; i++) {
            if (touch.x >= navButtons[i].x && touch.x < navButtons[i].x + navButtons[i].w &&
                touch.y >= navButtons[i].y && touch.y < navButtons[i].y + navButtons[i].h) {
                navButtons[i].isPressed = true;
                buttonPressed = true;
            }
        }
    }
    
    if (touch.wasReleased()) {
        for (int i = 0; i < 3; i++) {
            if (touch.x >= navButtons[i].x && touch.x < navButtons[i].x + navButtons[i].w &&
                touch.y >= navButtons[i].y && touch.y < navButtons[i].y + navButtons[i].h) {
                if (i == 0) {  // Previous button
                    currentDemo = (IMUDemo)((currentDemo - 1 + IMU_DEMO_COUNT) % IMU_DEMO_COUNT);
                    displayCurrentDemo();
                } else if (i == 1) {  // Calibrate button
                    calibrateIMU();
                    displayCurrentDemo();
                } else if (i == 2) {  // Next button
                    currentDemo = (IMUDemo)((currentDemo + 1) % IMU_DEMO_COUNT);
                    displayCurrentDemo();
                }
                return true;
            }
        }
    }
    
    return buttonPressed;
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("IMU Sensors", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString(imuDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(IMU_DEMO_COUNT), M5.Display.width()/2, 60);
    
    // Draw demo-specific background
    switch(currentDemo) {
        case DEMO_ACCELEROMETER:
            drawAccelerometerBackground();
            break;
        case DEMO_GYROSCOPE:
            drawGyroscopeBackground();
            break;
        case DEMO_MAGNETOMETER:
            drawMagnetometerBackground();
            break;
        case DEMO_ORIENTATION:
            drawOrientationBackground();
            break;
        case DEMO_MOTION_DETECT:
            drawMotionDetectBackground();
            break;
    }
    
    // Draw navigation buttons
    drawNavButtons();
}

void drawAccelerometerBackground() {
    // Draw much larger 3D visualization area
    M5.Display.drawRect(10, 80, M5.Display.width() - 20, 250, TFT_DARKGREY);
    
    // Draw axis labels
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("X-Axis (Red)", 25, 340);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Y-Axis (Green)", 150, 340);
    M5.Display.setTextColor(TFT_BLUE);
    M5.Display.drawString("Z-Axis (Blue)", 275, 340);
    
    // Draw center point
    M5.Display.fillCircle(M5.Display.width()/2, 205, 4, TFT_WHITE);
}

void drawGyroscopeBackground() {
    // Draw rotation visualization
    M5.Display.drawCircle(160, 140, 80, TFT_DARKGREY);
    M5.Display.drawCircle(160, 140, 40, TFT_DARKGREY);
    
    // Draw axis labels
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("Pitch", 80, 250);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Roll", 160, 250);
    M5.Display.setTextColor(TFT_BLUE);
    M5.Display.drawString("Yaw", 240, 250);
}

void drawMagnetometerBackground() {
    // Draw compass
    M5.Display.drawCircle(160, 140, 80, TFT_DARKGREY);
    
    // Draw compass directions
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("N", 160, 65);
    M5.Display.drawString("S", 160, 215);
    M5.Display.drawString("E", 235, 140);
    M5.Display.drawString("W", 85, 140);
    
    // Draw degree markers
    for (int i = 0; i < 360; i += 30) {
        float rad = i * PI / 180.0;
        int x1 = 160 + 75 * cos(rad);
        int y1 = 140 + 75 * sin(rad);
        int x2 = 160 + 70 * cos(rad);
        int y2 = 140 + 70 * sin(rad);
        M5.Display.drawLine(x1, y1, x2, y2, TFT_DARKGREY);
    }
}

void drawOrientationBackground() {
    // Draw 3D device representation area
    M5.Display.drawRect(50, 80, 220, 120, TFT_DARKGREY);
    
    // Draw orientation labels
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("Pitch:", 25, 210);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Roll:", 25, 225);
    M5.Display.setTextColor(TFT_BLUE);
    M5.Display.drawString("Yaw:", 25, 240);
}

void drawMotionDetectBackground() {
    // Draw motion detection area
    M5.Display.drawRect(50, 80, 220, 120, TFT_DARKGREY);
    
    // Draw threshold control
    M5.Display.drawRect(20, 220, 280, 20, TFT_DARKGREY);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Motion Threshold: " + String(motionThreshold, 1) + "g", 25, 210);
}

void readIMUData() {
    // Read accelerometer
    if (M5.Imu.getAccel(&accX, &accY, &accZ)) {
        // Apply calibration offsets
        accX -= accOffsetX;
        accY -= accOffsetY;
        accZ -= accOffsetZ;
    }
    
    // Read gyroscope
    if (M5.Imu.getGyro(&gyroX, &gyroY, &gyroZ)) {
        // Apply calibration offsets
        gyroX -= gyroOffsetX;
        gyroY -= gyroOffsetY;
        gyroZ -= gyroOffsetZ;
    }
    
    // Read magnetometer (if available)
    M5.Imu.getMag(&magX, &magY, &magZ);
    
    // Calculate orientation
    pitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;
    roll = atan2(accY, accZ) * 180.0 / PI;
    yaw = atan2(-magY, magX) * 180.0 / PI;
    
    // Calculate motion
    prevAccMagnitude = accMagnitude;
    accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
    float motionChange = abs(accMagnitude - prevAccMagnitude);
    motionDetected = (motionChange > motionThreshold);
    
    if (motionDetected) {
        lastMotionTime = millis();
    }
    
    // Update history
    accHistory[historyIndex] = accMagnitude;
    gyroHistory[historyIndex] = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
}

void handleAccelerometerDemo() {
    // Clear much larger visualization area
    M5.Display.fillRect(11, 81, M5.Display.width() - 22, 248, TFT_BLACK);
    
    // Draw 3D acceleration vectors with much larger scale
    int centerX = M5.Display.width()/2;
    int centerY = 205;
    int scale = 100;  // Much larger scale for better visibility
    
    // Draw thicker lines for better visibility
    // X-axis (red) - horizontal
    int xEndX = centerX + constrain(accX * scale, -190, 190);
    int xEndY = centerY;
    M5.Display.drawLine(centerX, centerY, xEndX, xEndY, TFT_RED);
    M5.Display.drawLine(centerX, centerY-1, xEndX, xEndY-1, TFT_RED);
    M5.Display.drawLine(centerX, centerY+1, xEndX, xEndY+1, TFT_RED);
    // Arrowhead
    int xArrowDir = (xEndX > centerX) ? 1 : -1;
    M5.Display.fillTriangle(xEndX, xEndY, 
                           xEndX - xArrowDir*8, xEndY - 5, 
                           xEndX - xArrowDir*8, xEndY + 5, TFT_RED);
    
    // Y-axis (green) - vertical
    int yEndX = centerX;
    int yEndY = centerY + constrain(accY * scale, -115, 115);
    M5.Display.drawLine(centerX, centerY, yEndX, yEndY, TFT_GREEN);
    M5.Display.drawLine(centerX-1, centerY, yEndX-1, yEndY, TFT_GREEN);
    M5.Display.drawLine(centerX+1, centerY, yEndX+1, yEndY, TFT_GREEN);
    // Arrowhead
    int yArrowDir = (yEndY > centerY) ? 1 : -1;
    M5.Display.fillTriangle(yEndX, yEndY,
                           yEndX - 5, yEndY - yArrowDir*8,
                           yEndX + 5, yEndY - yArrowDir*8, TFT_GREEN);
    
    // Z-axis (blue) - represented as circle size
    int zRadius = constrain(abs(accZ) * 40 + 10, 10, 80);
    M5.Display.drawCircle(centerX, centerY, zRadius, TFT_BLUE);
    M5.Display.drawCircle(centerX, centerY, zRadius-1, TFT_BLUE);
    if (accZ < 0) {
        // Draw dashed circle for negative Z
        for (int angle = 0; angle < 360; angle += 20) {
            float rad = angle * PI / 180.0;
            int x = centerX + zRadius * cos(rad);
            int y = centerY + zRadius * sin(rad);
            M5.Display.fillCircle(x, y, 2, TFT_BLUE);
        }
    }
    
    // Display values with larger text
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("X:" + String(accX, 2) + "g", 25, 355);
    M5.Display.drawString("Y:" + String(accY, 2) + "g", 150, 355);
    M5.Display.drawString("Z:" + String(accZ, 2) + "g", 275, 355);
    
    // Draw magnitude
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Magnitude: " + String(accMagnitude, 2) + "g", 25, 380);
}

void handleGyroscopeDemo() {
    // Clear visualization area
    M5.Display.fillCircle(160, 140, 78, TFT_BLACK);
    
    // Draw rotation indicators
    int centerX = 160;
    int centerY = 140;
    
    // Pitch (X rotation) - red
    float pitchRad = gyroX * PI / 180.0;
    int pitchX = centerX + 60 * cos(pitchRad);
    int pitchY = centerY + 60 * sin(pitchRad);
    M5.Display.drawLine(centerX, centerY, pitchX, pitchY, TFT_RED);
    
    // Roll (Y rotation) - green
    float rollRad = gyroY * PI / 180.0;
    int rollX = centerX + 40 * cos(rollRad);
    int rollY = centerY + 40 * sin(rollRad);
    M5.Display.drawLine(centerX, centerY, rollX, rollY, TFT_GREEN);
    
    // Yaw (Z rotation) - blue
    float yawRad = gyroZ * PI / 180.0;
    int yawX = centerX + 20 * cos(yawRad);
    int yawY = centerY + 20 * sin(yawRad);
    M5.Display.drawLine(centerX, centerY, yawX, yawY, TFT_BLUE);
    
    // Display angular velocities
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString(String(gyroX, 1) + "°/s", 80, 260);
    M5.Display.drawString(String(gyroY, 1) + "°/s", 160, 260);
    M5.Display.drawString(String(gyroZ, 1) + "°/s", 240, 260);
}

void handleMagnetometerDemo() {
    // Clear compass needle
    M5.Display.fillCircle(160, 140, 78, TFT_BLACK);
    
    // Calculate compass heading
    float heading = atan2(magY, magX) * 180.0 / PI;
    if (heading < 0) heading += 360;
    
    // Draw compass needle
    int centerX = 160;
    int centerY = 140;
    float headingRad = (heading - 90) * PI / 180.0;  // Adjust for screen orientation
    int needleX = centerX + 70 * cos(headingRad);
    int needleY = centerY + 70 * sin(headingRad);
    
    M5.Display.drawLine(centerX, centerY, needleX, needleY, TFT_RED);
    M5.Display.fillTriangle(needleX, needleY, 
                           needleX - 8 * cos(headingRad + 2.6), 
                           needleY - 8 * sin(headingRad + 2.6),
                           needleX - 8 * cos(headingRad - 2.6), 
                           needleY - 8 * sin(headingRad - 2.6), TFT_RED);
    
    // Display heading
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString(String(int(heading)) + "°", 160, 230);
    
    // Display raw values
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("X: " + String(magX, 1), 25, 260);
    M5.Display.drawString("Y: " + String(magY, 1), 120, 260);
    M5.Display.drawString("Z: " + String(magZ, 1), 215, 260);
}

void handleOrientationDemo() {
    // Clear device representation area
    M5.Display.fillRect(51, 81, 218, 118, TFT_BLACK);
    
    // Draw simplified 3D device representation
    int centerX = 160;
    int centerY = 140;
    
    // Calculate device corners based on orientation
    float pitchRad = pitch * PI / 180.0;
    float rollRad = roll * PI / 180.0;
    
    // Simple 2D projection of rotated rectangle
    int w = 60, h = 40;
    int corners[4][2];
    
    // Original corners
    corners[0][0] = -w/2; corners[0][1] = -h/2;  // Top-left
    corners[1][0] =  w/2; corners[1][1] = -h/2;  // Top-right
    corners[2][0] =  w/2; corners[2][1] =  h/2;  // Bottom-right
    corners[3][0] = -w/2; corners[3][1] =  h/2;  // Bottom-left
    
    // Apply rotation and draw
    for (int i = 0; i < 4; i++) {
        float x = corners[i][0] * cos(rollRad) - corners[i][1] * sin(pitchRad);
        float y = corners[i][0] * sin(rollRad) + corners[i][1] * cos(pitchRad);
        corners[i][0] = centerX + x;
        corners[i][1] = centerY + y;
    }
    
    // Draw device outline
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        M5.Display.drawLine(corners[i][0], corners[i][1], corners[next][0], corners[next][1], TFT_CYAN);
    }
    
    // Fill device
    // Simple fill by drawing horizontal lines (simplified)
    M5.Display.fillRect(centerX - 30, centerY - 20, 60, 40, TFT_DARKGREY);
    
    // Display orientation values
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString(String(pitch, 1) + "°", 80, 210);
    M5.Display.drawString(String(roll, 1) + "°", 80, 225);
    M5.Display.drawString(String(yaw, 1) + "°", 80, 240);
}

void handleMotionDetectDemo() {
    // Handle threshold adjustment
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed() && touch.y >= 220 && touch.y <= 240) {
            motionThreshold = constrain((float)(touch.x - 20) / 260.0 * 2.0, 0.1, 2.0);
            
            // Redraw threshold indicator
            M5.Display.fillRect(21, 221, 278, 18, TFT_BLACK);
            int thresholdX = 20 + (motionThreshold / 2.0) * 260;
            M5.Display.drawLine(thresholdX, 220, thresholdX, 240, TFT_YELLOW);
            
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.setTextDatum(TL_DATUM);
            M5.Display.drawString("Motion Threshold: " + String(motionThreshold, 1) + "g", 25, 210);
        }
    }
    
    // Clear motion area
    M5.Display.fillRect(51, 81, 218, 118, TFT_BLACK);
    
    // Visual feedback for motion detection
    if (motionDetected || (millis() - lastMotionTime < 500)) {
        M5.Display.fillRect(51, 81, 218, 118, TFT_RED);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(3);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("MOTION!", 160, 140);
    } else {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Still", 160, 140);
    }
    
    // Draw motion level bar
    int motionLevel = constrain(abs(accMagnitude - prevAccMagnitude) / 2.0 * 100, 0, 100);
    M5.Display.fillRect(60, 160, 200, 15, TFT_DARKGREY);
    uint16_t barColor = motionDetected ? TFT_RED : TFT_GREEN;
    M5.Display.fillRect(60, 160, motionLevel * 2, 15, barColor);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(String(motionLevel) + "%", 160, 185);
}

void loop() {
    M5.update();
    
    // Read IMU data
    readIMUData();
    
    // Handle navigation button presses
    bool navButtonChanged = checkNavButtons();
    if (navButtonChanged) {
        drawNavButtons();
    }
    
    // Handle current demo
    switch(currentDemo) {
        case DEMO_ACCELEROMETER:
            handleAccelerometerDemo();
            break;
        case DEMO_GYROSCOPE:
            handleGyroscopeDemo();
            break;
        case DEMO_MAGNETOMETER:
            handleMagnetometerDemo();
            break;
        case DEMO_ORIENTATION:
            handleOrientationDemo();
            break;
        case DEMO_MOTION_DETECT:
            handleMotionDetectDemo();
            break;
    }
    
    // Redraw buttons if needed
    static unsigned long lastButtonDraw = 0;
    if (millis() - lastButtonDraw > 100) {
        drawNavButtons();
        lastButtonDraw = millis();
    }
    
    delay(50);  // 20 FPS update rate
}