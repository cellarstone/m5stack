/*
 * M5GFX Tutorial - 04_sprites
 * 
 * This demo demonstrates advanced sprite operations in M5GFX:
 * - Creating and managing sprites
 * - Double buffering techniques
 * - Sprite rotation and scaling
 * - Transparency and alpha blending
 * - Sprite animation and movement
 * - Memory management for sprites
 * - Performance optimization
 * - Collision detection basics
 * 
 * Key concepts:
 * - LGFX_Sprite class usage
 * - Memory allocation for sprites
 * - Push/pop sprite operations
 * - Coordinate transformations
 * - Efficient sprite updates
 */

#include <M5Unified.h>

// Forward declarations
void initializeSprites();
void initializeObjects();
void displayWelcome();
void displayCurrentDemo();
void drawTouchButtons();
void drawCurrentSpriteDemo();
void drawBasicSpritesDemo();
void drawDoubleBufferDemo();
void drawRotationScalingDemo();
void drawTransparencyDemo();
void drawSpriteAnimationDemo();
void drawCollisionDetectionDemo();
void updateAnimatedObjects();

// Demo modes for different sprite features
enum SpriteDemo {
    DEMO_BASIC_SPRITES,
    DEMO_DOUBLE_BUFFER,
    DEMO_ROTATION_SCALING,
    DEMO_TRANSPARENCY,
    DEMO_SPRITE_ANIMATION,
    DEMO_COLLISION_DETECTION,
    SPRITE_DEMO_COUNT
};

SpriteDemo currentDemo = DEMO_BASIC_SPRITES;
const char* spriteDemoNames[] = {
    "Basic Sprites",
    "Double Buffering",
    "Rotation & Scaling",
    "Transparency",
    "Sprite Animation",
    "Collision Detection"
};

// Sprite objects
LGFX_Sprite basicSprite(&M5.Display);
LGFX_Sprite bufferSprite(&M5.Display);
LGFX_Sprite ballSprite(&M5.Display);
LGFX_Sprite playerSprite(&M5.Display);

// Animation variables
unsigned long lastUpdate = 0;
float animationAngle = 0;
int animationStep = 0;

// Animation objects
struct AnimatedObject {
    float x, y;
    float vx, vy;
    float angle;
    float scale;
    uint16_t color;
    bool active;
};

AnimatedObject objects[10];
int objectCount = 0;

// Double buffer variables
bool useDoubleBuffer = false;
unsigned long frameStartTime = 0;
unsigned long frameCount = 0;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize sprites
    initializeSprites();
    
    // Initialize animated objects
    initializeObjects();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void initializeSprites() {
    // Basic sprite - 160x160 pixels (scaled for 1280x720)
    basicSprite.createSprite(160, 160);
    basicSprite.fillScreen(TFT_BLACK);
    basicSprite.setTextColor(TFT_WHITE);
    basicSprite.setTextDatum(MC_DATUM);
    basicSprite.setTextSize(3);
    basicSprite.drawString("SPRITE", 80, 80);
    for (int i = 0; i < 3; i++) {
        basicSprite.drawRect(i, i, 160-i*2, 160-i*2, TFT_CYAN);
    }
    
    // Buffer sprite for double buffering - full screen size
    bufferSprite.createSprite(M5.Display.width(), M5.Display.height());
    
    // Ball sprite - 80x80 pixels (scaled for 1280x720)
    ballSprite.createSprite(80, 80);
    ballSprite.fillScreen(TFT_BLACK);
    ballSprite.fillCircle(40, 40, 35, TFT_RED);
    for (int i = 0; i < 3; i++) {
        ballSprite.drawCircle(40, 40, 35 + i, TFT_WHITE);
    }
    
    // Player sprite - 120x80 pixels (scaled for 1280x720)
    playerSprite.createSprite(120, 80);
    playerSprite.fillScreen(TFT_BLACK);
    // Draw simple spaceship (scaled up)
    playerSprite.fillTriangle(100, 40, 20, 10, 20, 70, TFT_BLUE);
    playerSprite.fillRect(20, 30, 40, 20, TFT_CYAN);
    playerSprite.fillRect(0, 35, 20, 10, TFT_RED);
}

