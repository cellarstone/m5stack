#include <M5Unified.h>
#include <M5GFX.h>

// Create sprite object for effects
LGFX_Sprite sprite;

// Demo state variables
int currentDemo = 0;
const int totalDemos = 6;
unsigned long lastDemoChange = 0;
unsigned long demoInterval = 5000; // Change demo every 5 seconds
float animationTime = 0;

// Touch variables
bool touchDetected = false;
int touchX = 0, touchY = 0;

// Function declarations
void demo1_GradientAndAlpha();
void demo2_RotatingSprites();
void demo3_TextEffects();
void demo4_DrawingPrimitives();
void demo5_ParticleSystem();
void demo6_WaveEffect();
void drawDemoTitle(const char* title);

// Particle system for demo 5
struct Particle {
    float x, y;
    float vx, vy;
    uint16_t color;
    int life;
};

const int MAX_PARTICLES = 50;
Particle particles[MAX_PARTICLES];

void setup() {
    // Initialize M5Stack Tab5
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // Setup display
    M5.Display.setRotation(3);
    M5.Display.setBrightness(128);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize sprite for effects  
    sprite.setColorDepth(16);
    sprite.createSprite(100, 100);
    
    // Initialize particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].life = 0;
    }
    
    // Welcome message
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextSize(2);
    M5.Display.drawString("M5GFX Demo", M5.Display.width()/2, M5.Display.height()/2 - 20);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Touch to switch demos", M5.Display.width()/2, M5.Display.height()/2 + 20);
    M5.Display.drawString("Starting in 2 seconds...", M5.Display.width()/2, M5.Display.height()/2 + 40);
    delay(2000);
    
    lastDemoChange = millis();
}

void loop() {
    M5.update();
    
    // Check for touch to manually switch demos
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        touchDetected = true;
        touchX = touch.x;
        touchY = touch.y;
        currentDemo = (currentDemo + 1) % totalDemos;
        lastDemoChange = millis();
        
        // Audio feedback
        M5.Speaker.tone(800, 50);
    }
    
    // Auto-switch demos
    if (millis() - lastDemoChange > demoInterval) {
        currentDemo = (currentDemo + 1) % totalDemos;
        lastDemoChange = millis();
    }
    
    // Update animation time
    animationTime += 0.02;
    
    // Clear display directly (simpler approach)
    M5.Display.fillScreen(TFT_BLACK);
    
    // Run current demo directly on display
    switch (currentDemo) {
        case 0: demo1_GradientAndAlpha(); break;
        case 1: demo2_RotatingSprites(); break; 
        case 2: demo3_TextEffects(); break;
        case 3: demo4_DrawingPrimitives(); break;
        case 4: demo5_ParticleSystem(); break;
        case 5: demo6_WaveEffect(); break;
    }
    
    delay(10);
}

void demo1_GradientAndAlpha() {
    drawDemoTitle("Gradients & Alpha Blending");
    
    // Vertical gradient
    for (int y = 40; y < 140; y++) {
        uint8_t r = map(y, 40, 140, 0, 255);
        uint8_t g = map(y, 40, 140, 255, 0);
        uint8_t b = 128;
        M5.Display.drawFastHLine(20, y, 100, M5.Display.color565(r, g, b));
    }
    
    // Horizontal gradient
    for (int x = 150; x < 250; x++) {
        uint8_t r = 128;
        uint8_t g = map(x, 150, 250, 0, 255);
        uint8_t b = map(x, 150, 250, 255, 0);
        M5.Display.drawFastVLine(x, 40, 100, M5.Display.color565(r, g, b));
    }
    
    // Radial gradient circles with alpha
    int centerX = M5.Display.width() - 100;
    int centerY = 90;
    for (int r = 50; r > 0; r -= 2) {
        uint8_t alpha = map(r, 0, 50, 255, 0);
        uint16_t color = M5.Display.color565(255 * alpha / 255, 100 * alpha / 255, 200 * alpha / 255);
        M5.Display.fillCircle(centerX, centerY, r, color);
    }
    
    // Animated overlapping circles
    for (int i = 0; i < 3; i++) {
        int x = M5.Display.width()/2 + cos(animationTime + i * 2.09) * 50;
        int y = 180 + sin(animationTime + i * 2.09) * 30;
        uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE};
        
        // Draw with transparency effect (simplified)
        for (int r = 30; r > 0; r--) {
            uint8_t intensity = map(r, 0, 30, 100, 20);
            uint16_t color = M5.Display.color565(
                (colors[i] >> 11) * intensity / 100,
                ((colors[i] >> 5) & 0x3F) * intensity / 100,
                (colors[i] & 0x1F) * intensity / 100
            );
            M5.Display.fillCircle(x, y, r, color);
        }
    }
}

