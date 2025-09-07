/*
 * M5GFX Tutorial - 02_colors
 * 
 * This demo explores color management in M5GFX:
 * - RGB565 color format
 * - Color conversion functions
 * - Gradients and color transitions
 * - Color palettes
 * - Transparency and alpha blending
 * - HSV color space
 * 
 * Key concepts:
 * - 16-bit color representation (5-6-5 bits for R-G-B)
 * - M5.Display.color565() for color creation
 * - Color mixing and blending
 * - Performance considerations with colors
 */

#include <M5Unified.h>

enum ColorDemo {
    DEMO_RGB_BASICS,
    DEMO_GRADIENTS,
    DEMO_COLOR_WHEEL,
    DEMO_PALETTES,
    DEMO_TRANSPARENCY,
    DEMO_COUNT
};

ColorDemo currentDemo = DEMO_RGB_BASICS;
const char* demoNames[] = {
    "RGB Basics",
    "Gradients",
    "Color Wheel",
    "Palettes",
    "Transparency"
};

// Convert HSV to RGB565
uint16_t hsv2rgb(float h, float s, float v) {
    float r, g, b;
    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    
    return M5.Display.color565(r * 255, g * 255, b * 255);
}

void drawRGBBasics() {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    
    // RGB component demonstration
    M5.Display.drawString("RGB Components:", 10, 70);
    
    // Red gradient
    M5.Display.drawString("R:", 10, 90);
    for (int i = 0; i < 256; i += 2) {
        M5.Display.drawLine(40 + i/2, 90, 40 + i/2, 110, M5.Display.color565(i, 0, 0));
    }
    
    // Green gradient
    M5.Display.drawString("G:", 10, 115);
    for (int i = 0; i < 256; i += 2) {
        M5.Display.drawLine(40 + i/2, 115, 40 + i/2, 135, M5.Display.color565(0, i, 0));
    }
    
    // Blue gradient
    M5.Display.drawString("B:", 10, 140);
    for (int i = 0; i < 256; i += 2) {
        M5.Display.drawLine(40 + i/2, 140, 40 + i/2, 160, M5.Display.color565(0, 0, i));
    }
    
    // Color mixing
    M5.Display.drawString("Color Mixing:", 10, 170);
    M5.Display.fillCircle(60, 200, 25, TFT_RED);
    M5.Display.fillCircle(80, 200, 25, TFT_GREEN);
    M5.Display.fillCircle(70, 215, 25, TFT_BLUE);
    
    // Common colors
    M5.Display.drawString("Common Colors:", 180, 90);
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, 
                         TFT_CYAN, TFT_MAGENTA, TFT_WHITE, TFT_BLACK};
    const char* names[] = {"RED", "GREEN", "BLUE", "YELLOW", 
                           "CYAN", "MAGENTA", "WHITE", "BLACK"};
    
    for (int i = 0; i < 8; i++) {
        M5.Display.fillRect(180, 110 + i * 15, 40, 12, colors[i]);
        M5.Display.setTextColor(i == 7 ? TFT_WHITE : TFT_BLACK);
        M5.Display.drawString(names[i], 225, 110 + i * 15);
    }
    M5.Display.setTextColor(TFT_WHITE);
}

void drawGradients() {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Gradient Types:", 10, 70);
    
    // Linear gradient horizontal
    M5.Display.drawString("Linear H:", 10, 90);
    for (int x = 0; x < 100; x++) {
        uint8_t val = map(x, 0, 100, 0, 255);
        M5.Display.drawLine(70 + x, 90, 70 + x, 110, M5.Display.color565(val, 0, 255 - val));
    }
    
    // Linear gradient vertical
    M5.Display.drawString("Linear V:", 10, 120);
    for (int y = 0; y < 30; y++) {
        uint8_t val = map(y, 0, 30, 0, 255);
        M5.Display.drawLine(70, 120 + y, 170, 120 + y, M5.Display.color565(255 - val, val, 128));
    }
    
    // Radial gradient
    M5.Display.drawString("Radial:", 10, 160);
    int cx = 120, cy = 180;
    for (int r = 30; r > 0; r--) {
        uint8_t val = map(r, 0, 30, 0, 255);
        M5.Display.fillCircle(cx, cy, r, M5.Display.color565(val, 255 - val, 128));
    }
    
    // Rainbow gradient
    M5.Display.drawString("Rainbow:", 180, 90);
    for (int x = 0; x < 120; x++) {
        float hue = (float)x / 120.0;
        M5.Display.drawLine(180 + x, 110, 180 + x, 140, hsv2rgb(hue, 1.0, 1.0));
    }
    
    // 2D gradient
    M5.Display.drawString("2D Gradient:", 180, 160);
    for (int x = 0; x < 40; x += 2) {
        for (int y = 0; y < 40; y += 2) {
            uint8_t r = map(x, 0, 40, 0, 255);
            uint8_t g = map(y, 0, 40, 0, 255);
            M5.Display.fillRect(180 + x, 180 + y, 2, 2, M5.Display.color565(r, g, 128));
        }
    }
}