void initializeObjects() {
    objectCount = 8;
    for (int i = 0; i < objectCount; i++) {
        objects[i].x = random(50, M5.Display.width() - 50);
        objects[i].y = random(100, M5.Display.height() - 100);
        objects[i].vx = (random(100) - 50) / 10.0;
        objects[i].vy = (random(100) - 50) / 10.0;
        objects[i].angle = random(360);
        objects[i].scale = 0.5 + random(100) / 100.0;
        objects[i].color = M5.Display.color565(random(255), random(255), random(255));
        objects[i].active = true;
    }
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(6);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Sprites", M5.Display.width()/2, 150);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Advanced", M5.Display.width()/2, 280);
    M5.Display.drawString("Graphics", M5.Display.width()/2, 350);
    
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Learn sprite management and optimization", M5.Display.width()/2, 450);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Sprite Creation and Management", M5.Display.width()/2, 510);
    M5.Display.drawString("• Double Buffering Techniques", M5.Display.width()/2, 560);
    M5.Display.drawString("• Animation and Effects", M5.Display.width()/2, 610);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header - scaled for 1280x720
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(4);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Sprites", M5.Display.width()/2, 30);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(3);
    M5.Display.drawString(spriteDemoNames[currentDemo], M5.Display.width()/2, 90);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(SPRITE_DEMO_COUNT), M5.Display.width()/2, 130);
    
    // Draw touch buttons
    drawTouchButtons();
    
    // Draw demo-specific content
    drawCurrentSpriteDemo();
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
    
    // Reset/Toggle button
    M5.Display.fillRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_NAVY);
    M5.Display.drawRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_BLUE);
    if (currentDemo == DEMO_DOUBLE_BUFFER) {
        M5.Display.drawString("TOGGLE", startX + btnWidth + spacing + btnWidth/2, btnY + btnHeight/2);
    } else {
        M5.Display.drawString("RESET", startX + btnWidth + spacing + btnWidth/2, btnY + btnHeight/2);
    }
    
    // Next button
    M5.Display.fillRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_DARKGREEN);
    M5.Display.drawRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_GREEN);
    M5.Display.drawString("NEXT >", startX + (btnWidth + spacing) * 2 + btnWidth/2, btnY + btnHeight/2);
}

void drawCurrentSpriteDemo() {
    switch(currentDemo) {
        case DEMO_BASIC_SPRITES:
            drawBasicSpritesDemo();
            break;
        case DEMO_DOUBLE_BUFFER:
            drawDoubleBufferDemo();
            break;
        case DEMO_ROTATION_SCALING:
            drawRotationScalingDemo();
            break;
        case DEMO_TRANSPARENCY:
            drawTransparencyDemo();
            break;
        case DEMO_SPRITE_ANIMATION:
            drawSpriteAnimationDemo();
            break;
        case DEMO_COLLISION_DETECTION:
            drawCollisionDetectionDemo();
            break;
    }
}