void demo2_RotatingSprites() {
    drawDemoTitle("Rotating Sprites & Transformations");
    
    // Create pattern in sprite
    sprite.fillScreen(TFT_BLACK);
    sprite.fillRect(10, 10, 30, 30, TFT_CYAN);
    sprite.fillCircle(70, 30, 15, TFT_MAGENTA);
    sprite.fillTriangle(20, 60, 50, 60, 35, 80, TFT_YELLOW);
    sprite.drawRect(60, 60, 30, 30, TFT_WHITE);
    
    // Draw multiple rotating sprites
    for (int i = 0; i < 4; i++) {
        float angle = animationTime + i * PI / 2;
        int x = M5.Display.width()/2 + cos(angle) * 100;
        int y = M5.Display.height()/2 + sin(angle) * 60;
        
        // Set pivot point and push with rotation
        sprite.setPivot(sprite.width()/2, sprite.height()/2);
        M5.Display.setPivot(x, y);
        sprite.pushRotated(&M5.Display, angle * 180 / PI);
    }
    
    // Central rotating sprite with scaling effect
    float scale = 1.0 + sin(animationTime) * 0.3;
    M5.Display.setPivot(M5.Display.width()/2, M5.Display.height()/2);
    sprite.setPivot(sprite.width()/2, sprite.height()/2);
    sprite.pushRotateZoom(&M5.Display, M5.Display.width()/2, M5.Display.height()/2, 
                          animationTime * 50, scale, scale);
    
    // Draw orbit paths
    M5.Display.drawCircle(M5.Display.width()/2, M5.Display.height()/2, 100, TFT_DARKGREY);
    M5.Display.drawCircle(M5.Display.width()/2, M5.Display.height()/2, 60, TFT_DARKGREY);
}

void demo3_TextEffects() {
    drawDemoTitle("Text Rendering Effects");
    
    // Different text sizes
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.setTextSize(1);
    M5.Display.drawString("Size 1: M5GFX Text", 10, 40);
    
    M5.Display.setTextSize(2);
    M5.Display.drawString("Size 2: Graphics", 10, 60);
    
    M5.Display.setTextSize(3);
    M5.Display.drawString("Size 3: Demo", 10, 85);
    
    // Colored text
    M5.Display.setTextSize(2);
    const char* text = "Rainbow";
    int xPos = 10;
    int yPos = 130;
    uint16_t colors[] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_PURPLE, TFT_PINK};
    
    for (int i = 0; i < 7; i++) {
        M5.Display.setTextColor(colors[i]);
        M5.Display.drawChar(text[i], xPos + i * 16, yPos);
    }
    
    // Rotating text
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextColor(TFT_CYAN);
    for (int i = 0; i < 8; i++) {
        float angle = animationTime + i * PI / 4;
        int x = M5.Display.width() - 100 + cos(angle) * 40;
        int y = 100 + sin(angle) * 40;
        M5.Display.drawNumber(i, x, y);
    }
    
    // Wave text effect
    M5.Display.setTextSize(2);
    const char* waveText = "WAVE EFFECT";
    M5.Display.setTextDatum(TC_DATUM);
    for (int i = 0; i < strlen(waveText); i++) {
        int x = 150 + i * 15;
        int y = 180 + sin(animationTime * 2 + i * 0.5) * 10;
        M5.Display.setTextColor(M5.Display.color565(200, 100 + sin(animationTime + i) * 100, 255));
        M5.Display.drawChar(waveText[i], x, y);
    }
}

