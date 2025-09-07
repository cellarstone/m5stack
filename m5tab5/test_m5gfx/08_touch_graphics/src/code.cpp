/*
 * M5GFX Tutorial - 08_touch_graphics
 * 
 * This demo demonstrates interactive graphics with touch input in M5GFX:
 * - Touch-responsive buttons and UI elements
 * - Drag and drop functionality
 * - Drawing and painting with touch
 * - Gesture recognition basics
 * - Interactive UI components
 * - Multi-touch handling concepts
 * - Touch feedback and visual responses
 * - Real-time touch-based graphics
 * 
 * Key concepts:
 * - M5.Touch API usage
 * - Touch state management
 * - Interactive UI design patterns
 * - Real-time input processing
 * - Touch-based animation triggers
 */

#include <M5Unified.h>
#include <math.h>

// Demo modes for different touch interaction features
enum TouchDemo {
    DEMO_TOUCH_BUTTONS,
    DEMO_DRAG_DROP,
    DEMO_DRAWING_PAINT,
    DEMO_GESTURE_RECOGNITION,
    DEMO_INTERACTIVE_UI,
    DEMO_TOUCH_EFFECTS,
    TOUCH_DEMO_COUNT
};

TouchDemo currentDemo = DEMO_TOUCH_BUTTONS;
const char* touchDemoNames[] = {
    "Touch Buttons",
    "Drag & Drop",
    "Drawing & Paint",
    "Gesture Recognition",
    "Interactive UI",
    "Touch Effects"
};

// Touch tracking
struct TouchState {
    bool isPressed;
    bool wasPressed;
    bool wasReleased;
    int x, y;
    int lastX, lastY;
    unsigned long pressTime;
    unsigned long releaseTime;
};

TouchState touch;

// Button structure
struct TouchButton {
    int x, y, width, height;
    String label;
    uint16_t color;
    uint16_t pressedColor;
    bool isPressed;
    bool isEnabled;
};

// Drawing/painting
const int MAX_PAINT_POINTS = 500;
struct PaintPoint {
    int x, y;
    uint16_t color;
    int size;
    bool active;
};
PaintPoint paintPoints[MAX_PAINT_POINTS];
int paintIndex = 0;
uint16_t currentPaintColor = TFT_WHITE;
int currentBrushSize = 3;

// Drag and drop objects
struct DragObject {
    float x, y;
    int width, height;
    uint16_t color;
    String label;
    bool isDragging;
    bool isSelected;
    int dragOffsetX, dragOffsetY;
};

const int MAX_DRAG_OBJECTS = 6;
DragObject dragObjects[MAX_DRAG_OBJECTS];

// Gesture recognition
struct GesturePoint {
    int x, y;
    unsigned long time;
};
const int MAX_GESTURE_POINTS = 20;
GesturePoint gesturePoints[MAX_GESTURE_POINTS];
int gesturePointCount = 0;
String detectedGesture = "";

// Interactive UI components
struct Slider {
    int x, y, width;
    float value; // 0.0 to 1.0
    String label;
    bool isDragging;
};

struct ToggleSwitch {
    int x, y;
    bool isOn;
    String label;
};

// Animation variables
unsigned long lastUpdate = 0;
int animationStep = 0;
float rippleTime = 0;
int rippleX = 0, rippleY = 0;
bool showRipple = false;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize touch system
    initTouchSystem();
    
    // Initialize drag objects
    initDragObjects();
    
    // Initialize paint system
    initPaintSystem();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void initTouchSystem() {
    touch.isPressed = false;
    touch.wasPressed = false;
    touch.wasReleased = false;
    touch.x = 0;
    touch.y = 0;
    touch.lastX = 0;
    touch.lastY = 0;
    touch.pressTime = 0;
    touch.releaseTime = 0;
}