void drawBasicSpritesDemo() {
    int startY = 180;  // Adjusted for header space
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Basic Sprite Operations", 50, startY);
    
    // Static sprite
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Static Sprite:", 50, startY + 60);
    basicSprite.pushSprite(50, startY + 100);
    
    // Sprite info
    M5.Display.drawString("Size: 160x160 pixels", 250, startY + 120);
    M5.Display.drawString("Memory: " + String(basicSprite.width() * basicSprite.height() * 2) + " bytes", 250, startY + 160);
    
    // Moving sprite - use full screen width
    M5.Display.drawString("Moving Sprite:", 50, startY + 280);
    int moveX = 50 + (animationStep * 8) % (M5.Display.width() - 210);
    basicSprite.pushSprite(moveX, startY + 320);
    
    // Sprite with different colors - larger sprites
    M5.Display.drawString("Colored Sprites:", 650, startY + 60);
    
    // Create temporary colored sprites (larger)
    for (int i = 0; i < 4; i++) {
        LGFX_Sprite colorSprite(&M5.Display);
        colorSprite.createSprite(80, 80);
        
        uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW};
        colorSprite.fillScreen(TFT_BLACK);
        colorSprite.fillRect(5, 5, 70, 70, colors[i]);
        colorSprite.drawRect(0, 0, 80, 80, TFT_WHITE);
        colorSprite.drawRect(1, 1, 78, 78, TFT_WHITE);
        
        colorSprite.pushSprite(650 + i * 100, startY + 100);
        colorSprite.deleteSprite();
    }
    
    // Memory usage info
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Sprite Memory Management:", 650, startY + 280);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• createSprite() allocates memory", 650, startY + 320);
    M5.Display.drawString("• deleteSprite() frees memory", 650, startY + 360);
    M5.Display.drawString("• pushSprite() displays to screen", 150, startY + 245);
}

void drawDoubleBufferDemo() {
    int startY = 70;
    
    if (useDoubleBuffer) {
        // Clear buffer
        bufferSprite.fillScreen(TFT_BLACK);
        
        // Draw to buffer instead of screen
        bufferSprite.setTextColor(TFT_CYAN);
        bufferSprite.setTextSize(1);
        bufferSprite.setTextDatum(TL_DATUM);
        bufferSprite.drawString("Double Buffer Mode ON", 10, startY);
        
        bufferSprite.setTextColor(TFT_WHITE);
        bufferSprite.drawString("Smooth animation without flicker", 10, startY + 20);
        
        // Draw animated content to buffer
        for (int i = 0; i < 20; i++) {
            float angle = (animationAngle + i * 18) * PI / 180.0;
            int x = 160 + 60 * cos(angle);
            int y = startY + 80 + 40 * sin(angle);
            
            uint16_t color = M5.Display.color565(
                128 + 127 * sin(angle),
                128 + 127 * cos(angle),
                128 + 127 * sin(angle + PI/2)
            );
            
            bufferSprite.fillCircle(x, y, 5, color);
        }
        
        // Performance info
        bufferSprite.setTextColor(TFT_GREEN);
        bufferSprite.drawString("Buffer FPS: " + String(1000 / max(1UL, millis() - frameStartTime)), 10, startY + 150);
        
        // Push entire buffer to screen at once
        bufferSprite.pushSprite(0, 0);
        
    } else {
        // Direct drawing mode
        M5.Display.setTextColor(TFT_CYAN);
        M5.Display.setTextSize(1);
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.drawString("Direct Drawing Mode", 10, startY);
        
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("May show flicker with complex graphics", 10, startY + 20);
        
        // Clear animation area
        M5.Display.fillRect(100, startY + 40, 140, 100, TFT_BLACK);
        
        // Draw animated content directly
        for (int i = 0; i < 20; i++) {
            float angle = (animationAngle + i * 18) * PI / 180.0;
            int x = 160 + 60 * cos(angle);
            int y = startY + 80 + 40 * sin(angle);
            
            uint16_t color = M5.Display.color565(
                128 + 127 * sin(angle),
                128 + 127 * cos(angle),
                128 + 127 * sin(angle + PI/2)
            );
            
            M5.Display.fillCircle(x, y, 5, color);
        }
        
        // Performance info
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Direct FPS: " + String(1000 / max(1UL, millis() - frameStartTime)), 10, startY + 150);
    }
    
    // Toggle instruction
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Press [B] to toggle buffering mode", 10, startY + 170);
}

