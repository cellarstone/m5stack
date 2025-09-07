#include <M5Unified.h>

// Color definitions
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_BUTTON_BG TFT_DARKGREY
#define COLOR_BUTTON_ACTIVE TFT_GREEN
#define COLOR_CIRCLE TFT_BLUE
#define COLOR_RECT TFT_RED
#define COLOR_TRIANGLE TFT_YELLOW

// Button structure
struct Button {
    int x, y, w, h;
    const char* label;
    uint16_t color;
    bool pressed;
};

// Global variables
Button buttons[3];
int selectedShape = 0;
float animationAngle = 0;
unsigned long lastTime = 0;

// Function declarations
void drawButtons();
void drawShape(int shape, float angle);
void handleTouch();

void setup() {
    // Initialize M5Stack Tab5
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(3);
    M5.Display.fillScreen(COLOR_BACKGROUND);
    
    // Set text properties
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(COLOR_TEXT);
    
    // Display welcome message
    M5.Display.setCursor(10, 10);
    M5.Display.println("M5Stack Tab5 Demo");
    M5.Display.setTextSize(1);
    M5.Display.println("Touch the buttons to draw shapes!");
    
    // Initialize buttons
    int buttonWidth = 100;
    int buttonHeight = 40;
    int startX = 50;
    int startY = M5.Display.height() - 60;
    
    buttons[0] = {startX, startY, buttonWidth, buttonHeight, "Circle", COLOR_BUTTON_BG, false};
    buttons[1] = {startX + buttonWidth + 20, startY, buttonWidth, buttonHeight, "Rectangle", COLOR_BUTTON_BG, false};
    buttons[2] = {startX + (buttonWidth + 20) * 2, startY, buttonWidth, buttonHeight, "Triangle", COLOR_BUTTON_BG, false};
    
    drawButtons();
    lastTime = millis();
}

void drawButtons() {
    for (int i = 0; i < 3; i++) {
        uint16_t btnColor = (buttons[i].pressed) ? COLOR_BUTTON_ACTIVE : buttons[i].color;
        M5.Display.fillRoundRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 5, btnColor);
        M5.Display.drawRoundRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, 5, COLOR_TEXT);
        
        // Center text in button
        M5.Display.setTextSize(1);
        int textWidth = strlen(buttons[i].label) * 6;
        int textX = buttons[i].x + (buttons[i].w - textWidth) / 2;
        int textY = buttons[i].y + (buttons[i].h - 8) / 2;
        M5.Display.setCursor(textX, textY);
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.print(buttons[i].label);
    }
}

void drawShape(int shape, float angle) {
    // Clear drawing area (keep header and buttons)
    M5.Display.fillRect(0, 60, M5.Display.width(), M5.Display.height() - 130, COLOR_BACKGROUND);
    
    int centerX = M5.Display.width() / 2;
    int centerY = M5.Display.height() / 2 - 20;
    
    switch (shape) {
        case 0: // Circle
            {
                int radius = 50 + sin(angle) * 20;
                M5.Display.fillCircle(centerX, centerY, radius, COLOR_CIRCLE);
                M5.Display.drawCircle(centerX, centerY, radius + 5, COLOR_TEXT);
                
                // Draw rotating dots around circle
                for (int i = 0; i < 8; i++) {
                    float dotAngle = angle + (i * PI / 4);
                    int dotX = centerX + cos(dotAngle) * (radius + 15);
                    int dotY = centerY + sin(dotAngle) * (radius + 15);
                    M5.Display.fillCircle(dotX, dotY, 3, COLOR_TEXT);
                }
            }
            break;
            
        case 1: // Rectangle
            {
                int width = 100 + sin(angle) * 30;
                int height = 60 + cos(angle) * 20;
                M5.Display.fillRect(centerX - width/2, centerY - height/2, width, height, COLOR_RECT);
                M5.Display.drawRect(centerX - width/2 - 5, centerY - height/2 - 5, width + 10, height + 10, COLOR_TEXT);
                
                // Draw corner decorations
                int corners[4][2] = {
                    {centerX - width/2, centerY - height/2},
                    {centerX + width/2, centerY - height/2},
                    {centerX + width/2, centerY + height/2},
                    {centerX - width/2, centerY + height/2}
                };
                for (int i = 0; i < 4; i++) {
                    M5.Display.fillCircle(corners[i][0], corners[i][1], 5, COLOR_TEXT);
                }
            }
            break;
            
        case 2: // Triangle
            {
                int size = 60 + sin(angle) * 20;
                int x1 = centerX;
                int y1 = centerY - size;
                int x2 = centerX - size * 0.866;
                int y2 = centerY + size * 0.5;
                int x3 = centerX + size * 0.866;
                int y3 = centerY + size * 0.5;
                
                // Rotate triangle
                float cosA = cos(angle);
                float sinA = sin(angle);
                
                int rx1 = centerX + (x1 - centerX) * cosA - (y1 - centerY) * sinA;
                int ry1 = centerY + (x1 - centerX) * sinA + (y1 - centerY) * cosA;
                int rx2 = centerX + (x2 - centerX) * cosA - (y2 - centerY) * sinA;
                int ry2 = centerY + (x2 - centerX) * sinA + (y2 - centerY) * cosA;
                int rx3 = centerX + (x3 - centerX) * cosA - (y3 - centerY) * sinA;
                int ry3 = centerY + (x3 - centerX) * sinA + (y3 - centerY) * cosA;
                
                M5.Display.fillTriangle(rx1, ry1, rx2, ry2, rx3, ry3, COLOR_TRIANGLE);
                M5.Display.drawTriangle(rx1, ry1, rx2, ry2, rx3, ry3, COLOR_TEXT);
            }
            break;
    }
    
    // Display info text
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setCursor(10, 80);
    M5.Display.printf("Animation: %.1f", angle * 180 / PI);
    
    // Display touch coordinates if touched
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        M5.Display.setCursor(10, 95);
        M5.Display.printf("Touch: (%d, %d)", touch.x, touch.y);
    }
}

void handleTouch() {
    auto touch = M5.Touch.getDetail();
    
    if (touch.wasPressed()) {
        // Check button presses
        for (int i = 0; i < 3; i++) {
            if (touch.x >= buttons[i].x && touch.x <= buttons[i].x + buttons[i].w &&
                touch.y >= buttons[i].y && touch.y <= buttons[i].y + buttons[i].h) {
                
                // Reset all buttons
                for (int j = 0; j < 3; j++) {
                    buttons[j].pressed = false;
                }
                
                // Set selected button
                buttons[i].pressed = true;
                selectedShape = i;
                drawButtons();
                
                // Visual feedback
                M5.Speaker.tone(1000 + i * 200, 50);
            }
        }
    }
}

void loop() {
    M5.update();
    
    // Handle touch input
    handleTouch();
    
    // Update animation
    unsigned long currentTime = millis();
    if (currentTime - lastTime > 50) { // Update every 50ms
        animationAngle += 0.05;
        if (animationAngle > 2 * PI) {
            animationAngle = 0;
        }
        
        // Draw animated shape
        drawShape(selectedShape, animationAngle);
        
        lastTime = currentTime;
    }
    
    // Display system info
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setCursor(10, 110);
    M5.Display.printf("FPS: %d", (int)(1000.0 / (currentTime - lastTime + 1)));
    
    delay(10);
}