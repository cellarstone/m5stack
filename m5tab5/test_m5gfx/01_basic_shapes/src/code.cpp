/*
 * M5GFX Tutorial - 01_basic_shapes
 * 
 * This demo demonstrates basic drawing primitives in M5GFX:
 * - Points and pixels
 * - Lines (horizontal, vertical, diagonal)
 * - Rectangles (filled and outlined)
 * - Circles and ellipses
 * - Triangles and polygons
 * - Rounded rectangles
 * - Bezier curves and arcs
 * 
 * Key concepts:
 * - M5.Display drawing functions
 * - Coordinate system and positioning
 * - Color specification and management
 * - Shape parameters and options
 * - Performance considerations
 */

#include <M5Unified.h>

// Sprite for double buffering to eliminate flicker
LGFX_Sprite demoSprite(&M5.Display);
bool spriteInitialized = false;

// Demo modes for different shape categories
enum ShapeDemo {
    DEMO_POINTS_LINES,
    DEMO_RECTANGLES,
    DEMO_CIRCLES,
    DEMO_TRIANGLES,
    DEMO_COMPLEX_SHAPES,
    DEMO_INTERACTIVE,
    SHAPE_DEMO_COUNT
};

ShapeDemo currentDemo = DEMO_POINTS_LINES;
const char* shapeDemoNames[] = {
    "Points & Lines",
    "Rectangles",
    "Circles & Ellipses",
    "Triangles",
    "Complex Shapes",
    "Interactive Drawing"
};

// Animation variables
unsigned long lastUpdate = 0;
float animationAngle = 0;
int animationStep = 0;

// Interactive drawing variables
int lastTouchX = -1, lastTouchY = -1;
uint16_t drawColor = TFT_WHITE;
int brushSize = 7;  // Larger default brush for 1280x720 display

// Touch button definitions
struct TouchButton {
    int x, y, w, h;
    String label;
    uint16_t color;
    bool pressed;
};

// Define touch buttons - at bottom of 1280x720 screen
TouchButton btnPrev = {340, 640, 200, 60, "< PREV", TFT_DARKGREEN, false};
TouchButton btnAnimate = {540, 640, 200, 60, "ANIMATE", TFT_BLUE, false};
TouchButton btnNext = {740, 640, 200, 60, "NEXT >", TFT_DARKGREEN, false};

// Forward declarations
void displayWelcome();
void displayCurrentDemo();
void drawCurrentShapeDemo();
void drawPointsLinesDemo();
void drawRectanglesDemo();
void drawCirclesDemo();
void drawTrianglesDemo();
void drawComplexShapesDemo();
void drawInteractiveDemo();
void drawTouchButton(TouchButton& btn);
bool checkTouchButton(TouchButton& btn, int touchX, int touchY);
void drawAllButtons();
void drawCurrentDemoToSprite();
void drawPointsLinesToSprite();
void drawRectanglesToSprite();
void drawCirclesToSprite();
void drawTrianglesToSprite();
void drawComplexShapesToSprite();

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);  // Rotate display by 180 degrees
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize sprite for the demo area (exclude buttons area)
    demoSprite.setColorDepth(16);
    if (demoSprite.createSprite(M5.Display.width(), M5.Display.height() - 120)) {  // Leave space for buttons
        spriteInitialized = true;
    }
    
    // Debug: Show screen dimensions
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Screen: " + String(M5.Display.width()) + "x" + String(M5.Display.height()), 10, 10);
    delay(1000);
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(6);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Shapes", M5.Display.width()/2, 150);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Basic Drawing", M5.Display.width()/2, 280);
    M5.Display.drawString("Primitives", M5.Display.width()/2, 350);
    
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Learn fundamental graphics operations", M5.Display.width()/2, 450);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Points, Lines, Rectangles", M5.Display.width()/2, 510);
    M5.Display.drawString("• Circles, Triangles, Polygons", M5.Display.width()/2, 560);
    M5.Display.drawString("• Interactive Drawing Tools", M5.Display.width()/2, 610);
}

// Flag to track if we need full redraw
bool needsFullRedraw = true;

