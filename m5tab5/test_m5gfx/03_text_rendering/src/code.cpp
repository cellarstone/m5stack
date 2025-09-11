/*
 * M5GFX Tutorial - 03_text_rendering
 * 
 * This demo demonstrates advanced text rendering capabilities in M5GFX:
 * - Built-in fonts and font sizes
 * - Text colors and background colors
 * - Text alignment using datums
 * - Custom font loading and usage
 * - Text effects and styling
 * - Multi-line text handling
 * - Text measurement and positioning
 * - Dynamic text updates and animations
 * 
 * Key concepts:
 * - Font management and selection
 * - Text datum system for alignment
 * - String positioning and measurement
 * - UTF-8 character support
 * - Performance considerations for text
 */

#include <M5Unified.h>

// Forward declarations
void displayWelcome();
void displayCurrentDemo();
void drawCurrentTextDemo();
void drawBasicTextDemo();
void drawFontSizesDemo();
void drawTextColorsDemo();
void drawTextAlignmentDemo();
void drawTextEffectsDemo();
void drawDynamicTextDemo();
void drawTouchButtons();

// Demo modes for different text features
enum TextDemo {
    DEMO_BASIC_TEXT,
    DEMO_FONT_SIZES,
    DEMO_TEXT_COLORS,
    DEMO_TEXT_ALIGNMENT,
    DEMO_TEXT_EFFECTS,
    DEMO_DYNAMIC_TEXT,
    TEXT_DEMO_COUNT
};

TextDemo currentDemo = DEMO_BASIC_TEXT;
const char* textDemoNames[] = {
    "Basic Text",
    "Font Sizes",
    "Text Colors",
    "Text Alignment",
    "Text Effects",
    "Dynamic Text"
};

// Animation variables
unsigned long lastUpdate = 0;
float animationAngle = 0;
int animationStep = 0;
unsigned long textUpdateTime = 0;

// Dynamic text variables
String dynamicText = "Hello World!";
int textColorR = 255, textColorG = 255, textColorB = 255;
bool colorDirection = true;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
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
    M5.Display.drawString("M5GFX Text", M5.Display.width()/2, 150);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Rendering", M5.Display.width()/2, 280);
    M5.Display.drawString("System", M5.Display.width()/2, 350);
    
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Master fonts, alignment, and text effects", M5.Display.width()/2, 450);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Built-in and Custom Fonts", M5.Display.width()/2, 510);
    M5.Display.drawString("• Alignment and Positioning", M5.Display.width()/2, 560);
    M5.Display.drawString("• Colors and Effects", M5.Display.width()/2, 610);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header - scaled for 1280x720
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(4);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Text", M5.Display.width()/2, 30);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(3);
    M5.Display.drawString(textDemoNames[currentDemo], M5.Display.width()/2, 90);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(TEXT_DEMO_COUNT), M5.Display.width()/2, 130);
    
    // Touch buttons at bottom
    drawTouchButtons();
    
    // Draw demo-specific content
    drawCurrentTextDemo();
}

void drawTouchButtons() {
    int btnWidth = 200;
    int btnHeight = 60;
    int btnY = M5.Display.height() - 80;
    int spacing = 50;
    int totalWidth = btnWidth * 3 + spacing * 2;
    int startX = (M5.Display.width() - totalWidth) / 2;
    
    // Previous button
    M5.Display.fillRoundRect(startX, btnY, btnWidth, btnHeight, 15, TFT_DARKGREEN);
    M5.Display.drawRoundRect(startX, btnY, btnWidth, btnHeight, 15, TFT_GREEN);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("< PREV", startX + btnWidth/2, btnY + btnHeight/2);
    
    // Animate button
    M5.Display.fillRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_NAVY);
    M5.Display.drawRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_BLUE);
    M5.Display.drawString("ANIMATE", startX + btnWidth + spacing + btnWidth/2, btnY + btnHeight/2);
    
    // Next button
    M5.Display.fillRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_DARKGREEN);
    M5.Display.drawRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_GREEN);
    M5.Display.drawString("NEXT >", startX + (btnWidth + spacing) * 2 + btnWidth/2, btnY + btnHeight/2);
}