void demo4_DrawingPrimitives() {
    drawDemoTitle("Drawing Primitives & Shapes");
    
    // Animated rectangles
    for (int i = 0; i < 5; i++) {
        int size = 20 + i * 10;
        int x = 50 + sin(animationTime + i * 0.5) * 20;
        int y = 80;
        uint16_t color = M5.Display.color565(255 - i * 40, i * 40, 128);
        M5.Display.fillRect(x - size/2, y - size/2, size, size, color);
        M5.Display.drawRect(x - size/2 - 2, y - size/2 - 2, size + 4, size + 4, TFT_WHITE);
    }
    
    // Bezier curves (simulated with lines)
    M5.Display.setTextDatum(TL_DATUM);
    int x0 = 200, y0 = 60;
    int x1 = 250 + sin(animationTime) * 30, y1 = 100;
    int x2 = 300, y2 = 60 + cos(animationTime) * 20;
    
    // Draw bezier control points and lines
    M5.Display.fillCircle(x0, y0, 3, TFT_RED);
    M5.Display.fillCircle(x1, y1, 3, TFT_GREEN);
    M5.Display.fillCircle(x2, y2, 3, TFT_BLUE);
    M5.Display.drawLine(x0, y0, x1, y1, TFT_DARKGREY);
    M5.Display.drawLine(x1, y1, x2, y2, TFT_DARKGREY);
    
    // Draw bezier curve
    for (float t = 0; t <= 1.0; t += 0.05) {
        float x = (1-t)*(1-t)*x0 + 2*(1-t)*t*x1 + t*t*x2;
        float y = (1-t)*(1-t)*y0 + 2*(1-t)*t*y1 + t*t*y2;
        M5.Display.fillCircle(x, y, 2, TFT_YELLOW);
    }
    
    // Filled polygons
    int centerX = 100;
    int centerY = 180;
    int radius = 30;
    int sides = 3 + (int)(animationTime) % 6; // 3 to 8 sides
    
    for (int j = 0; j < 3; j++) {
        int offsetX = j * 80;
        int points[sides * 2];
        
        for (int i = 0; i < sides; i++) {
            float angle = animationTime + (i * 2 * PI / sides);
            points[i * 2] = centerX + offsetX + cos(angle) * radius;
            points[i * 2 + 1] = centerY + sin(angle) * radius;
        }
        
        // Draw filled polygon (as triangles from center)
        uint16_t color = M5.Display.color565(100 + j * 50, 200 - j * 50, 150);
        for (int i = 0; i < sides; i++) {
            int next = (i + 1) % sides;
            M5.Display.fillTriangle(centerX + offsetX, centerY,
                              points[i * 2], points[i * 2 + 1],
                              points[next * 2], points[next * 2 + 1], color);
        }
    }
}

void demo5_ParticleSystem() {
    drawDemoTitle("Particle System");
    
    // Spawn new particles at touch or center
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life <= 0 && random(100) < 10) {
            particles[i].x = touchDetected ? touchX : M5.Display.width()/2;
            particles[i].y = touchDetected ? touchY : M5.Display.height()/2;
            particles[i].vx = (random(100) - 50) / 10.0;
            particles[i].vy = (random(100) - 80) / 10.0;
            particles[i].color = M5.Display.color565(random(128, 255), random(128, 255), random(128, 255));
            particles[i].life = random(50, 100);
            break;
        }
    }
    
    // Update and draw particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].life > 0) {
            // Physics
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].vy += 0.3; // Gravity
            particles[i].life--;
            
            // Bounce off walls
            if (particles[i].x < 0 || particles[i].x > M5.Display.width()) {
                particles[i].vx *= -0.8;
                particles[i].x = constrain(particles[i].x, 0, M5.Display.width());
            }
            if (particles[i].y > M5.Display.height() - 10) {
                particles[i].vy *= -0.7;
                particles[i].y = M5.Display.height() - 10;
                particles[i].vx *= 0.9; // Friction
            }
            
            // Draw particle with trail
            int size = map(particles[i].life, 0, 100, 1, 5);
            int alpha = map(particles[i].life, 0, 100, 50, 255);
            
            // Fade color based on life
            uint16_t r = (particles[i].color >> 11) * alpha / 255;
            uint16_t g = ((particles[i].color >> 5) & 0x3F) * alpha / 255;
            uint16_t b = (particles[i].color & 0x1F) * alpha / 255;
            uint16_t fadedColor = M5.Display.color565(r, g, b);
            
            M5.Display.fillCircle(particles[i].x, particles[i].y, size, fadedColor);
            
            // Draw trail
            if (particles[i].life > 10) {
                M5.Display.drawLine(particles[i].x, particles[i].y,
                              particles[i].x - particles[i].vx * 2,
                              particles[i].y - particles[i].vy * 2, fadedColor);
            }
        }
    }
    
    touchDetected = false;
}