void displayCurrentDemo() {
    // For animated demos, use sprite to eliminate flicker
    if (spriteInitialized && (currentDemo == DEMO_POINTS_LINES || 
                              currentDemo == DEMO_RECTANGLES || 
                              currentDemo == DEMO_CIRCLES || 
                              currentDemo == DEMO_TRIANGLES || 
                              currentDemo == DEMO_COMPLEX_SHAPES)) {
        // Clear sprite
        demoSprite.fillScreen(TFT_BLACK);
        
        // Draw demo name to sprite
        demoSprite.setTextColor(TFT_CYAN);
        demoSprite.setTextSize(4);
        demoSprite.setTextDatum(TC_DATUM);
        demoSprite.drawString(shapeDemoNames[currentDemo], demoSprite.width()/2, 30);
        
        // Draw demo content to sprite by calling draw functions
        // This is a workaround - we'll draw to sprite coordinates
        drawCurrentDemoToSprite();
        
        // Push sprite to display in one go (eliminates flicker)
        demoSprite.pushSprite(0, 0);
        
        // Draw buttons directly to display (they don't animate)
        if (needsFullRedraw) {
            drawAllButtons();
            needsFullRedraw = false;
        }
    } else {
        // For non-animated demos, draw directly
        M5.Display.startWrite();
        
        if (needsFullRedraw) {
            M5.Display.fillScreen(TFT_BLACK);
            M5.Display.setTextColor(TFT_CYAN);
            M5.Display.setTextSize(4);
            M5.Display.setTextDatum(TC_DATUM);
            M5.Display.drawString(shapeDemoNames[currentDemo], M5.Display.width()/2, 30);
            drawAllButtons();
            needsFullRedraw = false;
        }
        
        M5.Display.fillRect(0, 80, M5.Display.width(), M5.Display.height() - 200, TFT_BLACK);
        drawCurrentShapeDemo();
        
        M5.Display.endWrite();
    }
}

void drawCurrentDemoToSprite() {
    // Draw to sprite instead of display
    switch(currentDemo) {
        case DEMO_POINTS_LINES:
            drawPointsLinesToSprite();
            break;
        case DEMO_RECTANGLES:
            drawRectanglesToSprite();
            break;
        case DEMO_CIRCLES:
            drawCirclesToSprite();
            break;
        case DEMO_TRIANGLES:
            drawTrianglesToSprite();
            break;
        case DEMO_COMPLEX_SHAPES:
            drawComplexShapesToSprite();
            break;
        default:
            break;
    }
}

void drawPointsLinesToSprite() {
    int screenW = demoSprite.width();
    int screenH = demoSprite.height();
    
    // Draw a grid of colorful points
    for (int x = 30; x < screenW - 30; x += 60) {
        for (int y = 100; y < screenH/3; y += 50) {
            uint16_t color = demoSprite.color565((x*2) % 255, (y*2) % 255, ((x+y)*2) % 255);
            demoSprite.fillCircle(x, y, 15, color);
        }
    }
    
    // Draw thick horizontal lines
    int lineY = screenH/3 + 20;
    for (int i = 0; i < 8; i++) {
        uint16_t color = demoSprite.color565(255 - i*30, i*30, 128);
        demoSprite.fillRect(30, lineY + i*35, screenW - 60, 12, color);
    }
    
    // Animated star burst
    int centerX = screenW / 2;
    int centerY = screenH / 2;
    for (int i = 0; i < 20; i++) {
        float angle = (animationAngle + i * 18) * PI / 180.0;
        int radius = min(screenW, screenH) / 3;
        int endX = centerX + radius * cos(angle);
        int endY = centerY + radius * sin(angle);
        uint16_t intensity = 255 - (i * 12);
        uint16_t color = demoSprite.color565(intensity, intensity/2, intensity);
        for (int j = -2; j <= 2; j++) {
            demoSprite.drawLine(centerX + j, centerY, endX + j, endY, color);
        }
    }
}

void drawRectanglesToSprite() {
    int screenW = demoSprite.width();
    int screenH = demoSprite.height();
    int rectW = (screenW - 100) / 2;
    int rectH = (screenH - 200) / 2;
    
    // Outlined rectangle
    for (int i = 0; i < 5; i++) {
        demoSprite.drawRect(50 + i*2, 100 + i*2, rectW - i*4, rectH - i*4, TFT_WHITE);
    }
    
    // Filled rectangle
    demoSprite.fillRect(screenW/2 + 50, 100, rectW - 100, rectH, TFT_GREEN);
    
    // Rounded rectangle
    for (int i = 0; i < 5; i++) {
        demoSprite.drawRoundRect(50 + i*2, screenH/2 + i*2, rectW - i*4, rectH - i*4, 40, TFT_YELLOW);
    }
    
    // Filled rounded rectangle
    demoSprite.fillRoundRect(screenW/2 + 50, screenH/2, rectW - 100, rectH, 40, TFT_MAGENTA);
    
    // Animated moving rectangle
    int animX = 50 + (animationStep * 8) % (screenW - 200);
    demoSprite.fillRect(animX, screenH - 120, 150, 80, TFT_CYAN);
}