void initDragObjects() {
    String labels[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
    
    for (int i = 0; i < MAX_DRAG_OBJECTS; i++) {
        dragObjects[i].x = 20 + (i % 3) * 80;
        dragObjects[i].y = 120 + (i / 3) * 60;
        dragObjects[i].width = 60;
        dragObjects[i].height = 40;
        dragObjects[i].color = colors[i];
        dragObjects[i].label = labels[i];
        dragObjects[i].isDragging = false;
        dragObjects[i].isSelected = false;
        dragObjects[i].dragOffsetX = 0;
        dragObjects[i].dragOffsetY = 0;
    }
}

void initPaintSystem() {
    for (int i = 0; i < MAX_PAINT_POINTS; i++) {
        paintPoints[i].active = false;
    }
    paintIndex = 0;
}

void updateTouch() {
    // Store previous state
    bool prevPressed = touch.isPressed;
    
    // Update current touch state
    M5.update();
    if (M5.Touch.isEnabled()) {
        auto touchDetail = M5.Touch.getDetail();
        touch.lastX = touch.x;
        touch.lastY = touch.y;
        touch.x = touchDetail.x;
        touch.y = touchDetail.y;
        touch.isPressed = touchDetail.isPressed();
        
        // Detect press and release events
        touch.wasPressed = touch.isPressed && !prevPressed;
        touch.wasReleased = !touch.isPressed && prevPressed;
        
        if (touch.wasPressed) {
            touch.pressTime = millis();
        }
        if (touch.wasReleased) {
            touch.releaseTime = millis();
        }
    } else {
        touch.isPressed = false;
        touch.wasPressed = false;
        touch.wasReleased = false;
    }
}

bool isPointInRect(int px, int py, int x, int y, int width, int height) {
    return (px >= x && px <= x + width && py >= y && py <= y + height);
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Touch", M5.Display.width()/2, 40);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Interactive", M5.Display.width()/2, 90);
    M5.Display.drawString("Graphics", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Touch-responsive graphics and UI elements", M5.Display.width()/2, 160);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Touch Buttons & Controls", M5.Display.width()/2, 180);
    M5.Display.drawString("• Drag & Drop Interactions", M5.Display.width()/2, 195);
    M5.Display.drawString("• Drawing & Gesture Recognition", M5.Display.width()/2, 210);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Touch Graphics", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.drawString(touchDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(TOUCH_DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Touch status
    if (M5.Touch.isEnabled()) {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Touch: ENABLED", M5.Display.width()/2, 65);
    } else {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Touch: DISABLED", M5.Display.width()/2, 65);
    }
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("[A] Prev  [B] Clear  [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Draw demo-specific content
    drawCurrentTouchDemo();
}

void drawCurrentTouchDemo() {
    switch(currentDemo) {
        case DEMO_TOUCH_BUTTONS:
            drawTouchButtonsDemo();
            break;
        case DEMO_DRAG_DROP:
            drawDragDropDemo();
            break;
        case DEMO_DRAWING_PAINT:
            drawDrawingPaintDemo();
            break;
        case DEMO_GESTURE_RECOGNITION:
            drawGestureRecognitionDemo();
            break;
        case DEMO_INTERACTIVE_UI:
            drawInteractiveUIDemo();
            break;
        case DEMO_TOUCH_EFFECTS:
            drawTouchEffectsDemo();
            break;
    }
}

void drawTouchButton(TouchButton &button) {
    uint16_t color = button.isPressed ? button.pressedColor : button.color;
    
    // Draw button background
    M5.Display.fillRoundRect(button.x, button.y, button.width, button.height, 5, color);
    M5.Display.drawRoundRect(button.x, button.y, button.width, button.height, 5, TFT_WHITE);
    
    // Draw button label
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(button.label, button.x + button.width/2, button.y + button.height/2);
    
    // Visual feedback for pressed state
    if (button.isPressed) {
        M5.Display.drawRoundRect(button.x + 2, button.y + 2, button.width - 4, button.height - 4, 3, TFT_WHITE);
    }
}

bool updateTouchButton(TouchButton &button) {
    bool wasClicked = false;
    
    if (touch.isPressed && isPointInRect(touch.x, touch.y, button.x, button.y, button.width, button.height)) {
        button.isPressed = true;
    } else {
        if (button.isPressed && touch.wasReleased) {
            wasClicked = true;
        }
        button.isPressed = false;
    }
    
    return wasClicked;
}

void drawTouchButtonsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Touch Button Examples", 10, startY);
    
    // Create buttons
    static TouchButton buttons[6];
    static bool buttonsInitialized = false;
    static String buttonStatus = "Touch a button!";
    
    if (!buttonsInitialized) {
        // Initialize buttons
        String labels[] = {"Button 1", "Button 2", "Toggle", "Action", "Reset", "OK"};
        uint16_t colors[] = {TFT_BLUE, TFT_GREEN, TFT_ORANGE, TFT_RED, TFT_PURPLE, TFT_CYAN};
        
        for (int i = 0; i < 6; i++) {
            buttons[i].x = 20 + (i % 3) * 120;
            buttons[i].y = startY + 30 + (i / 3) * 60;
            buttons[i].width = 100;
            buttons[i].height = 40;
            buttons[i].label = labels[i];
            buttons[i].color = colors[i];
            buttons[i].pressedColor = TFT_WHITE;
            buttons[i].isPressed = false;
            buttons[i].isEnabled = true;
        }
        buttonsInitialized = true;
    }
    
    // Update and draw buttons
    for (int i = 0; i < 6; i++) {
        if (updateTouchButton(buttons[i])) {
            buttonStatus = buttons[i].label + " clicked!";
            
            // Special button actions
            if (i == 2) { // Toggle button
                static bool toggleState = false;
                toggleState = !toggleState;
                buttons[i].color = toggleState ? TFT_YELLOW : TFT_ORANGE;
                buttonStatus += " (State: " + String(toggleState ? "ON" : "OFF") + ")";
            }
            if (i == 4) { // Reset button
                buttonStatus = "All buttons reset!";
                buttons[2].color = TFT_ORANGE; // Reset toggle
            }
        }
        drawTouchButton(buttons[i]);
    }
    
    // Status display
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Status: " + buttonStatus, 10, startY + 160);
    
    // Touch coordinates
    if (touch.isPressed) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("Touch: (" + String(touch.x) + ", " + String(touch.y) + ")", 10, startY + 180);
        
        // Draw touch indicator
        M5.Display.drawCircle(touch.x, touch.y, 10, TFT_RED);
        M5.Display.drawCircle(touch.x, touch.y, 5, TFT_YELLOW);
    }
    
    // Button design tips
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Button Design Tips:", 300, startY + 30);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Minimum 40px height", 300, startY + 45);
    M5.Display.drawString("• Clear visual feedback", 300, startY + 60);
    M5.Display.drawString("• Rounded corners", 300, startY + 75);
    M5.Display.drawString("• High contrast text", 300, startY + 90);
    M5.Display.drawString("• Pressed state indication", 300, startY + 105);
}

void drawDragDropDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Drag & Drop Interface", 10, startY);
    
    // Drop zones
    M5.Display.drawRect(300, startY + 30, 120, 80, TFT_DARKGREY);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Drop Zone 1", 360, startY + 50);
    
    M5.Display.drawRect(300, startY + 130, 120, 80, TFT_DARKGREY);
    M5.Display.drawString("Drop Zone 2", 360, startY + 150);
    
    // Handle drag and drop
    static int selectedObject = -1;
    static String dropStatus = "Drag objects to zones";
    
    if (touch.wasPressed) {
        // Check if touch started on an object
        selectedObject = -1;
        for (int i = 0; i < MAX_DRAG_OBJECTS; i++) {
            if (isPointInRect(touch.x, touch.y, dragObjects[i].x, dragObjects[i].y, 
                             dragObjects[i].width, dragObjects[i].height)) {
                selectedObject = i;
                dragObjects[i].isDragging = true;
                dragObjects[i].dragOffsetX = touch.x - dragObjects[i].x;
                dragObjects[i].dragOffsetY = touch.y - dragObjects[i].y;
                break;
            }
        }
    }
    
    if (touch.isPressed && selectedObject != -1) {
        // Update dragging object position
        dragObjects[selectedObject].x = touch.x - dragObjects[selectedObject].dragOffsetX;
        dragObjects[selectedObject].y = touch.y - dragObjects[selectedObject].dragOffsetY;
    }
    
    if (touch.wasReleased && selectedObject != -1) {
        // Check drop zones
        int centerX = dragObjects[selectedObject].x + dragObjects[selectedObject].width / 2;
        int centerY = dragObjects[selectedObject].y + dragObjects[selectedObject].height / 2;
        
        if (isPointInRect(centerX, centerY, 300, startY + 30, 120, 80)) {
            dropStatus = dragObjects[selectedObject].label + " dropped in Zone 1";
        } else if (isPointInRect(centerX, centerY, 300, startY + 130, 120, 80)) {
            dropStatus = dragObjects[selectedObject].label + " dropped in Zone 2";
        } else {
            dropStatus = dragObjects[selectedObject].label + " dropped outside zones";
        }
        
        dragObjects[selectedObject].isDragging = false;
        selectedObject = -1;
    }
    
    // Draw drag objects
    for (int i = 0; i < MAX_DRAG_OBJECTS; i++) {
        uint16_t color = dragObjects[i].color;
        
        // Highlight if dragging
        if (dragObjects[i].isDragging) {
            M5.Display.drawRect(dragObjects[i].x - 2, dragObjects[i].y - 2, 
                               dragObjects[i].width + 4, dragObjects[i].height + 4, TFT_WHITE);
        }
        
        // Draw object
        M5.Display.fillRoundRect(dragObjects[i].x, dragObjects[i].y, 
                                dragObjects[i].width, dragObjects[i].height, 5, color);
        M5.Display.drawRoundRect(dragObjects[i].x, dragObjects[i].y, 
                                dragObjects[i].width, dragObjects[i].height, 5, TFT_WHITE);
        
        // Draw label
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(dragObjects[i].label, 
                             dragObjects[i].x + dragObjects[i].width/2, 
                             dragObjects[i].y + dragObjects[i].height/2);
    }
    
    // Status
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Status: " + dropStatus, 10, startY + 20);
    
    if (selectedObject != -1) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("Dragging: " + dragObjects[selectedObject].label, 10, startY + 35);
    }
    
    // Instructions
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Instructions:", 10, startY + 220);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("1. Touch and hold an object", 10, startY + 235);
    M5.Display.drawString("2. Drag to move", 150, startY + 235);
    M5.Display.drawString("3. Release in drop zone", 250, startY + 235);
}

void drawDrawingPaintDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Drawing & Painting", 10, startY);
    
    // Color palette
    uint16_t palette[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, 
                         TFT_CYAN, TFT_MAGENTA, TFT_ORANGE, TFT_BLACK};
    int paletteSize = 9;
    
    for (int i = 0; i < paletteSize; i++) {
        int palX = 10 + i * 30;
        int palY = startY + 20;
        
        M5.Display.fillRect(palX, palY, 25, 25, palette[i]);
        if (palette[i] == currentPaintColor) {
            M5.Display.drawRect(palX - 2, palY - 2, 29, 29, TFT_WHITE);
            M5.Display.drawRect(palX - 1, palY - 1, 27, 27, TFT_WHITE);
        } else {
            M5.Display.drawRect(palX, palY, 25, 25, TFT_DARKGREY);
        }
        
        // Check for palette selection
        if (touch.wasPressed && isPointInRect(touch.x, touch.y, palX, palY, 25, 25)) {
            currentPaintColor = palette[i];
        }
    }
    
    // Brush size selector
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Size:", 300, startY + 25);
    
    for (int i = 1; i <= 5; i++) {
        int sizeX = 330 + i * 25;
        int sizeY = startY + 20;
        
        M5.Display.drawRect(sizeX, sizeY, 20, 20, TFT_DARKGREY);
        M5.Display.fillCircle(sizeX + 10, sizeY + 10, i, TFT_WHITE);
        
        if (i == currentBrushSize) {
            M5.Display.drawRect(sizeX - 1, sizeY - 1, 22, 22, TFT_CYAN);
        }
        
        if (touch.wasPressed && isPointInRect(touch.x, touch.y, sizeX, sizeY, 20, 20)) {
            currentBrushSize = i;
        }
    }
    
    // Drawing area
    int drawAreaY = startY + 60;
    int drawAreaHeight = 160;
    
    M5.Display.drawRect(10, drawAreaY, M5.Display.width() - 20, drawAreaHeight, TFT_DARKGREY);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Drawing Area", M5.Display.width()/2, drawAreaY - 10);
    
    // Handle drawing
    if (touch.isPressed && touch.y >= drawAreaY && touch.y <= drawAreaY + drawAreaHeight &&
        touch.x >= 10 && touch.x <= M5.Display.width() - 20) {
        
        // Add paint point
        paintPoints[paintIndex].x = touch.x;
        paintPoints[paintIndex].y = touch.y;
        paintPoints[paintIndex].color = currentPaintColor;
        paintPoints[paintIndex].size = currentBrushSize;
        paintPoints[paintIndex].active = true;
        
        paintIndex = (paintIndex + 1) % MAX_PAINT_POINTS;
        
        // Draw line between current and last position if touch was already pressed
        if (touch.lastX != touch.x || touch.lastY != touch.y) {
            if (touch.lastX >= 10 && touch.lastX <= M5.Display.width() - 20 &&
                touch.lastY >= drawAreaY && touch.lastY <= drawAreaY + drawAreaHeight) {
                
                // Draw smooth line by interpolating between points
                int dx = touch.x - touch.lastX;
                int dy = touch.y - touch.lastY;
                int steps = max(abs(dx), abs(dy));
                
                for (int i = 0; i <= steps; i++) {
                    int x = touch.lastX + (dx * i) / steps;
                    int y = touch.lastY + (dy * i) / steps;
                    
                    if (currentBrushSize > 1) {
                        M5.Display.fillCircle(x, y, currentBrushSize, currentPaintColor);
                    } else {
                        M5.Display.drawPixel(x, y, currentPaintColor);
                    }
                }
            }
        }
    }
    
    // Draw all paint points (for persistence)
    for (int i = 0; i < MAX_PAINT_POINTS; i++) {
        if (paintPoints[i].active) {
            if (paintPoints[i].size > 1) {
                M5.Display.fillCircle(paintPoints[i].x, paintPoints[i].y, paintPoints[i].size, paintPoints[i].color);
            } else {
                M5.Display.drawPixel(paintPoints[i].x, paintPoints[i].y, paintPoints[i].color);
            }
        }
    }
    
    // Current status
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Color: " + String(currentPaintColor, HEX), 10, drawAreaY + drawAreaHeight + 10);
    M5.Display.drawString("Size: " + String(currentBrushSize), 100, drawAreaY + drawAreaHeight + 10);
    M5.Display.drawString("Points: " + String(paintIndex), 200, drawAreaY + drawAreaHeight + 10);
}

