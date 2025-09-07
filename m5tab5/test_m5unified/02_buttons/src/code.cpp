/*
 * M5Unified Tutorial - 02_buttons (Virtual Touch Buttons)
 * For M5Stack Tab5
 * 
 * Since M5Stack Tab5 has no physical buttons, this demo creates
 * virtual touch buttons to demonstrate button-like interactions:
 * - Virtual button creation and detection
 * - Different button states (pressed, released, held)
 * - Long press detection
 * - Multi-button handling
 * - Visual and audio feedback
 * 
 * Key concepts:
 * - Touch zones as virtual buttons
 * - State tracking for touch interactions
 * - Visual feedback for button states
 * - Simulating button behavior with touch
 */

#include <M5Unified.h>

// Virtual button structure
struct VirtualButton {
    int x, y, w, h;
    const char* label;
    uint16_t color;
    uint16_t pressedColor;
    bool isPressed;
    bool wasPressed;
    bool wasReleased;
    bool isHeld;
    unsigned long pressStartTime;
    unsigned long holdThreshold;
    int pressCount;
};

// Create three virtual buttons
VirtualButton buttons[3];

// Touch tracking
int lastTouchX = -1;
int lastTouchY = -1;
bool wasTouching = false;

// Forward declarations
void drawButtons();
void drawStatus();

void initializeButtons() {
    int buttonWidth = (M5.Display.width() - 40) / 3;
    int buttonHeight = 60;
    int buttonY = M5.Display.height() - 100;
    
    // Button A - Blue
    buttons[0] = {
        10, buttonY, buttonWidth, buttonHeight,
        "Button A", TFT_DARKGREY, TFT_BLUE,
        false, false, false, false, 0, 1000, 0
    };
    
    // Button B - Green  
    buttons[1] = {
        20 + buttonWidth, buttonY, buttonWidth, buttonHeight,
        "Button B", TFT_DARKGREY, TFT_GREEN,
        false, false, false, false, 0, 1500, 0
    };
    
    // Button C - Red
    buttons[2] = {
        30 + buttonWidth * 2, buttonY, buttonWidth, buttonHeight,
        "Button C", TFT_DARKGREY, TFT_RED,
        false, false, false, false, 0, 2000, 0
    };
}

void drawInterface() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Virtual Button Demo", M5.Display.width()/2, 10);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("M5Stack Tab5 - Touch Buttons", M5.Display.width()/2, 35);
    
    // Instructions
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Instructions:", 10, 55);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("- Touch virtual buttons below", 10, 70);
    M5.Display.drawString("- Hold for long press (1s, 1.5s, 2s)", 10, 85);
    M5.Display.drawString("- Each button tracks press count", 10, 100);
    M5.Display.drawString("- See button states update in real-time", 10, 115);
    
    // Draw buttons
    drawButtons();
    
    // Status area
    drawStatus();
}

void drawButtons() {
    static bool firstDraw = true;
    static bool lastPressed[3] = {false, false, false};
    static int lastCount[3] = {-1, -1, -1};
    
    for (int i = 0; i < 3; i++) {
        // Only redraw if state changed or first draw
        if (firstDraw || lastPressed[i] != buttons[i].isPressed || lastCount[i] != buttons[i].pressCount) {
            uint16_t color = buttons[i].isPressed ? buttons[i].pressedColor : buttons[i].color;
            
            // Draw button
            M5.Display.fillRoundRect(buttons[i].x, buttons[i].y, 
                                     buttons[i].w, buttons[i].h, 10, color);
            
            // Draw border
            M5.Display.drawRoundRect(buttons[i].x, buttons[i].y, 
                                    buttons[i].w, buttons[i].h, 10, TFT_WHITE);
            
            // Draw label
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setTextSize(2);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString(buttons[i].label, 
                                 buttons[i].x + buttons[i].w/2, 
                                 buttons[i].y + buttons[i].h/2 - 10);
            
            // Show press count inside button
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(TFT_YELLOW);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString("Count: " + String(buttons[i].pressCount), 
                                 buttons[i].x + buttons[i].w/2, 
                                 buttons[i].y + buttons[i].h/2 + 10);
            
            // Update tracking variables
            lastPressed[i] = buttons[i].isPressed;
            lastCount[i] = buttons[i].pressCount;
        }
    }
    firstDraw = false;
}

