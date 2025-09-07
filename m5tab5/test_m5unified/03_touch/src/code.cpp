/*
 * M5Unified Tutorial - 03_touch
 * 
 * This demo explores touch interface capabilities:
 * - Single touch point detection
 * - Multi-touch support (if available)
 * - Touch coordinates and pressure
 * - Gesture recognition (swipe, pinch, tap)
 * - Touch zones and buttons
 * - Drawing with touch
 * 
 * Key concepts:
 * - M5.Touch.getDetail() returns detailed touch information
 * - wasPressed(), wasReleased(), isPressed() for touch states
 * - Touch coordinates are screen-relative
 * - Gesture detection through movement tracking
 */

#include <M5Unified.h>
#include <vector>

// Touch tracking structures
struct TouchPoint {
    int x, y;
    unsigned long timestamp;
};

std::vector<TouchPoint> touchHistory;
const int MAX_HISTORY = 50;

// Gesture detection variables
enum Gesture {
    NONE,
    TAP,
    DOUBLE_TAP,
    LONG_PRESS,
    SWIPE_UP,
    SWIPE_DOWN,
    SWIPE_LEFT,
    SWIPE_RIGHT
};

Gesture lastGesture = NONE;
unsigned long lastTapTime = 0;
int tapCount = 0;

// Touch zones for virtual buttons
struct TouchZone {
    int x, y, w, h;
    const char* label;
    uint16_t color;
    bool active;
};

TouchZone zones[6];

// Drawing mode variables
bool drawingMode = false;
uint16_t drawColor = TFT_WHITE;
int brushSize = 3;

void initializeTouchZones() {
    // Create 6 touch zones
    int zoneWidth = M5.Display.width() / 3;
    int zoneHeight = 40;
    int startY = M5.Display.height() - 90;
    
    zones[0] = {0, startY, zoneWidth, zoneHeight, "Clear", TFT_RED, false};
    zones[1] = {zoneWidth, startY, zoneWidth, zoneHeight, "Draw", TFT_GREEN, false};
    zones[2] = {zoneWidth * 2, startY, zoneWidth, zoneHeight, "Color", TFT_BLUE, false};
    
    zones[3] = {0, startY + zoneHeight + 5, zoneWidth, zoneHeight, "Size-", TFT_YELLOW, false};
    zones[4] = {zoneWidth, startY + zoneHeight + 5, zoneWidth, zoneHeight, "Size+", TFT_CYAN, false};
    zones[5] = {zoneWidth * 2, startY + zoneHeight + 5, zoneWidth, zoneHeight, "Info", TFT_MAGENTA, false};
}

void drawInterface() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Title
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Touch Demo", M5.Display.width()/2, 5);
    
    // Mode indicator
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(drawingMode ? TFT_GREEN : TFT_WHITE);
    M5.Display.drawString("Mode: " + String(drawingMode ? "Drawing" : "Touch Test"), 10, 25);
    
    // Current settings
    M5.Display.drawString("Brush Size: " + String(brushSize), 10, 40);
    M5.Display.fillCircle(150, 45, brushSize, drawColor);
    
    // Draw touch zones
    for (int i = 0; i < 6; i++) {
        uint16_t color = zones[i].active ? TFT_WHITE : zones[i].color;
        M5.Display.fillRoundRect(zones[i].x, zones[i].y, zones[i].w, zones[i].h, 5, color);
        M5.Display.setTextColor(zones[i].active ? zones[i].color : TFT_BLACK);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(zones[i].label, zones[i].x + zones[i].w/2, zones[i].y + zones[i].h/2);
    }
    
    // Gesture area indicator
    M5.Display.drawRect(10, 60, M5.Display.width() - 20, M5.Display.height() - 160, TFT_DARKGREY);
}