void drawColorWheel() {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("HSV Color Wheel:", 10, 70);
    
    int centerX = M5.Display.width() / 2;
    int centerY = 150;
    int radius = 60;
    
    // Draw color wheel
    for (float angle = 0; angle < 360; angle += 2) {
        float hue = angle / 360.0;
        for (int r = 0; r < radius; r += 2) {
            float saturation = (float)r / radius;
            int x = centerX + r * cos(angle * PI / 180);
            int y = centerY + r * sin(angle * PI / 180);
            M5.Display.fillRect(x, y, 2, 2, hsv2rgb(hue, saturation, 1.0));
        }
    }
    
    // Value bar
    M5.Display.drawString("Value:", 10, 220);
    for (int i = 0; i < 256; i += 2) {
        float value = i / 255.0;
        M5.Display.drawLine(60 + i/2, 220, 60 + i/2, 240, hsv2rgb(0.0, 0.0, value));
    }
}

void drawPalettes() {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Color Palettes:", 10, 70);
    
    // Warm palette
    M5.Display.drawString("Warm:", 10, 90);
    uint16_t warm[] = {0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0};
    for (int i = 0; i < 6; i++) {
        M5.Display.fillRect(60 + i * 20, 90, 18, 18, warm[i]);
    }
    
    // Cool palette
    M5.Display.drawString("Cool:", 10, 115);
    uint16_t cool[] = {0x001F, 0x03FF, 0x07FF, 0x0FFF, 0x87FF, 0xCFFF};
    for (int i = 0; i < 6; i++) {
        M5.Display.fillRect(60 + i * 20, 115, 18, 18, cool[i]);
    }
    
    // Earth tones
    M5.Display.drawString("Earth:", 10, 140);
    uint16_t earth[] = {0x8410, 0xA514, 0xC618, 0x6B4D, 0x3186, 0x528A};
    for (int i = 0; i < 6; i++) {
        M5.Display.fillRect(60 + i * 20, 140, 18, 18, earth[i]);
    }
    
    // Pastel
    M5.Display.drawString("Pastel:", 10, 165);
    uint16_t pastel[] = {0xFCF3, 0xF7DF, 0xE7FC, 0xDFFF, 0xF7FF, 0xFFDF};
    for (int i = 0; i < 6; i++) {
        M5.Display.fillRect(60 + i * 20, 165, 18, 18, pastel[i]);
    }
    
    // Monochrome
    M5.Display.drawString("Gray:", 10, 190);
    for (int i = 0; i < 6; i++) {
        uint8_t gray = i * 51;
        M5.Display.fillRect(60 + i * 20, 190, 18, 18, M5.Display.color565(gray, gray, gray));
    }
}

void drawTransparency() {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Transparency Effects:", 10, 70);
    
    // Background pattern
    for (int x = 20; x < 300; x += 10) {
        for (int y = 90; y < 230; y += 10) {
            M5.Display.fillRect(x, y, 9, 9, ((x + y) / 10) % 2 ? TFT_DARKGREY : TFT_LIGHTGREY);
        }
    }
    
    // Simulated transparency with different alpha levels
    M5.Display.drawString("Alpha Levels:", 10, 90);
    
    for (int i = 0; i < 5; i++) {
        int alpha = 255 - i * 50;
        int x = 50 + i * 50;
        
        // Blend color with background
        for (int px = 0; px < 40; px++) {
            for (int py = 0; py < 40; py++) {
                // Get "background" color (checker pattern)
                uint16_t bgColor = ((x + px + 130 + py) / 10) % 2 ? TFT_DARKGREY : TFT_LIGHTGREY;
                
                // Extract RGB components from background
                uint8_t bgR = (bgColor >> 11) << 3;
                uint8_t bgG = (bgColor >> 5) << 2;
                uint8_t bgB = (bgColor) << 3;
                
                // Blend with red
                uint8_t r = (255 * alpha + bgR * (255 - alpha)) / 255;
                uint8_t g = (0 * alpha + bgG * (255 - alpha)) / 255;
                uint8_t b = (0 * alpha + bgB * (255 - alpha)) / 255;
                
                M5.Display.drawPixel(x + px, 130 + py, M5.Display.color565(r, g, b));
            }
        }
        
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString(String(alpha * 100 / 255) + "%", x + 10, 175);
    }
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    drawInterface();
}

void drawInterface() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Color Management", M5.Display.width()/2, 10);
    
    // Demo name
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString(demoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + "/" + String(DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Draw current demo
    switch(currentDemo) {
        case DEMO_RGB_BASICS: drawRGBBasics(); break;
        case DEMO_GRADIENTS: drawGradients(); break;
        case DEMO_COLOR_WHEEL: drawColorWheel(); break;
        case DEMO_PALETTES: drawPalettes(); break;
        case DEMO_TRANSPARENCY: drawTransparency(); break;
    }
    
    // Navigation
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("Touch to continue", M5.Display.width()/2, M5.Display.height() - 10);
}

void loop() {
    M5.update();
    
    if (M5.Touch.wasPressed() || M5.BtnA.wasPressed() || M5.BtnC.wasPressed()) {
        currentDemo = (ColorDemo)((currentDemo + 1) % DEMO_COUNT);
        drawInterface();
        
        if (M5.Speaker.isEnabled()) {
            M5.Speaker.tone(1000, 50);
        }
    }
    
    delay(10);
}