void drawCurrentTextDemo() {
    switch(currentDemo) {
        case DEMO_BASIC_TEXT:
            drawBasicTextDemo();
            break;
        case DEMO_FONT_SIZES:
            drawFontSizesDemo();
            break;
        case DEMO_TEXT_COLORS:
            drawTextColorsDemo();
            break;
        case DEMO_TEXT_ALIGNMENT:
            drawTextAlignmentDemo();
            break;
        case DEMO_TEXT_EFFECTS:
            drawTextEffectsDemo();
            break;
        case DEMO_DYNAMIC_TEXT:
            drawDynamicTextDemo();
            break;
    }
}

void drawBasicTextDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Basic Text Rendering", 10, startY);
    
    // Simple text
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Default font and size", 10, startY + 20);
    
    // Different text sizes
    M5.Display.drawString("Size 1 text", 10, startY + 40);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Size 2 text", 10, startY + 60);
    M5.Display.setTextSize(3);
    M5.Display.drawString("Size 3", 10, startY + 85);
    
    // Reset size
    M5.Display.setTextSize(1);
    
    // Text with background
    M5.Display.setTextColor(TFT_BLACK, TFT_YELLOW);
    M5.Display.drawString("Text with background", 10, startY + 120);
    
    // Transparent background (default)
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Transparent background", 10, startY + 140);
    
    // Character display
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Special chars: !@#$%^&*()", 10, startY + 160);
    M5.Display.drawString("Numbers: 0123456789", 10, startY + 180);
}

void drawFontSizesDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Font Size Comparison", 10, startY);
    
    // Size progression
    const char* sampleText = "Sample Text";
    int yPos = startY + 25;
    
    for (int size = 1; size <= 4; size++) {
        M5.Display.setTextSize(size);
        String label = "Size " + String(size) + ": " + String(sampleText);
        
        // Color gradient based on size
        uint16_t color = M5.Display.color565(50 + size * 50, 100 + size * 30, 255 - size * 40);
        M5.Display.setTextColor(color);
        M5.Display.drawString(label, 10, yPos);
        
        // Calculate next position based on text height
        yPos += M5.Display.fontHeight() * size + 5;
    }
    
    // Font metrics display
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Font Metrics:", 10, yPos + 10);
    
    M5.Display.setTextColor(TFT_WHITE);
    String heightInfo = "Font Height: " + String(M5.Display.fontHeight());
    M5.Display.drawString(heightInfo, 10, yPos + 30);
    
    String widthInfo = "Text Width ('Hello'): " + String(M5.Display.textWidth("Hello"));
    M5.Display.drawString(widthInfo, 10, yPos + 50);
}

void drawTextColorsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Text Colors & Backgrounds", 10, startY);
    
    // Primary colors
    M5.Display.setTextSize(2);
    int yPos = startY + 25;
    
    const uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, 
                              TFT_MAGENTA, TFT_CYAN, TFT_WHITE, TFT_ORANGE};
    const char* colorNames[] = {"RED", "GREEN", "BLUE", "YELLOW", 
                               "MAGENTA", "CYAN", "WHITE", "ORANGE"};
    
    for (int i = 0; i < 4; i++) {
        M5.Display.setTextColor(colors[i]);
        M5.Display.drawString(colorNames[i], 10, yPos);
        yPos += 25;
    }
    
    // Right column
    yPos = startY + 25;
    for (int i = 4; i < 8; i++) {
        M5.Display.setTextColor(colors[i]);
        M5.Display.drawString(colorNames[i], 150, yPos);
        yPos += 25;
    }
    
    // Gradient effect
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("RGB Gradient:", 10, yPos + 10);
    
    String gradientText = "Gradient Text Effect";
    int textWidth = M5.Display.textWidth(gradientText);
    int startX = 10;
    
    for (int i = 0; i < gradientText.length(); i++) {
        uint8_t r = 255 * i / gradientText.length();
        uint8_t g = 255 - (255 * i / gradientText.length());
        uint8_t b = 128 + 127 * sin(i * 0.5);
        
        M5.Display.setTextColor(M5.Display.color565(r, g, b));
        String ch = String(gradientText.charAt(i));
        M5.Display.drawString(ch, startX, yPos + 30);
        startX += M5.Display.textWidth(ch);
    }
}

void drawTextAlignmentDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Text Alignment (Datums)", 10, startY);
    
    // Reference lines
    int centerX = M5.Display.width() / 2;
    int centerY = startY + 80;
    
    // Draw reference cross
    M5.Display.drawLine(centerX - 50, centerY, centerX + 50, centerY, TFT_DARKGREY);
    M5.Display.drawLine(centerX, centerY - 30, centerX, centerY + 30, TFT_DARKGREY);
    
    // Text with different alignments
    M5.Display.setTextSize(2);
    
    // Top alignments
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("TL", centerX, centerY - 30);
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("TC", centerX, centerY - 30);
    
    M5.Display.setTextColor(TFT_BLUE);
    M5.Display.setTextDatum(TR_DATUM);
    M5.Display.drawString("TR", centerX, centerY - 30);
    
    // Middle alignments
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(ML_DATUM);
    M5.Display.drawString("ML", centerX, centerY);
    
    M5.Display.setTextColor(TFT_MAGENTA);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("MC", centerX, centerY);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextDatum(MR_DATUM);
    M5.Display.drawString("MR", centerX, centerY);
    
    // Bottom alignments
    M5.Display.setTextColor(TFT_ORANGE);
    M5.Display.setTextDatum(BL_DATUM);
    M5.Display.drawString("BL", centerX, centerY + 30);
    
    M5.Display.setTextColor(TFT_PINK);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("BC", centerX, centerY + 30);
    
    M5.Display.setTextColor(TFT_GREENYELLOW);
    M5.Display.setTextDatum(BR_DATUM);
    M5.Display.drawString("BR", centerX, centerY + 30);
    
    // Legend
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("T=Top, M=Middle, B=Bottom", 10, startY + 130);
    M5.Display.drawString("L=Left, C=Center, R=Right", 10, startY + 150);
}

void drawTextEffectsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Text Effects & Styling", 10, startY);
    
    // Shadow effect
    M5.Display.setTextSize(3);
    String shadowText = "SHADOW";
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString(shadowText, 12, startY + 27); // Shadow offset
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString(shadowText, 10, startY + 25);
    
    // Outline effect (simulated with multiple draws)
    M5.Display.setTextSize(2);
    String outlineText = "OUTLINE";
    int outlineX = 10, outlineY = startY + 65;
    
    // Draw outline
    M5.Display.setTextColor(TFT_BLACK);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                M5.Display.drawString(outlineText, outlineX + dx, outlineY + dy);
            }
        }
    }
    // Draw main text
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString(outlineText, outlineX, outlineY);
    
    // Animated wave text
    M5.Display.setTextSize(2);
    String waveText = "WAVE EFFECT";
    M5.Display.setTextColor(TFT_CYAN);
    int waveStartX = 10;
    int waveBaseY = startY + 110;
    
    for (int i = 0; i < waveText.length(); i++) {
        if (waveText.charAt(i) != ' ') {
            float offset = sin((animationAngle * PI / 180.0) + i * 0.5) * 8;
            String ch = String(waveText.charAt(i));
            uint8_t intensity = 128 + 127 * sin((animationAngle * PI / 180.0) + i * 0.3);
            M5.Display.setTextColor(M5.Display.color565(intensity, intensity/2, 255-intensity));
            M5.Display.drawString(ch, waveStartX, waveBaseY + offset);
        }
        waveStartX += M5.Display.textWidth(String(waveText.charAt(i)));
    }
    
    // Typewriter effect
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN);
    String typeText = "Typewriter effect...";
    int visibleChars = (animationStep / 5) % (typeText.length() + 10);
    if (visibleChars <= typeText.length()) {
        M5.Display.drawString(typeText.substring(0, visibleChars) + "_", 10, startY + 150);
    } else {
        M5.Display.drawString(typeText, 10, startY + 150);
    }
}

void drawDynamicTextDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Dynamic Text Updates", 10, startY);
    
    // Real-time clock simulation
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    unsigned long seconds = millis() / 1000;
    String timeStr = String(seconds / 3600) + ":" + 
                    String((seconds % 3600) / 60) + ":" + 
                    String(seconds % 60);
    M5.Display.drawString("Time: " + timeStr, 10, startY + 25);
    
    // Counter
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_YELLOW);
    String counterStr = String(animationStep);
    M5.Display.drawString("Count: " + counterStr, 10, startY + 50);
    
    // Animated color text
    M5.Display.setTextSize(2);
    uint16_t animColor = M5.Display.color565(textColorR, textColorG, textColorB);
    M5.Display.setTextColor(animColor);
    M5.Display.drawString("Color Fade", 10, startY + 90);
    
    // Update color animation
    if (colorDirection) {
        textColorR -= 3;
        textColorG += 2;
        if (textColorR <= 50) colorDirection = false;
    } else {
        textColorR += 3;
        textColorG -= 2;
        if (textColorR >= 255) colorDirection = true;
    }
    
    // Scrolling text
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_GREEN);
    String scrollText = "This is a scrolling text message that moves across the screen... ";
    int scrollOffset = (animationStep * 2) % (M5.Display.textWidth(scrollText) + M5.Display.width());
    M5.Display.drawString(scrollText, M5.Display.width() - scrollOffset, startY + 125);
    
    // Performance info
    M5.Display.setTextColor(TFT_CYAN);
    unsigned long fps = 1000 / max(1UL, millis() - lastUpdate);
    M5.Display.drawString("FPS: " + String(fps), 10, startY + 150);
    
    String memInfo = "Free heap: " + String(ESP.getFreeHeap());
    M5.Display.drawString(memInfo, 10, startY + 170);
}

void loop() {
    M5.update();
    
    // Handle touch input
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            int btnWidth = 200;
            int btnHeight = 60;
            int btnY = M5.Display.height() - 80;
            int spacing = 50;
            int totalWidth = btnWidth * 3 + spacing * 2;
            int startX = (M5.Display.width() - totalWidth) / 2;
            
            // Check which button was pressed
            if (touch.y >= btnY && touch.y <= btnY + btnHeight) {
                if (touch.x >= startX && touch.x <= startX + btnWidth) {
                    // Previous button
                    currentDemo = (TextDemo)((currentDemo - 1 + TEXT_DEMO_COUNT) % TEXT_DEMO_COUNT);
                    displayCurrentDemo();
                } else if (touch.x >= startX + btnWidth + spacing && 
                          touch.x <= startX + btnWidth + spacing + btnWidth) {
                    // Animate button
                    animationStep = 0; // Reset animation
                    displayCurrentDemo();
                } else if (touch.x >= startX + (btnWidth + spacing) * 2 && 
                          touch.x <= startX + (btnWidth + spacing) * 2 + btnWidth) {
                    // Next button
                    currentDemo = (TextDemo)((currentDemo + 1) % TEXT_DEMO_COUNT);
                    displayCurrentDemo();
                }
            }
        }
    }
    
    // Also handle physical buttons if available
    if (M5.BtnA.wasPressed()) {
        currentDemo = (TextDemo)((currentDemo - 1 + TEXT_DEMO_COUNT) % TEXT_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        animationStep = 0; // Reset animation
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (TextDemo)((currentDemo + 1) % TEXT_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS
        animationAngle += 5;
        if (animationAngle >= 360) animationAngle = 0;
        
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Only redraw animated demos
        if (currentDemo == DEMO_TEXT_EFFECTS || currentDemo == DEMO_DYNAMIC_TEXT) {
            drawCurrentTextDemo();
        }
        
        lastUpdate = millis();
    }
}