void demo6_WaveEffect() {
    drawDemoTitle("Wave & Plasma Effects");
    
    // Plasma effect
    for (int y = 40; y < 140; y += 4) {
        for (int x = 10; x < 310; x += 4) {
            float value = sin(x * 0.05 + animationTime) + 
                         sin(y * 0.05 + animationTime * 1.1) +
                         sin(sqrt((x-160)*(x-160) + (y-90)*(y-90)) * 0.05 - animationTime);
            
            uint8_t color = (uint8_t)((value + 3) * 42);
            uint16_t pixelColor = M5.Display.color565(color, color/2, 255 - color);
            M5.Display.fillRect(x, y, 4, 4, pixelColor);
        }
    }
    
    // Sine wave lines
    M5.Display.drawFastHLine(0, 160, M5.Display.width(), TFT_DARKGREY);
    
    for (int x = 0; x < M5.Display.width(); x += 2) {
        int y1 = 180 + sin((x * 0.02) + animationTime) * 20;
        int y2 = 180 + sin((x * 0.03) + animationTime * 1.5) * 15;
        int y3 = 180 + sin((x * 0.01) + animationTime * 0.7) * 25;
        
        M5.Display.drawPixel(x, y1, TFT_RED);
        M5.Display.drawPixel(x, y2, TFT_GREEN);
        M5.Display.drawPixel(x, y3, TFT_BLUE);
        
        if (x > 0) {
            int prevY1 = 180 + sin(((x-2) * 0.02) + animationTime) * 20;
            int prevY2 = 180 + sin(((x-2) * 0.03) + animationTime * 1.5) * 15;
            int prevY3 = 180 + sin(((x-2) * 0.01) + animationTime * 0.7) * 25;
            
            M5.Display.drawLine(x-2, prevY1, x, y1, TFT_RED);
            M5.Display.drawLine(x-2, prevY2, x, y2, TFT_GREEN);
            M5.Display.drawLine(x-2, prevY3, x, y3, TFT_BLUE);
        }
    }
    
    // Lissajous curve
    int centerX = M5.Display.width()/2;
    int centerY = 180;
    
    for (float t = 0; t < 2 * PI; t += 0.05) {
        int x = centerX + sin(3 * t + animationTime) * 50;
        int y = centerY + sin(2 * t) * 30;
        M5.Display.fillCircle(x, y, 2, TFT_CYAN);
    }
}

void drawDemoTitle(const char* title) {
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.drawString(title, M5.Display.width()/2, 10);
    
    // Progress bar
    int progress = map(millis() - lastDemoChange, 0, demoInterval, 0, M5.Display.width() - 20);
    M5.Display.drawRect(10, 30, M5.Display.width() - 20, 5, TFT_DARKGREY);
    M5.Display.fillRect(10, 30, progress, 5, TFT_GREEN);
    
    // Demo counter
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TR_DATUM);
    M5.Display.drawString(String(currentDemo + 1) + "/" + String(totalDemos), M5.Display.width() - 10, 10);
}