Gesture detectGesture() {
    if (touchHistory.size() < 2) return NONE;
    
    TouchPoint start = touchHistory[0];
    TouchPoint end = touchHistory[touchHistory.size() - 1];
    
    int deltaX = end.x - start.x;
    int deltaY = end.y - start.y;
    int distance = sqrt(deltaX * deltaX + deltaY * deltaY);
    unsigned long duration = end.timestamp - start.timestamp;
    
    // Long press detection
    if (distance < 10 && duration > 1000) {
        return LONG_PRESS;
    }
    
    // Swipe detection
    if (distance > 50 && duration < 500) {
        if (abs(deltaX) > abs(deltaY)) {
            // Horizontal swipe
            return (deltaX > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
        } else {
            // Vertical swipe
            return (deltaY > 0) ? SWIPE_DOWN : SWIPE_UP;
        }
    }
    
    // Tap detection
    if (distance < 10 && duration < 200) {
        return TAP;
    }
    
    return NONE;
}

void handleTouchZone(int x, int y) {
    for (int i = 0; i < 6; i++) {
        if (x >= zones[i].x && x < zones[i].x + zones[i].w &&
            y >= zones[i].y && y < zones[i].y + zones[i].h) {
            
            zones[i].active = true;
            
            // Handle zone actions
            switch(i) {
                case 0: // Clear
                    drawInterface();
                    break;
                    
                case 1: // Draw mode toggle
                    drawingMode = !drawingMode;
                    drawInterface();
                    break;
                    
                case 2: // Change color
                    {
                        uint16_t colors[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, 
                                           TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
                        static int colorIndex = 0;
                        colorIndex = (colorIndex + 1) % 7;
                        drawColor = colors[colorIndex];
                        drawInterface();
                    }
                    break;
                    
                case 3: // Size decrease
                    brushSize = max(1, brushSize - 2);
                    drawInterface();
                    break;
                    
                case 4: // Size increase
                    brushSize = min(20, brushSize + 2);
                    drawInterface();
                    break;
                    
                case 5: // Show info
                    M5.Display.fillRect(10, 60, M5.Display.width() - 20, 100, TFT_BLACK);
                    M5.Display.setTextColor(TFT_CYAN);
                    M5.Display.setTextDatum(TL_DATUM);
                    M5.Display.drawString("Touch Info:", 15, 65);
                    M5.Display.setTextColor(TFT_WHITE);
                    M5.Display.drawString("Points: " + String(touchHistory.size()), 15, 80);
                    M5.Display.drawString("Last Gesture: " + String(lastGesture), 15, 95);
                    M5.Display.drawString("Tap Count: " + String(tapCount), 15, 110);
                    break;
            }
            
            // Audio feedback
            if (M5.Speaker.isEnabled()) {
                M5.Speaker.tone(1000 + i * 200, 50);
            }
            
            return;
        }
    }
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.setBrightness(128);
    
    // Check touch capability
    if (!M5.Touch.isEnabled()) {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Touch Not Available", M5.Display.width()/2, M5.Display.height()/2);
        while(1) delay(100);
    }
    
    initializeTouchZones();
    drawInterface();
    
    // Welcome sound
    if (M5.Speaker.isEnabled()) {
        M5.Speaker.tone(1000, 100);
        delay(100);
        M5.Speaker.tone(1500, 100);
    }
}

void loop() {
    M5.update();
    
    static bool wasPressed = false;
    static unsigned long pressStartTime = 0;
    static int lastX = 0, lastY = 0;
    
    auto touch = M5.Touch.getDetail();
    
    if (touch.wasPressed()) {
        wasPressed = true;
        pressStartTime = millis();
        lastX = touch.x;
        lastY = touch.y;
        
        // Clear touch history for new gesture
        touchHistory.clear();
        touchHistory.push_back({touch.x, touch.y, millis()});
        
        // Reset zone states
        for (int i = 0; i < 6; i++) {
            zones[i].active = false;
        }
        
        // Check if touch is in a zone
        handleTouchZone(touch.x, touch.y);
        
        // Double tap detection
        if (millis() - lastTapTime < 300) {
            tapCount++;
            if (tapCount == 2) {
                lastGesture = DOUBLE_TAP;
                M5.Display.fillRect(10, 60, 200, 30, TFT_BLACK);
                M5.Display.setTextColor(TFT_YELLOW);
                M5.Display.setTextDatum(TL_DATUM);
                M5.Display.drawString("DOUBLE TAP!", 15, 65);
                
                if (M5.Speaker.isEnabled()) {
                    M5.Speaker.tone(2000, 50);
                    delay(50);
                    M5.Speaker.tone(2000, 50);
                }
                
                tapCount = 0;
            }
        } else {
            tapCount = 1;
        }
        lastTapTime = millis();
    }
    
    if (touch.isPressed()) {
        // Add to touch history
        if (touchHistory.size() < MAX_HISTORY) {
            touchHistory.push_back({touch.x, touch.y, millis()});
        }
        
        // Drawing mode
        if (drawingMode && touch.y > 60 && touch.y < M5.Display.height() - 100) {
            // Draw line from last position
            if (abs(touch.x - lastX) < 50 && abs(touch.y - lastY) < 50) {
                M5.Display.drawLine(lastX, lastY, touch.x, touch.y, drawColor);
                M5.Display.fillCircle(touch.x, touch.y, brushSize/2, drawColor);
            }
            lastX = touch.x;
            lastY = touch.y;
        } else if (!drawingMode) {
            // Show touch position
            M5.Display.fillRect(200, 25, 120, 15, TFT_BLACK);
            M5.Display.setTextColor(TFT_GREEN);
            M5.Display.setTextSize(1);
            M5.Display.setTextDatum(TL_DATUM);
            M5.Display.drawString("Touch: (" + String(touch.x) + ", " + String(touch.y) + ")", 200, 25);
            
            // Draw touch indicator
            M5.Display.drawCircle(touch.x, touch.y, 10, TFT_GREEN);
            M5.Display.drawLine(touch.x - 15, touch.y, touch.x + 15, touch.y, TFT_GREEN);
            M5.Display.drawLine(touch.x, touch.y - 15, touch.x, touch.y + 15, TFT_GREEN);
        }
        
        // Check for long press
        if (millis() - pressStartTime > 1000 && touchHistory.size() > 0) {
            TouchPoint start = touchHistory[0];
            int distance = sqrt(pow(touch.x - start.x, 2) + pow(touch.y - start.y, 2));
            
            if (distance < 10 && lastGesture != LONG_PRESS) {
                lastGesture = LONG_PRESS;
                M5.Display.fillRect(10, 60, 200, 30, TFT_BLACK);
                M5.Display.setTextColor(TFT_MAGENTA);
                M5.Display.setTextDatum(TL_DATUM);
                M5.Display.drawString("LONG PRESS!", 15, 65);
                
                if (M5.Speaker.isEnabled()) {
                    M5.Speaker.tone(500, 500);
                }
            }
        }
    }
    
    if (touch.wasReleased()) {
        wasPressed = false;
        
        // Reset zone states
        for (int i = 0; i < 6; i++) {
            zones[i].active = false;
        }
        drawInterface();
        
        // Detect gesture
        Gesture gesture = detectGesture();
        if (gesture != NONE && gesture != lastGesture) {
            lastGesture = gesture;
            
            M5.Display.fillRect(10, 60, 200, 30, TFT_BLACK);
            M5.Display.setTextColor(TFT_CYAN);
            M5.Display.setTextDatum(TL_DATUM);
            
            String gestureText = "";
            switch(gesture) {
                case TAP: gestureText = "TAP"; break;
                case SWIPE_UP: gestureText = "SWIPE UP"; break;
                case SWIPE_DOWN: gestureText = "SWIPE DOWN"; break;
                case SWIPE_LEFT: gestureText = "SWIPE LEFT"; break;
                case SWIPE_RIGHT: gestureText = "SWIPE RIGHT"; break;
                default: break;
            }
            
            if (gestureText != "") {
                M5.Display.drawString(gestureText, 15, 65);
                
                if (M5.Speaker.isEnabled()) {
                    M5.Speaker.tone(1500, 100);
                }
            }
        }
    }
    
    delay(10);
}