void drawRotationScalingDemo() {
    int startY = 180;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Sprite Rotation & Scaling", 50, startY);
    
    // Rotating sprite
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Rotating Sprite:", 50, startY + 60);
    
    int centerX = 200, centerY = startY + 180;
    ballSprite.setPivot(ballSprite.width()/2, ballSprite.height()/2); // Set pivot to center of sprite
    // Push rotated sprite to display
    M5.Display.setPivot(centerX, centerY);
    ballSprite.pushRotated(&M5.Display, animationAngle);
    
    // Scaling sprite
    M5.Display.drawString("Scaling Sprite:", 50, startY + 320);
    
    float scale = 0.5 + 1.0 * sin(animationAngle * PI / 180.0);
    int scaledWidth = ballSprite.width() * scale;
    int scaledHeight = ballSprite.height() * scale;
    
    // Note: pushRotateZoom combines rotation and scaling
    ballSprite.pushRotateZoom(200, startY + 420, 0, scale, scale);
    
    // Multiple transformations
    M5.Display.drawString("Combined Transform:", 600, startY + 60);
    
    for (int i = 0; i < 8; i++) {
        float angle = (animationAngle + i * 45) * PI / 180.0;
        float x = 800 + 150 * cos(angle);
        float y = startY + 250 + 150 * sin(angle);
        float rotAngle = animationAngle + i * 45;
        float scaleVal = 0.5 + 0.5 * sin((animationAngle + i * 30) * PI / 180.0);
        
        ballSprite.pushRotateZoom(x, y, rotAngle, scaleVal, scaleVal);
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Transform Functions:", 160, startY + 120);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• pushRotated() - rotation only", 160, startY + 140);
    M5.Display.drawString("• pushRotateZoom() - rotate + scale", 160, startY + 155);
    M5.Display.drawString("• setPivot() - rotation center", 160, startY + 170);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Angle: " + String((int)animationAngle) + "°", 160, startY + 190);
    M5.Display.drawString("Scale: " + String(scale, 2), 160, startY + 205);
}

void drawTransparencyDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Transparency & Alpha", 10, startY);
    
    // Create background pattern
    for (int x = 0; x < M5.Display.width(); x += 20) {
        for (int y = startY + 30; y < M5.Display.height() - 50; y += 20) {
            uint16_t color = ((x/20 + y/20) % 2) ? TFT_DARKGREY : TFT_BLACK;
            M5.Display.fillRect(x, y, 20, 20, color);
        }
    }
    
    // Transparent color key demo
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Color Key Transparency:", 10, startY + 20);
    
    LGFX_Sprite transSprite(&M5.Display);
    transSprite.createSprite(60, 40);
    transSprite.fillScreen(TFT_MAGENTA); // Transparent color
    transSprite.fillCircle(30, 20, 18, TFT_GREEN);
    transSprite.drawString("KEY", 30, 20);
    
    // Push with transparency (TFT_MAGENTA becomes transparent)
    transSprite.pushSprite(50, startY + 40, TFT_MAGENTA);
    transSprite.deleteSprite();
    
    // Alpha blending simulation
    M5.Display.drawString("Alpha Blending:", 10, startY + 90);
    
    for (int i = 0; i < 5; i++) {
        LGFX_Sprite alphaSprite(&M5.Display);
        alphaSprite.createSprite(40, 40);
        
        // Simulate alpha by mixing colors
        uint8_t alpha = 50 + i * 40; // Increasing opacity
        uint16_t blendColor = M5.Display.color565(alpha, 0, 255-alpha);
        
        alphaSprite.fillScreen(TFT_BLACK);
        alphaSprite.fillCircle(20, 20, 18, blendColor);
        alphaSprite.drawString(String(alpha), 20, 20);
        
        alphaSprite.pushSprite(50 + i * 35, startY + 110, TFT_BLACK);
        alphaSprite.deleteSprite();
    }
    
    // Overlay effects
    M5.Display.drawString("Overlay Effects:", 10, startY + 160);
    
    // Moving transparent overlay
    int overlayX = 50 + (animationStep * 2) % 200;
    LGFX_Sprite overlaySprite(&M5.Display);
    overlaySprite.createSprite(80, 30);
    overlaySprite.fillScreen(TFT_BLACK);
    overlaySprite.fillRoundRect(5, 5, 70, 20, 10, TFT_YELLOW);
    overlaySprite.setTextColor(TFT_BLACK);
    overlaySprite.setTextDatum(MC_DATUM);
    overlaySprite.drawString("OVERLAY", 40, 15);
    
    overlaySprite.pushSprite(overlayX, startY + 180, TFT_BLACK);
    overlaySprite.deleteSprite();
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Transparency Tips:", 240, startY + 30);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Use pushSprite(x,y,key)", 240, startY + 50);
    M5.Display.drawString("• Key color = transparent", 240, startY + 65);
    M5.Display.drawString("• Common: TFT_BLACK/MAGENTA", 240, startY + 80);
    M5.Display.drawString("• Alpha = color mixing", 240, startY + 95);
}

void drawSpriteAnimationDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Sprite Animation System", 10, startY);
    
    // Update animated objects
    updateAnimatedObjects();
    
    // Draw animated objects
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].active) {
            ballSprite.pushRotateZoom(
                objects[i].x, objects[i].y, 
                objects[i].angle, 
                objects[i].scale, objects[i].scale,
                objects[i].color
            );
        }
    }
    
    // Animation info panel
    M5.Display.fillRect(10, startY + 20, 200, 60, TFT_NAVY);
    M5.Display.drawRect(10, startY + 20, 200, 60, TFT_CYAN);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Active Objects: " + String(objectCount), 15, startY + 30);
    M5.Display.drawString("Animation Step: " + String(animationStep), 15, startY + 45);
    M5.Display.drawString("FPS: " + String(1000 / max(1UL, millis() - frameStartTime)), 15, startY + 60);
    
    // Control instructions
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Animation Features:", 230, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Velocity-based movement", 230, startY + 35);
    M5.Display.drawString("• Boundary collision", 230, startY + 50);
    M5.Display.drawString("• Rotation animation", 230, startY + 65);
    M5.Display.drawString("• Scale variation", 230, startY + 80);
}

void updateAnimatedObjects() {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].active) {
            // Update position
            objects[i].x += objects[i].vx;
            objects[i].y += objects[i].vy;
            
            // Update rotation
            objects[i].angle += 2;
            if (objects[i].angle >= 360) objects[i].angle = 0;
            
            // Boundary collision
            if (objects[i].x <= 16 || objects[i].x >= M5.Display.width() - 16) {
                objects[i].vx = -objects[i].vx;
                objects[i].x = constrain(objects[i].x, 16, M5.Display.width() - 16);
            }
            if (objects[i].y <= 90 || objects[i].y >= M5.Display.height() - 50) {
                objects[i].vy = -objects[i].vy;
                objects[i].y = constrain(objects[i].y, 90, M5.Display.height() - 50);
            }
            
            // Scale animation
            objects[i].scale = 0.5 + 0.3 * sin((animationStep + i * 20) * 0.1);
        }
    }
}

