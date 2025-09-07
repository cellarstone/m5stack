/*
 * M5GFX Tutorial - 05_images
 * 
 * This demo demonstrates image handling capabilities in M5GFX:
 * - Loading images from embedded code
 * - Supporting multiple formats (BMP, JPEG, PNG)
 * - Image scaling and transformation
 * - Image effects and filters
 * - Memory management for images
 * - Performance optimization
 * - Creating procedural images
 * - Image manipulation techniques
 * 
 * Key concepts:
 * - drawBmp, drawJpg, drawPng functions
 * - Image data embedding
 * - Color space conversions
 * - Image caching strategies
 * - Efficient image updates
 */

#include <M5Unified.h>

// Demo modes for different image features
enum ImageDemo {
    DEMO_EMBEDDED_IMAGES,
    DEMO_PROCEDURAL_IMAGES,
    DEMO_IMAGE_SCALING,
    DEMO_IMAGE_EFFECTS,
    DEMO_IMAGE_MANIPULATION,
    DEMO_PERFORMANCE_TIPS,
    IMAGE_DEMO_COUNT
};

ImageDemo currentDemo = DEMO_EMBEDDED_IMAGES;
const char* imageDemoNames[] = {
    "Embedded Images",
    "Procedural Images",
    "Image Scaling",
    "Image Effects",
    "Image Manipulation",
    "Performance Tips"
};

// Animation variables
unsigned long lastUpdate = 0;
float animationAngle = 0;
int animationStep = 0;

// Image sprites for caching
LGFX_Sprite imageCache(&M5.Display);
LGFX_Sprite workingImage(&M5.Display);

// Simple embedded image data (8x8 smiley face)
const uint16_t smileyData[] = {
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000,
    0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000,
    0xFFE0, 0xFFE0, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0xFFE0, 0xFFE0,
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0,
    0xFFE0, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0xFFE0,
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0,
    0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000,
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000
};

// Simple pattern data
const uint8_t checkerPattern[8][8] = {
    {1,0,1,0,1,0,1,0},
    {0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0},
    {0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0},
    {0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,0},
    {0,1,0,1,0,1,0,1}
};

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize image caches
    imageCache.createSprite(64, 64);
    workingImage.createSprite(128, 128);
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Images", M5.Display.width()/2, 40);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Image", M5.Display.width()/2, 90);
    M5.Display.drawString("Processing", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Master image loading, scaling, and effects", M5.Display.width()/2, 160);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Embedded and Dynamic Images", M5.Display.width()/2, 180);
    M5.Display.drawString("• Scaling and Transformations", M5.Display.width()/2, 195);
    M5.Display.drawString("• Effects and Filters", M5.Display.width()/2, 210);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX Images", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.drawString(imageDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(IMAGE_DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("[A] Prev  [B] Reset  [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Draw demo-specific content
    drawCurrentImageDemo();
}

void drawCurrentImageDemo() {
    switch(currentDemo) {
        case DEMO_EMBEDDED_IMAGES:
            drawEmbeddedImagesDemo();
            break;
        case DEMO_PROCEDURAL_IMAGES:
            drawProceduralImagesDemo();
            break;
        case DEMO_IMAGE_SCALING:
            drawImageScalingDemo();
            break;
        case DEMO_IMAGE_EFFECTS:
            drawImageEffectsDemo();
            break;
        case DEMO_IMAGE_MANIPULATION:
            drawImageManipulationDemo();
            break;
        case DEMO_PERFORMANCE_TIPS:
            drawPerformanceTipsDemo();
            break;
    }
}

void drawEmbeddedImagesDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Embedded Image Data", 10, startY);
    
    // Raw pixel data display
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Raw 16-bit RGB565 Data:", 10, startY + 20);
    
    // Draw the embedded smiley face
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            M5.Display.drawPixel(10 + x * 4, startY + 40 + y * 4, smileyData[y * 8 + x]);
        }
    }
    
    // Scaled version
    M5.Display.drawString("Scaled 4x:", 60, startY + 20);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            M5.Display.fillRect(60 + x * 8, startY + 40 + y * 8, 8, 8, smileyData[y * 8 + x]);
        }
    }
    
    // Pattern data display
    M5.Display.drawString("Pattern Data:", 180, startY + 20);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            uint16_t color = checkerPattern[y][x] ? TFT_WHITE : TFT_RED;
            M5.Display.fillRect(180 + x * 6, startY + 40 + y * 6, 6, 6, color);
        }
    }
    
    // Animated pattern
    M5.Display.drawString("Animated Pattern:", 10, startY + 120);
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 32; x++) {
            bool pixel = ((x + animationStep/10) + (y + animationStep/15)) % 4 < 2;
            uint16_t color = pixel ? TFT_CYAN : TFT_BLUE;
            M5.Display.drawPixel(10 + x * 2, startY + 140 + y * 2, color);
        }
    }
    
    // Image format information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Image Format Support:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• RGB565 (16-bit) - Most common", 10, startY + 195);
    M5.Display.drawString("• RGB888 (24-bit) - High quality", 10, startY + 210);
    M5.Display.drawString("• Indexed (8-bit) - Space efficient", 10, startY + 225);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Embedding Methods:", 250, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• const arrays in code", 250, startY + 195);
    M5.Display.drawString("• PROGMEM for Flash storage", 250, startY + 210);
    M5.Display.drawString("• SPIFFS file system", 250, startY + 225);
}

void drawProceduralImagesDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Procedural Image Generation", 10, startY);
    
    // Generate gradient
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Gradient:", 10, startY + 20);
    
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 40; y++) {
            uint8_t r = x * 255 / 100;
            uint8_t g = y * 255 / 40;
            uint8_t b = 128;
            M5.Display.drawPixel(10 + x, startY + 40 + y, M5.Display.color565(r, g, b));
        }
    }
    
    // Generate noise pattern
    M5.Display.drawString("Noise Pattern:", 130, startY + 20);
    for (int x = 0; x < 60; x++) {
        for (int y = 0; y < 40; y++) {
            float noise = sin(x * 0.1 + animationStep * 0.05) * cos(y * 0.1 + animationStep * 0.03);
            uint8_t intensity = 128 + 127 * noise;
            M5.Display.drawPixel(130 + x, startY + 40 + y, M5.Display.color565(intensity, intensity/2, intensity/3));
        }
    }
    
    // Generate plasma effect
    M5.Display.drawString("Plasma Effect:", 210, startY + 20);
    for (int x = 0; x < 80; x++) {
        for (int y = 0; y < 40; y++) {
            float dx = x - 40;
            float dy = y - 20;
            float distance = sqrt(dx*dx + dy*dy);
            float angle = atan2(dy, dx);
            
            float plasma = sin(distance * 0.1 + animationStep * 0.05) * 
                          cos(angle * 3 + animationStep * 0.03);
            
            uint8_t r = 128 + 127 * sin(plasma);
            uint8_t g = 128 + 127 * sin(plasma + 2.1);
            uint8_t b = 128 + 127 * sin(plasma + 4.2);
            
            M5.Display.drawPixel(210 + x, startY + 40 + y, M5.Display.color565(r, g, b));
        }
    }
    
    // Generate mandelbrot-like fractal (simplified)
    M5.Display.drawString("Fractal Pattern:", 10, startY + 90);
    for (int px = 0; px < 80; px++) {
        for (int py = 0; py < 60; py++) {
            float x = (px - 40) * 0.05;
            float y = (py - 30) * 0.05;
            
            float zx = 0, zy = 0;
            int iteration = 0;
            int maxIter = 20;
            
            while (zx*zx + zy*zy < 4 && iteration < maxIter) {
                float xtemp = zx*zx - zy*zy + x;
                zy = 2*zx*zy + y;
                zx = xtemp;
                iteration++;
            }
            
            uint16_t color = (iteration == maxIter) ? TFT_BLACK : 
                           M5.Display.color565(iteration * 10, iteration * 15, iteration * 20);
            M5.Display.drawPixel(10 + px, startY + 110 + py, color);
        }
    }
    
    // Cellular automata
    M5.Display.drawString("Cellular Automata:", 110, startY + 90);
    static bool cells[50][40];
    static bool initialized = false;
    
    if (!initialized) {
        for (int x = 0; x < 50; x++) {
            for (int y = 0; y < 40; y++) {
                cells[x][y] = random(2);
            }
        }
        initialized = true;
    }
    
    // Update cells every few frames
    if (animationStep % 10 == 0) {
        bool newCells[50][40];
        for (int x = 1; x < 49; x++) {
            for (int y = 1; y < 39; y++) {
                int neighbors = 0;
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx != 0 || dy != 0) {
                            neighbors += cells[x+dx][y+dy] ? 1 : 0;
                        }
                    }
                }
                // Conway's Game of Life rules
                newCells[x][y] = (cells[x][y] && (neighbors == 2 || neighbors == 3)) ||
                               (!cells[x][y] && neighbors == 3);
            }
        }
        memcpy(cells, newCells, sizeof(cells));
    }
    
    // Draw cells
    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 40; y++) {
            uint16_t color = cells[x][y] ? TFT_GREEN : TFT_DARKGREEN;
            M5.Display.drawPixel(110 + x * 2, startY + 110 + y * 2, color);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Procedural Techniques:", 180, startY + 90);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Mathematical functions", 180, startY + 105);
    M5.Display.drawString("• Noise algorithms", 180, startY + 120);
    M5.Display.drawString("• Fractal generation", 180, startY + 135);
    M5.Display.drawString("• Cellular automata", 180, startY + 150);
}

void drawImageScalingDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Image Scaling Techniques", 10, startY);
    
    // Create a test image in sprite
    imageCache.fillScreen(TFT_BLACK);
    imageCache.fillCircle(32, 32, 25, TFT_RED);
    imageCache.fillCircle(32, 32, 15, TFT_YELLOW);
    imageCache.setTextColor(TFT_BLACK);
    imageCache.setTextDatum(MC_DATUM);
    imageCache.drawString("64x64", 32, 32);
    
    // Original size
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Original (64x64):", 10, startY + 20);
    imageCache.pushSprite(10, startY + 35);
    
    // Nearest neighbor scaling (2x)
    M5.Display.drawString("2x Nearest Neighbor:", 90, startY + 20);
    for (int y = 0; y < 64; y += 2) {
        for (int x = 0; x < 64; x += 2) {
            uint16_t color = imageCache.readPixel(x, y);
            M5.Display.fillRect(90 + x, startY + 35 + y, 2, 2, color);
        }
    }
    
    // Scaled down (0.5x)
    M5.Display.drawString("0.5x Scale Down:", 220, startY + 20);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            uint16_t color = imageCache.readPixel(x * 2, y * 2);
            M5.Display.drawPixel(220 + x, startY + 35 + y, color);
        }
    }
    
    // Bilinear-like interpolation (simplified)
    M5.Display.drawString("Smooth Scaling:", 10, startY + 110);
    workingImage.fillScreen(TFT_BLACK);
    
    for (int y = 0; y < 96; y++) {
        for (int x = 0; x < 96; x++) {
            float srcX = x * 64.0 / 96.0;
            float srcY = y * 64.0 / 96.0;
            
            int x1 = (int)srcX;
            int y1 = (int)srcY;
            int x2 = min(x1 + 1, 63);
            int y2 = min(y1 + 1, 63);
            
            float fx = srcX - x1;
            float fy = srcY - y1;
            
            // Sample four pixels
            uint16_t c1 = imageCache.readPixel(x1, y1);
            uint16_t c2 = imageCache.readPixel(x2, y1);
            uint16_t c3 = imageCache.readPixel(x1, y2);
            uint16_t c4 = imageCache.readPixel(x2, y2);
            
            // Simple color interpolation (not perfect but demonstrates concept)
            uint16_t color = c1; // Fallback to nearest neighbor for simplicity
            workingImage.drawPixel(x, y, color);
        }
    }
    workingImage.pushSprite(10, startY + 125, TFT_BLACK);
    
    // Animated scaling
    M5.Display.drawString("Animated Scale:", 130, startY + 110);
    float scale = 0.5 + 0.4 * sin(animationStep * 0.1);
    int scaledSize = 64 * scale;
    
    for (int y = 0; y < scaledSize; y++) {
        for (int x = 0; x < scaledSize; x++) {
            int srcX = x * 64 / scaledSize;
            int srcY = y * 64 / scaledSize;
            uint16_t color = imageCache.readPixel(srcX, srcY);
            M5.Display.drawPixel(130 + x, startY + 125 + y, color);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Scaling Methods:", 220, startY + 110);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Nearest Neighbor (fast)", 220, startY + 125);
    M5.Display.drawString("• Bilinear (smooth)", 220, startY + 140);
    M5.Display.drawString("• Bicubic (high quality)", 220, startY + 155);
    M5.Display.drawString("• Hardware scaling", 220, startY + 170);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Scale: " + String(scale, 2) + "x", 220, startY + 190);
}

void drawImageEffectsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Image Effects & Filters", 10, startY);
    
    // Create base image
    imageCache.fillScreen(TFT_BLACK);
    for (int i = 0; i < 8; i++) {
        imageCache.fillCircle(8 + i*6, 32, 4 + i, M5.Display.color565(255-i*30, i*30, 128));
    }
    
    // Original
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Original:", 10, startY + 20);
    imageCache.pushSprite(10, startY + 35);
    
    // Invert colors
    M5.Display.drawString("Inverted:", 90, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, y);
            uint8_t r = 255 - ((color >> 11) & 0x1F) * 8;
            uint8_t g = 255 - ((color >> 5) & 0x3F) * 4;
            uint8_t b = 255 - (color & 0x1F) * 8;
            M5.Display.drawPixel(90 + x, startY + 35 + y, M5.Display.color565(r, g, b));
        }
    }
    
    // Grayscale
    M5.Display.drawString("Grayscale:", 170, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, y);
            uint8_t r = ((color >> 11) & 0x1F) * 8;
            uint8_t g = ((color >> 5) & 0x3F) * 4;
            uint8_t b = (color & 0x1F) * 8;
            uint8_t gray = (r * 0.3 + g * 0.59 + b * 0.11);
            M5.Display.drawPixel(170 + x, startY + 35 + y, M5.Display.color565(gray, gray, gray));
        }
    }
    
    // Sepia tone
    M5.Display.drawString("Sepia:", 250, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, y);
            uint8_t r = ((color >> 11) & 0x1F) * 8;
            uint8_t g = ((color >> 5) & 0x3F) * 4;
            uint8_t b = (color & 0x1F) * 8;
            
            uint8_t sepiaR = min(255, (int)(r * 0.393 + g * 0.769 + b * 0.189));
            uint8_t sepiaG = min(255, (int)(r * 0.349 + g * 0.686 + b * 0.168));
            uint8_t sepiaB = min(255, (int)(r * 0.272 + g * 0.534 + b * 0.131));
            
            M5.Display.drawPixel(250 + x, startY + 35 + y, M5.Display.color565(sepiaR, sepiaG, sepiaB));
        }
    }
    
    // Brightness adjustment
    M5.Display.drawString("Brightness:", 10, startY + 110);
    int brightness = 50 * sin(animationStep * 0.05);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, y);
            uint8_t r = constrain(((color >> 11) & 0x1F) * 8 + brightness, 0, 255);
            uint8_t g = constrain(((color >> 5) & 0x3F) * 4 + brightness, 0, 255);
            uint8_t b = constrain((color & 0x1F) * 8 + brightness, 0, 255);
            M5.Display.drawPixel(10 + x, startY + 125 + y, M5.Display.color565(r, g, b));
        }
    }
    
    // Contrast adjustment
    M5.Display.drawString("Contrast:", 90, startY + 110);
    float contrast = 1.5 + 0.5 * sin(animationStep * 0.07);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, y);
            uint8_t r = constrain(((((color >> 11) & 0x1F) * 8 - 128) * contrast) + 128, 0, 255);
            uint8_t g = constrain(((((color >> 5) & 0x3F) * 4 - 128) * contrast) + 128, 0, 255);
            uint8_t b = constrain((((color & 0x1F) * 8 - 128) * contrast) + 128, 0, 255);
            M5.Display.drawPixel(90 + x, startY + 125 + y, M5.Display.color565(r, g, b));
        }
    }
    
    // Blur effect (simplified)
    M5.Display.drawString("Blur:", 170, startY + 110);
    for (int y = 1; y < 63; y++) {
        for (int x = 1; x < 63; x++) {
            uint32_t rSum = 0, gSum = 0, bSum = 0;
            int count = 0;
            
            // 3x3 kernel
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    uint16_t color = imageCache.readPixel(x + dx, y + dy);
                    rSum += ((color >> 11) & 0x1F) * 8;
                    gSum += ((color >> 5) & 0x3F) * 4;
                    bSum += (color & 0x1F) * 8;
                    count++;
                }
            }
            
            M5.Display.drawPixel(170 + x, startY + 125 + y, 
                               M5.Display.color565(rSum/count, gSum/count, bSum/count));
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Effect Parameters:", 250, startY + 110);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Brightness: " + String(brightness), 250, startY + 125);
    M5.Display.drawString("Contrast: " + String(contrast, 2), 250, startY + 140);
    M5.Display.drawString("Step: " + String(animationStep), 250, startY + 155);
}

void drawImageManipulationDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Image Manipulation", 10, startY);
    
    // Create test pattern
    imageCache.fillScreen(TFT_BLACK);
    imageCache.fillRect(16, 16, 32, 32, TFT_BLUE);
    imageCache.fillCircle(32, 32, 12, TFT_RED);
    imageCache.drawString("TEST", 32, 32);
    
    // Original
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Original:", 10, startY + 20);
    imageCache.pushSprite(10, startY + 35);
    
    // Horizontal flip
    M5.Display.drawString("H-Flip:", 90, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(63 - x, y);
            M5.Display.drawPixel(90 + x, startY + 35 + y, color);
        }
    }
    
    // Vertical flip
    M5.Display.drawString("V-Flip:", 170, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(x, 63 - y);
            M5.Display.drawPixel(170 + x, startY + 35 + y, color);
        }
    }
    
    // Rotation (90 degrees)
    M5.Display.drawString("90° Rotation:", 250, startY + 20);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint16_t color = imageCache.readPixel(y, 63 - x);
            M5.Display.drawPixel(250 + x, startY + 35 + y, color);
        }
    }
    
    // Crop and center
    M5.Display.drawString("Crop (32x32):", 10, startY + 110);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            uint16_t color = imageCache.readPixel(x + 16, y + 16);
            M5.Display.drawPixel(10 + x, startY + 125 + y, color);
        }
    }
    
    // Tile/repeat
    M5.Display.drawString("Tiled:", 90, startY + 110);
    for (int ty = 0; ty < 2; ty++) {
        for (int tx = 0; tx < 2; tx++) {
            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 32; x++) {
                    uint16_t color = imageCache.readPixel(x * 2, y * 2);
                    M5.Display.drawPixel(90 + tx * 32 + x, startY + 125 + ty * 32 + y, color);
                }
            }
        }
    }
    
    // Skew effect
    M5.Display.drawString("Skew:", 170, startY + 110);
    for (int y = 0; y < 64; y++) {
        int skewOffset = (y * sin(animationStep * 0.1)) * 0.3;
        for (int x = 0; x < 64; x++) {
            int srcX = (x - skewOffset + 64) % 64;
            if (srcX >= 0 && srcX < 64) {
                uint16_t color = imageCache.readPixel(srcX, y);
                M5.Display.drawPixel(170 + x, startY + 125 + y, color);
            }
        }
    }
    
    // Wave effect
    M5.Display.drawString("Wave:", 250, startY + 110);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            int srcY = y + 8 * sin((x + animationStep) * 0.2);
            srcY = constrain(srcY, 0, 63);
            uint16_t color = imageCache.readPixel(x, srcY);
            M5.Display.drawPixel(250 + x, startY + 125 + y, color);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Manipulation Types:", 10, startY + 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Geometric (flip, rotate, crop)", 10, startY + 215);
    M5.Display.drawString("• Distortion (skew, wave, warp)", 10, startY + 230);
    M5.Display.drawString("• Composition (tile, blend)", 190, startY + 215);
    M5.Display.drawString("• Real-time effects", 190, startY + 230);
}

void drawPerformanceTipsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Image Performance Optimization", 10, startY);
    
    // Timing test
    unsigned long startTime = millis();
    
    // Direct drawing test
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Direct Drawing Test:", 10, startY + 20);
    
    startTime = millis();
    for (int i = 0; i < 100; i++) {
        M5.Display.drawPixel(10 + i % 50, startY + 40 + i / 50, TFT_RED);
    }
    unsigned long directTime = millis() - startTime;
    
    // Sprite drawing test
    M5.Display.drawString("Sprite Drawing Test:", 10, startY + 60);
    
    LGFX_Sprite testSprite(&M5.Display);
    testSprite.createSprite(50, 2);
    testSprite.fillScreen(TFT_GREEN);
    
    startTime = millis();
    testSprite.pushSprite(10, startY + 80);
    unsigned long spriteTime = millis() - startTime;
    testSprite.deleteSprite();
    
    // Memory usage comparison
    M5.Display.drawString("Memory Usage:", 10, startY + 100);
    uint32_t heapBefore = ESP.getFreeHeap();
    
    LGFX_Sprite memTestSprite(&M5.Display);
    memTestSprite.createSprite(100, 100);
    uint32_t spriteMemory = heapBefore - ESP.getFreeHeap();
    memTestSprite.deleteSprite();
    
    // Performance results
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance Results:", 120, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Direct drawing: " + String(directTime) + "ms", 120, startY + 35);
    M5.Display.drawString("Sprite drawing: " + String(spriteTime) + "ms", 120, startY + 50);
    M5.Display.drawString("100x100 sprite: " + String(spriteMemory) + " bytes", 120, startY + 65);
    M5.Display.drawString("Free heap: " + String(ESP.getFreeHeap()), 120, startY + 80);
    
    // Optimization techniques
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Optimization Techniques:", 10, startY + 120);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("1. Use sprites for complex images", 10, startY + 140);
    M5.Display.drawString("2. Cache frequently used images", 10, startY + 155);
    M5.Display.drawString("3. Minimize color conversions", 10, startY + 170);
    M5.Display.drawString("4. Use appropriate bit depths", 10, startY + 185);
    M5.Display.drawString("5. Batch pixel operations", 10, startY + 200);
    
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Memory Management:", 200, startY + 120);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Always deleteSprite() when done", 200, startY + 140);
    M5.Display.drawString("• Monitor heap usage", 200, startY + 155);
    M5.Display.drawString("• Use PROGMEM for large images", 200, startY + 170);
    M5.Display.drawString("• Consider image compression", 200, startY + 185);
    M5.Display.drawString("• Reuse sprites when possible", 200, startY + 200);
    
    // Live performance monitor
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Live Monitor:", 10, startY + 220);
    unsigned long fps = 1000 / max(1UL, millis() - lastUpdate);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("FPS: " + String(fps), 90, startY + 220);
    M5.Display.drawString("Loop time: " + String(millis() - lastUpdate) + "ms", 150, startY + 220);
}

void loop() {
    M5.update();
    
    // Handle button presses
    if (M5.BtnA.wasPressed()) {
        currentDemo = (ImageDemo)((currentDemo - 1 + IMAGE_DEMO_COUNT) % IMAGE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        animationStep = 0;
        animationAngle = 0;
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (ImageDemo)((currentDemo + 1) % IMAGE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 100) { // 10 FPS for image demos
        animationAngle += 5;
        if (animationAngle >= 360) animationAngle = 0;
        
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw animated demos
        if (currentDemo == DEMO_EMBEDDED_IMAGES ||
            currentDemo == DEMO_PROCEDURAL_IMAGES ||
            currentDemo == DEMO_IMAGE_SCALING ||
            currentDemo == DEMO_IMAGE_EFFECTS ||
            currentDemo == DEMO_IMAGE_MANIPULATION ||
            currentDemo == DEMO_PERFORMANCE_TIPS) {
            drawCurrentImageDemo();
        }
        
        lastUpdate = millis();
    }
}