float calculateDistance(int x1, int y1, int x2, int y2) {
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

String recognizeGesture() {
    if (gesturePointCount < 3) return "Too short";
    
    // Calculate total distance and direction changes
    float totalDistance = 0;
    int directionChanges = 0;
    
    for (int i = 1; i < gesturePointCount; i++) {
        totalDistance += calculateDistance(gesturePoints[i-1].x, gesturePoints[i-1].y,
                                         gesturePoints[i].x, gesturePoints[i].y);
    }
    
    if (totalDistance < 50) return "Too small";
    
    // Check for circular motion
    float centerX = 0, centerY = 0;
    for (int i = 0; i < gesturePointCount; i++) {
        centerX += gesturePoints[i].x;
        centerY += gesturePoints[i].y;
    }
    centerX /= gesturePointCount;
    centerY /= gesturePointCount;
    
    // Check if points form roughly a circle
    float avgRadius = 0;
    for (int i = 0; i < gesturePointCount; i++) {
        avgRadius += calculateDistance(centerX, centerY, gesturePoints[i].x, gesturePoints[i].y);
    }
    avgRadius /= gesturePointCount;
    
    int pointsNearRadius = 0;
    for (int i = 0; i < gesturePointCount; i++) {
        float dist = calculateDistance(centerX, centerY, gesturePoints[i].x, gesturePoints[i].y);
        if (abs(dist - avgRadius) < avgRadius * 0.3) {
            pointsNearRadius++;
        }
    }
    
    if (pointsNearRadius > gesturePointCount * 0.7) {
        return "Circle";
    }
    
    // Check for straight line
    int startX = gesturePoints[0].x;
    int startY = gesturePoints[0].y;
    int endX = gesturePoints[gesturePointCount-1].x;
    int endY = gesturePoints[gesturePointCount-1].y;
    
    int pointsNearLine = 0;
    for (int i = 1; i < gesturePointCount-1; i++) {
        // Calculate distance from point to line
        float A = endY - startY;
        float B = startX - endX;
        float C = endX * startY - startX * endY;
        float distance = abs(A * gesturePoints[i].x + B * gesturePoints[i].y + C) / 
                        sqrt(A*A + B*B);
        
        if (distance < 20) {
            pointsNearLine++;
        }
    }
    
    if (pointsNearLine > gesturePointCount * 0.7) {
        int dx = endX - startX;
        int dy = endY - startY;
        
        if (abs(dx) > abs(dy)) {
            return dx > 0 ? "Right swipe" : "Left swipe";
        } else {
            return dy > 0 ? "Down swipe" : "Up swipe";
        }
    }
    
    return "Unknown";
}

void drawGestureRecognitionDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Gesture Recognition", 10, startY);
    
    // Gesture area
    int gestureAreaY = startY + 40;
    int gestureAreaHeight = 140;
    
    M5.Display.drawRect(10, gestureAreaY, M5.Display.width() - 20, gestureAreaHeight, TFT_DARKGREY);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Draw gestures here", M5.Display.width()/2, gestureAreaY - 10);
    
    // Handle gesture input
    if (touch.wasPressed && touch.y >= gestureAreaY && touch.y <= gestureAreaY + gestureAreaHeight) {
        // Start new gesture
        gesturePointCount = 0;
        detectedGesture = "Drawing...";
    }
    
    if (touch.isPressed && touch.y >= gestureAreaY && touch.y <= gestureAreaY + gestureAreaHeight &&
        touch.x >= 10 && touch.x <= M5.Display.width() - 20) {
        
        // Add point to gesture if it's different enough from last point
        if (gesturePointCount == 0 || 
            calculateDistance(touch.x, touch.y, gesturePoints[gesturePointCount-1].x, 
                            gesturePoints[gesturePointCount-1].y) > 5) {
            
            if (gesturePointCount < MAX_GESTURE_POINTS) {
                gesturePoints[gesturePointCount].x = touch.x;
                gesturePoints[gesturePointCount].y = touch.y;
                gesturePoints[gesturePointCount].time = millis();
                gesturePointCount++;
            }
        }
    }
    
    if (touch.wasReleased && gesturePointCount > 0) {
        // Recognize gesture
        detectedGesture = recognizeGesture();
    }
    
    // Draw gesture trail
    for (int i = 1; i < gesturePointCount; i++) {
        uint8_t alpha = 255 * i / gesturePointCount; // Fade older points
        uint16_t color = M5.Display.color565(alpha, alpha, alpha);
        M5.Display.drawLine(gesturePoints[i-1].x, gesturePoints[i-1].y,
                           gesturePoints[i].x, gesturePoints[i].y, color);
        
        // Draw point
        M5.Display.fillCircle(gesturePoints[i].x, gesturePoints[i].y, 2, TFT_CYAN);
    }
    
    // Current touch point
    if (touch.isPressed && touch.y >= gestureAreaY && touch.y <= gestureAreaY + gestureAreaHeight) {
        M5.Display.fillCircle(touch.x, touch.y, 4, TFT_RED);
    }
    
    // Gesture result
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Detected: " + detectedGesture, 10, gestureAreaY + gestureAreaHeight + 15);
    M5.Display.drawString("Points: " + String(gesturePointCount), 10, gestureAreaY + gestureAreaHeight + 30);
    
    // Recognized gestures list
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Supported Gestures:", 250, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Circle", 250, startY + 35);
    M5.Display.drawString("• Left/Right swipe", 250, startY + 50);
    M5.Display.drawString("• Up/Down swipe", 250, startY + 65);
    M5.Display.drawString("• Tap (short touch)", 250, startY + 80);
    
    // Instructions
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Draw slowly for best results", 10, startY + 20);
}

void drawInteractiveUIDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Interactive UI Components", 10, startY);
    
    // Sliders
    static Slider sliders[3] = {
        {50, startY + 40, 150, 0.5, "Red", false},
        {50, startY + 80, 150, 0.3, "Green", false},
        {50, startY + 120, 150, 0.8, "Blue", false}
    };
    
    // Update sliders
    for (int i = 0; i < 3; i++) {
        Slider &slider = sliders[i];
        
        // Check for touch on slider
        if (touch.isPressed && 
            touch.y >= slider.y - 10 && touch.y <= slider.y + 20 &&
            touch.x >= slider.x && touch.x <= slider.x + slider.width) {
            
            slider.isDragging = true;
            slider.value = (float)(touch.x - slider.x) / slider.width;
            slider.value = constrain(slider.value, 0.0, 1.0);
        } else if (touch.wasReleased) {
            slider.isDragging = false;
        }
        
        // Draw slider track
        M5.Display.drawRect(slider.x, slider.y, slider.width, 10, TFT_DARKGREY);
        M5.Display.fillRect(slider.x + 1, slider.y + 1, slider.width - 2, 8, TFT_BLACK);
        
        // Draw slider fill
        int fillWidth = slider.value * (slider.width - 2);
        uint16_t fillColor = TFT_BLUE;
        if (i == 0) fillColor = TFT_RED;
        else if (i == 1) fillColor = TFT_GREEN;
        
        M5.Display.fillRect(slider.x + 1, slider.y + 1, fillWidth, 8, fillColor);
        
        // Draw slider handle
        int handleX = slider.x + slider.value * slider.width;
        M5.Display.fillCircle(handleX, slider.y + 5, 8, TFT_WHITE);
        M5.Display.drawCircle(handleX, slider.y + 5, 8, TFT_DARKGREY);
        
        if (slider.isDragging) {
            M5.Display.drawCircle(handleX, slider.y + 5, 10, TFT_CYAN);
        }
        
        // Draw label and value
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString(slider.label + ": " + String((int)(slider.value * 100)) + "%", 
                             slider.x, slider.y - 15);
    }
    
    // Color preview using slider values
    uint8_t r = sliders[0].value * 255;
    uint8_t g = sliders[1].value * 255;
    uint8_t b = sliders[2].value * 255;
    uint16_t previewColor = M5.Display.color565(r, g, b);
    
    M5.Display.fillRect(250, startY + 40, 80, 80, previewColor);
    M5.Display.drawRect(250, startY + 40, 80, 80, TFT_WHITE);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Preview", 290, startY + 30);
    
    // Toggle switches
    static ToggleSwitch switches[2] = {
        {50, startY + 170, false, "Option A"},
        {200, startY + 170, true, "Option B"}
    };
    
    for (int i = 0; i < 2; i++) {
        ToggleSwitch &sw = switches[i];
        
        // Check for touch on switch
        if (touch.wasPressed &&
            touch.x >= sw.x && touch.x <= sw.x + 60 &&
            touch.y >= sw.y && touch.y <= sw.y + 30) {
            sw.isOn = !sw.isOn;
        }
        
        // Draw switch background
        uint16_t bgColor = sw.isOn ? TFT_GREEN : TFT_DARKGREY;
        M5.Display.fillRoundRect(sw.x, sw.y, 60, 30, 15, bgColor);
        M5.Display.drawRoundRect(sw.x, sw.y, 60, 30, 15, TFT_WHITE);
        
        // Draw switch handle
        int handleX = sw.isOn ? sw.x + 35 : sw.x + 5;
        M5.Display.fillCircle(handleX + 10, sw.y + 15, 12, TFT_WHITE);
        M5.Display.drawCircle(handleX + 10, sw.y + 15, 12, TFT_BLACK);
        
        // Draw label
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.drawString(sw.label, sw.x, sw.y - 15);
    }
    
    // Progress bar (animated)
    float progress = (sin(animationStep * 0.1) + 1) / 2; // 0 to 1
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Progress: " + String((int)(progress * 100)) + "%", 350, startY + 40);
    
    M5.Display.drawRect(350, startY + 60, 120, 20, TFT_WHITE);
    M5.Display.fillRect(351, startY + 61, progress * 118, 18, TFT_CYAN);
    
    // Instructions
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("UI Components:", 350, startY + 100);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Drag sliders to adjust", 350, startY + 115);
    M5.Display.drawString("• Tap switches to toggle", 350, startY + 130);
    M5.Display.drawString("• Real-time color mixing", 350, startY + 145);
}

void drawTouchEffectsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Touch Effects & Feedback", 10, startY);
    
    // Ripple effect on touch
    if (touch.wasPressed) {
        rippleX = touch.x;
        rippleY = touch.y;
        rippleTime = 0;
        showRipple = true;
    }
    
    if (showRipple) {
        rippleTime += 0.1;
        if (rippleTime > 2) {
            showRipple = false;
        } else {
            // Draw expanding ripples
            for (int i = 0; i < 3; i++) {
                float radius = rippleTime * 50 + i * 15;
                uint8_t alpha = 255 * (1 - rippleTime / 2);
                uint16_t color = M5.Display.color565(alpha, alpha/2, alpha/4);
                M5.Display.drawCircle(rippleX, rippleY, radius, color);
            }
        }
    }
    
    // Touch trail effect
    static int trailPoints[20][2];
    static int trailIndex = 0;
    
    if (touch.isPressed) {
        trailPoints[trailIndex][0] = touch.x;
        trailPoints[trailIndex][1] = touch.y;
        trailIndex = (trailIndex + 1) % 20;
    }
    
    // Draw trail
    for (int i = 0; i < 20; i++) {
        if (trailPoints[i][0] != 0 || trailPoints[i][1] != 0) {
            int age = (trailIndex - i + 20) % 20;
            uint8_t alpha = 255 * (20 - age) / 20;
            uint16_t color = M5.Display.color565(alpha, alpha/3, alpha/6);
            M5.Display.fillCircle(trailPoints[i][0], trailPoints[i][1], 3, color);
        }
    }
    
    // Pressure simulation (based on touch duration)
    if (touch.isPressed) {
        unsigned long touchDuration = millis() - touch.pressTime;
        float pressure = min(1.0f, touchDuration / 1000.0f); // 0 to 1 over 1 second
        
        int pressureRadius = 10 + pressure * 20;
        uint8_t pressureIntensity = 50 + pressure * 200;
        uint16_t pressureColor = M5.Display.color565(pressureIntensity, pressureIntensity/4, 0);
        
        M5.Display.fillCircle(touch.x, touch.y, pressureRadius, pressureColor);
        M5.Display.drawCircle(touch.x, touch.y, pressureRadius, TFT_WHITE);
        
        // Pressure indicator
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("Pressure: " + String((int)(pressure * 100)) + "%", 10, startY + 20);
    }
    
    // Touch particles
    static struct TouchParticle {
        float x, y, vx, vy, life;
        uint16_t color;
        bool active;
    } touchParticles[30];
    
    if (touch.wasPressed) {
        // Spawn particles
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 30; j++) {
                if (!touchParticles[j].active) {
                    touchParticles[j].x = touch.x;
                    touchParticles[j].y = touch.y;
                    touchParticles[j].vx = (random(200) - 100) / 50.0;
                    touchParticles[j].vy = (random(200) - 100) / 50.0;
                    touchParticles[j].life = 30 + random(30);
                    touchParticles[j].color = M5.Display.color565(random(255), random(255), random(255));
                    touchParticles[j].active = true;
                    break;
                }
            }
        }
    }
    
    // Update and draw particles
    for (int i = 0; i < 30; i++) {
        if (touchParticles[i].active) {
            touchParticles[i].x += touchParticles[i].vx;
            touchParticles[i].y += touchParticles[i].vy;
            touchParticles[i].life -= 1;
            
            if (touchParticles[i].life <= 0) {
                touchParticles[i].active = false;
            } else {
                float alpha = touchParticles[i].life / 60.0;
                uint8_t r = ((touchParticles[i].color >> 11) & 0x1F) * 8 * alpha;
                uint8_t g = ((touchParticles[i].color >> 5) & 0x3F) * 4 * alpha;
                uint8_t b = (touchParticles[i].color & 0x1F) * 8 * alpha;
                uint16_t fadedColor = M5.Display.color565(r, g, b);
                
                M5.Display.drawPixel(touchParticles[i].x, touchParticles[i].y, fadedColor);
            }
        }
    }
    
    // Touch information display
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Touch Effects:", 10, startY + 40);
    M5.Display.drawString("• Ripple on press", 10, startY + 55);
    M5.Display.drawString("• Trail following", 10, startY + 70);
    M5.Display.drawString("• Pressure simulation", 10, startY + 85);
    M5.Display.drawString("• Particle burst", 10, startY + 100);
    
    if (touch.isPressed) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("Active Touch:", 200, startY + 40);
        M5.Display.drawString("X: " + String(touch.x), 200, startY + 55);
        M5.Display.drawString("Y: " + String(touch.y), 200, startY + 70);
        M5.Display.drawString("Duration: " + String(millis() - touch.pressTime) + "ms", 200, startY + 85);
    }
    
    // Effect intensity controls
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Touch anywhere for effects!", 10, startY + 200);
}

void loop() {
    // Update touch state
    updateTouch();
    
    // Handle button presses
    if (M5.BtnA.wasPressed()) {
        currentDemo = (TouchDemo)((currentDemo - 1 + TOUCH_DEMO_COUNT) % TOUCH_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        // Clear/reset current demo
        if (currentDemo == DEMO_DRAWING_PAINT) {
            initPaintSystem();
        } else if (currentDemo == DEMO_DRAG_DROP) {
            initDragObjects();
        } else if (currentDemo == DEMO_GESTURE_RECOGNITION) {
            gesturePointCount = 0;
            detectedGesture = "";
        }
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (TouchDemo)((currentDemo + 1) % TOUCH_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw current demo (for animated elements)
        drawCurrentTouchDemo();
        
        lastUpdate = millis();
    }
}