void drawCollisionDetectionDemo() {
    int startY = 180;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Collision Detection", 50, startY);
    
    // Player controlled sprite
    static int playerX = M5.Display.width() / 2;
    static int playerY = M5.Display.height() - 200;
    
    // Simple touch/button control
    M5.update();
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed()) {
            playerX = touch.x;
            playerY = touch.y;
        }
    }
    
    // Draw player
    playerSprite.pushSprite(playerX - 24, playerY - 16);
    
    // Draw collision targets - spread across full screen
    static int targetCount = 6;
    static float targetX[6] = {200, 400, 600, 800, 1000, 1100};
    static float targetY[6] = {300, 400, 350, 450, 320, 380};
    static bool targetActive[6] = {true, true, true, true, true, true};
    static int collisionCount = 0;
    
    for (int i = 0; i < targetCount; i++) {
        if (targetActive[i]) {
            // Move targets
            targetY[i] += sin((animationStep + i * 30) * 0.1) * 0.5;
            
            // Check collision (simple distance check)
            float dx = playerX - targetX[i];
            float dy = playerY - targetY[i];
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < 60) {  // Larger collision radius for bigger sprites
                targetActive[i] = false;
                collisionCount++;
                
                // Collision effect
                for (int j = 0; j < 8; j++) {
                    float angle = j * 45 * PI / 180;
                    int fx = targetX[i] + 20 * cos(angle);
                    int fy = targetY[i] + 20 * sin(angle);
                    M5.Display.drawPixel(fx, fy, TFT_YELLOW);
                }
            } else {
                // Draw target
                uint16_t color = (distance < 50) ? TFT_RED : TFT_GREEN;
                ballSprite.pushSprite(targetX[i] - 40, targetY[i] - 40, TFT_BLACK);
                
                // Draw collision radius indicator
                M5.Display.drawCircle(targetX[i], targetY[i], 60, TFT_DARKGREY);
            }
        }
    }
    
    // Reset targets if all collected
    if (collisionCount >= targetCount) {
        for (int i = 0; i < targetCount; i++) {
            targetActive[i] = true;
        }
        collisionCount = 0;
    }
    
    // Info panel
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Player Position: " + String(playerX) + "," + String(playerY), 50, startY + 60);
    M5.Display.drawString("Targets Collected: " + String(collisionCount) + "/" + String(targetCount), 50, startY + 100);
    
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Touch screen to move player", 50, startY + 140);
    M5.Display.drawString("Collect all green targets!", 50, startY + 180);
    
    // Collision detection info
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Collision Methods:", 700, startY + 60);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Distance-based (circles)", 700, startY + 100);
    M5.Display.drawString("• Bounding box (rectangles)", 700, startY + 140);
    M5.Display.drawString("• Pixel-perfect (advanced)", 700, startY + 180);
}

void loop() {
    M5.update();
    frameStartTime = millis();
    
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
                    currentDemo = (SpriteDemo)((currentDemo - 1 + SPRITE_DEMO_COUNT) % SPRITE_DEMO_COUNT);
                    displayCurrentDemo();
                } else if (touch.x >= startX + btnWidth + spacing && 
                          touch.x <= startX + btnWidth + spacing + btnWidth) {
                    // Reset/Toggle button
                    if (currentDemo == DEMO_DOUBLE_BUFFER) {
                        useDoubleBuffer = !useDoubleBuffer;
                    } else {
                        // Reset animation
                        animationStep = 0;
                        animationAngle = 0;
                        initializeObjects();
                    }
                    displayCurrentDemo();
                } else if (touch.x >= startX + (btnWidth + spacing) * 2 && 
                          touch.x <= startX + (btnWidth + spacing) * 2 + btnWidth) {
                    // Next button
                    currentDemo = (SpriteDemo)((currentDemo + 1) % SPRITE_DEMO_COUNT);
                    displayCurrentDemo();
                }
            }
            
            // Handle collision detection demo touch
            // Note: Touch handling for collision detection is done within drawCollisionDetectionDemo()
        }
    }
    
    // Also handle physical buttons if available
    if (M5.BtnA.wasPressed()) {
        currentDemo = (SpriteDemo)((currentDemo - 1 + SPRITE_DEMO_COUNT) % SPRITE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        if (currentDemo == DEMO_DOUBLE_BUFFER) {
            useDoubleBuffer = !useDoubleBuffer;
        } else {
            // Reset animation
            animationStep = 0;
            animationAngle = 0;
            initializeObjects();
        }
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (SpriteDemo)((currentDemo + 1) % SPRITE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS
        animationAngle += 3;
        if (animationAngle >= 360) animationAngle = 0;
        
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw animated demos
        if (currentDemo == DEMO_DOUBLE_BUFFER || 
            currentDemo == DEMO_ROTATION_SCALING || 
            currentDemo == DEMO_TRANSPARENCY ||
            currentDemo == DEMO_SPRITE_ANIMATION ||
            currentDemo == DEMO_COLLISION_DETECTION) {
            drawCurrentSpriteDemo();
        }
        
        lastUpdate = millis();
    }
}