void drawCirclesToSprite() {
    int screenW = demoSprite.width();
    int screenH = demoSprite.height();
    
    // Outlined circles
    for (int i = 0; i < 5; i++) {
        demoSprite.drawCircle(200, 250, 120 - i*2, TFT_WHITE);
    }
    
    // Filled circle
    demoSprite.fillCircle(500, 250, 120, TFT_GREEN);
    
    // Ellipses
    for (int i = 0; i < 4; i++) {
        demoSprite.drawEllipse(200, 480, 140 - i*2, 80 - i, TFT_YELLOW);
    }
    
    // Filled ellipse
    demoSprite.fillEllipse(500, 480, 140, 80, TFT_MAGENTA);
    
    // Animated pulsing circle
    int radius = 80 + (sin(animationAngle * PI / 180.0) * 50);
    for (int i = 0; i < 5; i++) {
        demoSprite.drawCircle(screenW/2, screenH/2, radius - i*2, TFT_CYAN);
    }
}

void drawTrianglesToSprite() {
    int screenW = demoSprite.width();
    int screenH = demoSprite.height();
    
    // Outlined triangles
    for (int i = 0; i < 5; i++) {
        demoSprite.drawTriangle(200, 120 + i*2, 80 + i*2, 320 - i*2, 320 - i*2, 320 - i*2, TFT_WHITE);
    }
    
    // Filled triangle
    demoSprite.fillTriangle(500, 120, 380, 320, 620, 320, TFT_GREEN);
    
    // Animated rotating triangle
    float angle = animationAngle * PI / 180.0;
    int centerX = screenW/2, centerY = screenH/2;
    int radius = 120;
    int x1 = centerX + radius * cos(angle);
    int y1 = centerY + radius * sin(angle);
    int x2 = centerX + radius * cos(angle + 2*PI/3);
    int y2 = centerY + radius * sin(angle + 2*PI/3);
    int x3 = centerX + radius * cos(angle + 4*PI/3);
    int y3 = centerY + radius * sin(angle + 4*PI/3);
    demoSprite.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
}

void drawComplexShapesToSprite() {
    int screenW = demoSprite.width();
    int screenH = demoSprite.height();
    int startY = 120;
    
    demoSprite.setTextColor(TFT_CYAN);
    demoSprite.setTextSize(3);
    demoSprite.setTextDatum(TL_DATUM);
    demoSprite.drawString("Complex Shapes Demo", 50, startY);
    
    // Star
    int centerX = 250, centerY = startY + 200;
    for (int i = 0; i < 5; i++) {
        float angle1 = (i * 72 - 90) * PI / 180.0;
        float angle2 = ((i + 2) * 72 - 90) * PI / 180.0;
        for (int j = 0; j < 3; j++) {
            int x1 = centerX + (80 - j*2) * cos(angle1);
            int y1 = centerY + (80 - j*2) * sin(angle1);
            int x2 = centerX + (80 - j*2) * cos(angle2);
            int y2 = centerY + (80 - j*2) * sin(angle2);
            demoSprite.drawLine(x1, y1, x2, y2, TFT_YELLOW);
        }
    }
    
    // Animated spiral
    centerX = screenW/2;
    centerY = startY + 380;
    for (int i = 0; i < 60; i++) {
        float angle = (animationAngle + i * 6) * PI / 180.0;
        float radius = i * 2;
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);
        uint16_t color = demoSprite.color565(255 - i*4, i*4, 128);
        demoSprite.fillCircle(x, y, 5, color);
    }
}

void drawCurrentShapeDemo() {
    switch(currentDemo) {
        case DEMO_POINTS_LINES:
            drawPointsLinesDemo();
            break;
        case DEMO_RECTANGLES:
            drawRectanglesDemo();
            break;
        case DEMO_CIRCLES:
            drawCirclesDemo();
            break;
        case DEMO_TRIANGLES:
            drawTrianglesDemo();
            break;
        case DEMO_COMPLEX_SHAPES:
            drawComplexShapesDemo();
            break;
        case DEMO_INTERACTIVE:
            drawInteractiveDemo();
            break;
    }
}

void drawPointsLinesDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    
    // Draw a grid of colorful points across the full screen
    for (int x = 30; x < screenW - 30; x += 60) {
        for (int y = 100; y < screenH/3; y += 50) {
            uint16_t color = M5.Display.color565((x*2) % 255, (y*2) % 255, ((x+y)*2) % 255);
            M5.Display.fillCircle(x, y, 15, color);
        }
    }
    
    // Draw thick horizontal lines with gradient
    int lineY = screenH/3 + 20;
    for (int i = 0; i < 8; i++) {
        uint16_t color = M5.Display.color565(255 - i*30, i*30, 128);
        M5.Display.fillRect(30, lineY + i*35, screenW - 60, 12, color);
    }
    
    // Draw diagonal crossing lines forming a pattern
    int patternY = screenH * 2/3;
    int patternW = (screenW - 60) / 8;
    for (int i = 0; i < 8; i++) {
        uint16_t color = M5.Display.color565(255, i*30, 255 - i*30);
        int x = 30 + i * patternW;
        for (int j = 0; j < 5; j++) {
            M5.Display.drawLine(x + j*2, patternY, x + patternW + j*2, screenH - 100, color);
            M5.Display.drawLine(x + patternW + j*2, patternY, x + j*2, screenH - 100, color);
        }
    }
    
    // Add animated star burst in center
    int centerX = screenW / 2;
    int centerY = screenH / 2;
    for (int i = 0; i < 20; i++) {
        float angle = (animationAngle + i * 18) * PI / 180.0;
        int radius = min(screenW, screenH) / 3;
        int endX = centerX + radius * cos(angle);
        int endY = centerY + radius * sin(angle);
        uint16_t intensity = 255 - (i * 12);
        uint16_t color = M5.Display.color565(intensity, intensity/2, intensity);
        for (int j = -2; j <= 2; j++) {
            M5.Display.drawLine(centerX + j, centerY, endX + j, endY, color);
        }
    }
}

void drawRectanglesDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    int rectW = (screenW - 100) / 2;
    int rectH = (screenH - 200) / 2;
    
    // Large outlined rectangle (top-left)
    for (int i = 0; i < 5; i++) {
        M5.Display.drawRect(50 + i*2, 100 + i*2, rectW - i*4, rectH - i*4, TFT_WHITE);
    }
    
    // Large filled rectangle (top-right)
    M5.Display.fillRect(screenW/2 + 50, 100, rectW - 100, rectH, TFT_GREEN);
    
    // Large rounded rectangle (bottom-left)
    for (int i = 0; i < 5; i++) {
        M5.Display.drawRoundRect(50 + i*2, screenH/2 + i*2, rectW - i*4, rectH - i*4, 40, TFT_YELLOW);
    }
    
    // Large filled rounded rectangle (bottom-right)
    M5.Display.fillRoundRect(screenW/2 + 50, screenH/2, rectW - 100, rectH, 40, TFT_MAGENTA);
    
    // Animated moving rectangles at bottom
    int animX = 50 + (animationStep * 8) % (screenW - 200);
    M5.Display.fillRect(animX, screenH - 120, 150, 80, TFT_CYAN);
    for (int i = 0; i < 3; i++) {
        M5.Display.drawRect(animX + 20 + i*2, screenH - 110 + i*2, 110 - i*4, 60 - i*4, TFT_RED);
    }
    
    // Nested rectangles in center
    int centerX = screenW / 2;
    int centerY = screenH / 2;
    for (int i = 0; i < 8; i++) {
        uint16_t color = M5.Display.color565(255 - i*30, i*30, 128);
        int size = 40 + i*30;
        M5.Display.drawRect(centerX - size, centerY - size/2, size*2, size, color);
    }
}

void drawCirclesDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    
    // Large outlined circles
    for (int i = 0; i < 5; i++) {
        M5.Display.drawCircle(200, 250, 120 - i*2, TFT_WHITE);
    }
    
    // Large filled circle
    M5.Display.fillCircle(500, 250, 120, TFT_GREEN);
    
    // Large ellipses
    for (int i = 0; i < 4; i++) {
        M5.Display.drawEllipse(200, 480, 140 - i*2, 80 - i, TFT_YELLOW);
    }
    
    // Large filled ellipse
    M5.Display.fillEllipse(500, 480, 140, 80, TFT_MAGENTA);
    
    // Animated pulsing circle in center
    int radius = 80 + (sin(animationAngle * PI / 180.0) * 50);
    for (int i = 0; i < 5; i++) {
        M5.Display.drawCircle(screenW/2, screenH/2, radius - i*2, TFT_CYAN);
    }
    
    // Concentric circles around center
    for (int i = 0; i < 10; i++) {
        uint16_t color = M5.Display.color565(255 - i*25, 128, i*25);
        M5.Display.drawCircle(screenW/2, screenH/2, 20 + i*20, color);
    }
    
    // Additional decorative circles
    for (int i = 0; i < 6; i++) {
        float angle = i * 60 * PI / 180.0;
        int x = screenW/2 + 250 * cos(angle);
        int y = screenH/2 + 150 * sin(angle);
        M5.Display.fillCircle(x, y, 40, M5.Display.color565(255, i*40, 255 - i*40));
    }
}

void drawTrianglesDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    
    // Large outlined triangles
    for (int i = 0; i < 5; i++) {
        M5.Display.drawTriangle(200, 120 + i*2, 80 + i*2, 320 - i*2, 320 - i*2, 320 - i*2, TFT_WHITE);
    }
    
    // Large filled triangle
    M5.Display.fillTriangle(500, 120, 380, 320, 620, 320, TFT_GREEN);
    
    // Animated rotating triangle in center
    float angle = animationAngle * PI / 180.0;
    int centerX = screenW/2, centerY = screenH/2;
    int radius = 120;
    int x1 = centerX + radius * cos(angle);
    int y1 = centerY + radius * sin(angle);
    int x2 = centerX + radius * cos(angle + 2*PI/3);
    int y2 = centerY + radius * sin(angle + 2*PI/3);
    int x3 = centerX + radius * cos(angle + 4*PI/3);
    int y3 = centerY + radius * sin(angle + 4*PI/3);
    M5.Display.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_YELLOW);
    
    // Different triangle shapes
    for (int i = 0; i < 3; i++) {
        M5.Display.drawTriangle(900 + i*2, 150, 850 + i*2, 280, 1000 - i*2, 270, TFT_MAGENTA);
    }
    
    M5.Display.fillTriangle(1050, 150, 950, 280, 1150, 260, TFT_CYAN);
    
    // Star made of triangles
    for (int i = 0; i < 8; i++) {
        float a = (i * 45) * PI / 180.0;
        int x1 = centerX + 80 * cos(a);
        int y1 = centerY + 80 * sin(a);
        int x2 = centerX + 40 * cos(a + PI/8);
        int y2 = centerY + 40 * sin(a + PI/8);
        int x3 = centerX + 40 * cos(a - PI/8);
        int y3 = centerY + 40 * sin(a - PI/8);
        uint16_t color = M5.Display.color565(255 - i*30, i*30, 200);
        M5.Display.fillTriangle(centerX, centerY, x2, y2, x3, y3, color);
    }
    
    // Additional triangles for decoration
    for (int i = 0; i < 4; i++) {
        int x = 100 + i * 300;
        int y = 500;
        M5.Display.fillTriangle(x, y, x - 60, y + 100, x + 60, y + 100, M5.Display.color565(100 + i*40, 255 - i*40, 180));
    }
}

void drawComplexShapesDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    int startY = 120;
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Complex Shapes Demo", 50, startY);
    
    // Draw a large star
    int centerX = 250, centerY = startY + 200;
    for (int i = 0; i < 5; i++) {
        float angle1 = (i * 72 - 90) * PI / 180.0;
        float angle2 = ((i + 2) * 72 - 90) * PI / 180.0;
        for (int j = 0; j < 3; j++) {
            int x1 = centerX + (80 - j*2) * cos(angle1);
            int y1 = centerY + (80 - j*2) * sin(angle1);
            int x2 = centerX + (80 - j*2) * cos(angle2);
            int y2 = centerY + (80 - j*2) * sin(angle2);
            M5.Display.drawLine(x1, y1, x2, y2, TFT_YELLOW);
        }
    }
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Star", 220, startY + 300);
    
    // Draw a large hexagon
    centerX = 640;
    for (int i = 0; i < 6; i++) {
        float angle1 = (i * 60) * PI / 180.0;
        float angle2 = ((i + 1) * 60) * PI / 180.0;
        for (int j = 0; j < 3; j++) {
            int x1 = centerX + (80 - j*2) * cos(angle1);
            int y1 = centerY + (80 - j*2) * sin(angle1);
            int x2 = centerX + (80 - j*2) * cos(angle2);
            int y2 = centerY + (80 - j*2) * sin(angle2);
            M5.Display.drawLine(x1, y1, x2, y2, TFT_GREEN);
        }
    }
    M5.Display.drawString("Hexagon", 600, startY + 300);
    
    // Draw a large house shape
    int houseX = 950, houseY = startY + 130;
    M5.Display.fillRect(houseX, houseY + 70, 120, 100, TFT_WHITE);
    M5.Display.fillTriangle(houseX - 20, houseY + 70, houseX + 60, houseY, houseX + 140, houseY + 70, TFT_RED);
    M5.Display.fillRect(houseX + 45, houseY + 120, 30, 50, TFT_BROWN);
    M5.Display.fillRect(houseX + 15, houseY + 90, 25, 25, TFT_LIGHTGREY);  // Window
    M5.Display.fillRect(houseX + 80, houseY + 90, 25, 25, TFT_LIGHTGREY);  // Window
    M5.Display.drawString("House", 970, startY + 300);
    
    // Animated spiral in center bottom
    centerX = screenW/2;
    centerY = startY + 380;
    for (int i = 0; i < 60; i++) {
        float angle = (animationAngle + i * 6) * PI / 180.0;
        float radius = i * 2;
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);
        uint16_t color = M5.Display.color565(255 - i*4, i*4, 128);
        M5.Display.fillCircle(x, y, 5, color);
    }
}

void drawInteractiveDemo() {
    int screenW = spriteInitialized ? demoSprite.width() : M5.Display.width();
    int screenH = spriteInitialized ? demoSprite.height() : M5.Display.height();
    int startY = 100;
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Interactive Drawing - Touch to draw!", 50, startY);
    
    // Color palette
    int paletteY = startY + 60;
    uint16_t colors[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_MAGENTA, TFT_CYAN};
    for (int i = 0; i < 7; i++) {
        M5.Display.fillRect(100 + i * 80, paletteY, 70, 70, colors[i]);
        if (drawColor == colors[i]) {
            for (int j = 0; j < 3; j++) {
                M5.Display.drawRect(97 + i * 80 + j, paletteY - 3 + j, 76 - j*2, 76 - j*2, TFT_WHITE);
            }
        }
    }
    
    // Brush size indicators
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Brush Size:", 800, paletteY + 20);
    for (int i = 1; i <= 4; i++) {
        int x = 950 + i * 60;
        M5.Display.fillCircle(x, paletteY + 35, i * 4, TFT_DARKGREY);
        if (brushSize == i * 4 - 1) {
            M5.Display.drawCircle(x, paletteY + 35, i * 4 + 5, TFT_WHITE);
            M5.Display.drawCircle(x, paletteY + 35, i * 4 + 6, TFT_WHITE);
        }
    }
    
    // Drawing area
    int drawAreaY = startY + 160;
    M5.Display.drawRect(50, drawAreaY, screenW - 100, 380, TFT_DARKGREY);
    M5.Display.drawRect(51, drawAreaY + 1, screenW - 102, 378, TFT_DARKGREY);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Drawing Area", screenW/2 - 100, drawAreaY + 390);
    
    // Handle touch drawing
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed()) {
            int x = touch.x;
            int y = touch.y;
            
            // Check color palette
            if (y >= paletteY && y <= paletteY + 70) {
                for (int i = 0; i < 7; i++) {
                    if (x >= 100 + i * 80 && x <= 170 + i * 80) {
                        drawColor = colors[i];
                        displayCurrentDemo();
                        return;
                    }
                }
            }
            
            // Check brush size
            if (y >= paletteY && y <= paletteY + 70) {
                for (int i = 1; i <= 4; i++) {
                    int bx = 950 + i * 60;
                    if (x >= bx - 25 && x <= bx + 25) {
                        brushSize = i * 4 - 1;
                        displayCurrentDemo();
                        return;
                    }
                }
            }
            
            // Draw in the drawing area
            if (x >= 50 && x <= screenW - 50 && y >= drawAreaY && y <= drawAreaY + 380) {
                M5.Display.fillCircle(x, y, brushSize, drawColor);
                if (lastTouchX != -1 && lastTouchY != -1) {
                    // Draw thick line from last position for smooth drawing
                    for (int j = -brushSize/2; j <= brushSize/2; j++) {
                        M5.Display.drawLine(lastTouchX + j, lastTouchY, x + j, y, drawColor);
                        M5.Display.drawLine(lastTouchX, lastTouchY + j, x, y + j, drawColor);
                    }
                }
                lastTouchX = x;
                lastTouchY = y;
            }
        } else {
            lastTouchX = -1;
            lastTouchY = -1;
        }
    }
}