void drawStatus() {
    static bool firstDraw = true;
    static bool lastPressed[3] = {false, false, false};
    static bool lastHeld[3] = {false, false, false};
    static unsigned long lastUpdateTime = 0;
    
    int statusY = 140;
    
    // Only clear and redraw note on first draw
    if (firstDraw) {
        M5.Display.fillRect(0, statusY, M5.Display.width(), 100, TFT_BLACK);
        
        // Note about single touch limitation (draw once)
        M5.Display.setTextColor(TFT_CYAN);
        M5.Display.setTextDatum(TC_DATUM);
        M5.Display.setTextSize(1);
        M5.Display.drawString("Note: Single touch only - one button at a time", 
                             M5.Display.width()/2, statusY + 60);
    }
    
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    
    for (int i = 0; i < 3; i++) {
        int x = 10 + i * (M5.Display.width() / 3);
        bool needsUpdate = firstDraw || 
                          lastPressed[i] != buttons[i].isPressed || 
                          lastHeld[i] != buttons[i].isHeld ||
                          (buttons[i].isPressed && millis() - lastUpdateTime > 100);
        
        if (needsUpdate) {
            // Clear this button's status area
            M5.Display.fillRect(x, statusY, M5.Display.width()/3 - 10, 55, TFT_BLACK);
            
            M5.Display.setTextColor(buttons[i].isPressed ? TFT_GREEN : TFT_WHITE);
            M5.Display.drawString(String(buttons[i].label) + ":", x, statusY);
            
            String state = buttons[i].isPressed ? "PRESSED" : "RELEASED";
            if (buttons[i].isHeld) state = "HELD";
            M5.Display.drawString("State: " + state, x, statusY + 15);
            
            if (buttons[i].isPressed) {
                unsigned long holdTime = millis() - buttons[i].pressStartTime;
                M5.Display.drawString("Hold: " + String(holdTime) + "ms", x, statusY + 30);
                
                // Draw progress bar for hold time
                int barWidth = 80;
                int progress = min(barWidth, (int)((holdTime * barWidth) / buttons[i].holdThreshold));
                M5.Display.drawRect(x, statusY + 45, barWidth, 5, TFT_DARKGREY);
                M5.Display.fillRect(x, statusY + 45, progress, 5, 
                                   buttons[i].isHeld ? TFT_GREEN : TFT_YELLOW);
            }
            
            lastPressed[i] = buttons[i].isPressed;
            lastHeld[i] = buttons[i].isHeld;
        }
    }
    
    if (millis() - lastUpdateTime > 100) {
        lastUpdateTime = millis();
    }
    firstDraw = false;
}

bool isTouchInButton(int touchX, int touchY, VirtualButton &btn) {
    return touchX >= btn.x && touchX < btn.x + btn.w &&
           touchY >= btn.y && touchY < btn.y + btn.h;
}

void updateButtonStates() {
    auto touch = M5.Touch.getDetail();
    bool isTouching = touch.isPressed();
    
    // Reset single-frame states
    for (int i = 0; i < 3; i++) {
        buttons[i].wasPressed = false;
        buttons[i].wasReleased = false;
    }
    
    if (isTouching) {
        lastTouchX = touch.x;
        lastTouchY = touch.y;
        
        for (int i = 0; i < 3; i++) {
            bool inButton = isTouchInButton(touch.x, touch.y, buttons[i]);
            
            if (inButton && !buttons[i].isPressed) {
                // Button just pressed
                buttons[i].isPressed = true;
                buttons[i].wasPressed = true;
                buttons[i].pressStartTime = millis();
                buttons[i].pressCount++;
                buttons[i].isHeld = false;
                
                // Audio feedback
                if (M5.Speaker.isEnabled()) {
                    M5.Speaker.tone(500 + i * 200, 50);
                }
            } else if (!inButton && buttons[i].isPressed) {
                // Finger moved out of button
                buttons[i].isPressed = false;
                buttons[i].wasReleased = true;
                buttons[i].isHeld = false;
            }
            
            // Check for long press
            if (buttons[i].isPressed && !buttons[i].isHeld) {
                unsigned long holdTime = millis() - buttons[i].pressStartTime;
                if (holdTime >= buttons[i].holdThreshold) {
                    buttons[i].isHeld = true;
                    
                    // Long press feedback
                    if (M5.Speaker.isEnabled()) {
                        M5.Speaker.tone(200 + i * 100, 200);
                    }
                    
                    // Visual feedback
                    M5.Display.fillRect(0, 0, M5.Display.width(), 30, TFT_YELLOW);
                    M5.Display.setTextColor(TFT_BLACK);
                    M5.Display.setTextDatum(MC_DATUM);
                    M5.Display.setTextSize(1);
                    M5.Display.drawString(String(buttons[i].label) + " LONG PRESS!", 
                                        M5.Display.width()/2, 15);
                }
            }
        }
    } else {
        // Touch released
        for (int i = 0; i < 3; i++) {
            if (buttons[i].isPressed) {
                buttons[i].isPressed = false;
                buttons[i].wasReleased = true;
                buttons[i].isHeld = false;
            }
        }
        
        // Clear long press message
        if (wasTouching) {
            M5.Display.fillRect(0, 0, M5.Display.width(), 30, TFT_BLACK);
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setTextSize(2);
            M5.Display.setTextDatum(TC_DATUM);
            M5.Display.drawString("Virtual Button Demo", M5.Display.width()/2, 10);
        }
    }
    
    wasTouching = isTouching;
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.setBrightness(128);
    
    // Check for touch capability
    if (!M5.Touch.isEnabled()) {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Touch Not Available", M5.Display.width()/2, M5.Display.height()/2);
        while(1) delay(100);
    }
    
    // Initialize virtual buttons
    initializeButtons();
    
    // Draw interface
    drawInterface();
    
    // Play startup sound
    if (M5.Speaker.isEnabled()) {
        M5.Speaker.tone(1000, 100);
    }
}

void loop() {
    M5.update();
    
    // Update button states
    updateButtonStates();
    
    // Check if any button state changed
    bool stateChanged = false;
    for (int i = 0; i < 3; i++) {
        if (buttons[i].wasPressed || buttons[i].wasReleased) {
            stateChanged = true;
            break;
        }
    }
    
    // Only redraw when something changes
    if (stateChanged) {
        drawButtons();
    }
    
    // Update status less frequently and only when needed
    static unsigned long lastStatusUpdate = 0;
    if (stateChanged || (millis() - lastStatusUpdate > 100)) {
        drawStatus();
        lastStatusUpdate = millis();
    }
    
    delay(10);
}