// Touch button helper functions
void drawTouchButton(TouchButton& btn) {
    uint16_t fillColor = btn.pressed ? TFT_YELLOW : btn.color;
    uint16_t textColor = btn.pressed ? TFT_BLACK : TFT_WHITE;
    uint16_t borderColor = btn.pressed ? TFT_YELLOW : TFT_WHITE;
    
    M5.Display.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 15, fillColor);
    M5.Display.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 15, borderColor);
    M5.Display.drawRoundRect(btn.x+1, btn.y+1, btn.w-2, btn.h-2, 14, borderColor);
    M5.Display.drawRoundRect(btn.x+2, btn.y+2, btn.w-4, btn.h-4, 13, borderColor);
    
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextColor(textColor);
    M5.Display.setTextSize(3);
    M5.Display.drawString(btn.label, btn.x + btn.w/2, btn.y + btn.h/2);
}

bool checkTouchButton(TouchButton& btn, int touchX, int touchY) {
    return (touchX >= btn.x && touchX <= btn.x + btn.w &&
            touchY >= btn.y && touchY <= btn.y + btn.h);
}

void drawAllButtons() {
    drawTouchButton(btnPrev);
    drawTouchButton(btnAnimate);
    drawTouchButton(btnNext);
}

void loop() {
    M5.update();
    
    static bool animating = false;
    static ShapeDemo lastDemo = currentDemo;
    static unsigned long lastAnimUpdate = 0;
    
    // Handle touch input
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        
        if (touch.wasPressed()) {
            // Check Previous button
            if (checkTouchButton(btnPrev, touch.x, touch.y)) {
                btnPrev.pressed = true;
                drawTouchButton(btnPrev);
                animating = false;  // Stop animation when switching demos
                currentDemo = (ShapeDemo)((currentDemo - 1 + SHAPE_DEMO_COUNT) % SHAPE_DEMO_COUNT);
                delay(100);
                needsFullRedraw = true;  // Request full redraw for new demo
                displayCurrentDemo();
                btnPrev.pressed = false;
                lastDemo = currentDemo;
            }
            // Check Next button
            else if (checkTouchButton(btnNext, touch.x, touch.y)) {
                btnNext.pressed = true;
                drawTouchButton(btnNext);
                animating = false;  // Stop animation when switching demos
                currentDemo = (ShapeDemo)((currentDemo + 1) % SHAPE_DEMO_COUNT);
                delay(100);
                needsFullRedraw = true;  // Request full redraw for new demo
                displayCurrentDemo();
                btnNext.pressed = false;
                lastDemo = currentDemo;
            }
            // Check Animate button
            else if (checkTouchButton(btnAnimate, touch.x, touch.y)) {
                btnAnimate.pressed = true;
                drawTouchButton(btnAnimate);
                animating = !animating;
                delay(100);
                btnAnimate.pressed = false;
                drawTouchButton(btnAnimate);
                if (!animating) {
                    // Redraw once when stopping animation to clear any artifacts
                    needsFullRedraw = false;  // Don't clear title/buttons
                    displayCurrentDemo();
                }
            }
        }
    }
    
    // Handle animation with reduced flicker
    if (animating && millis() - lastAnimUpdate > 100) {  // Slower update rate to reduce flicker
        animationAngle += 10;
        if (animationAngle >= 360) animationAngle = 0;
        animationStep += 2;
        
        lastAnimUpdate = millis();
        
        // Only redraw the demo content area, not title/buttons
        needsFullRedraw = false;
        displayCurrentDemo();
    }
    
    // Check if demo changed to force redraw
    if (lastDemo != currentDemo) {
        lastDemo = currentDemo;
        needsFullRedraw = true;
        displayCurrentDemo();
    }
    
    // Handle interactive drawing continuously
    if (currentDemo == DEMO_INTERACTIVE) {
        drawInteractiveDemo();
    }
    
    delay(10);
}