/*
 * M5GFX Tutorial - 09_performance
 * 
 * This demo demonstrates performance optimization techniques in M5GFX:
 * - startWrite/endWrite usage for batch operations
 * - Dirty rectangles for selective updates
 * - Efficient screen clearing strategies
 * - FPS measurement and monitoring
 * - Buffer management optimization
 * - Memory usage optimization
 * - Drawing operation profiling
 * - Real-world optimization examples
 * 
 * Key concepts:
 * - Transaction-based drawing
 * - Selective screen updates
 * - Performance measurement
 * - Memory management
 * - Optimization trade-offs
 */

#include <M5Unified.h>
#include <math.h>

// Demo modes for different performance techniques
enum PerformanceDemo {
    DEMO_BATCH_OPERATIONS,
    DEMO_DIRTY_RECTANGLES,
    DEMO_BUFFER_MANAGEMENT,
    DEMO_FPS_OPTIMIZATION,
    DEMO_MEMORY_MANAGEMENT,
    DEMO_PROFILING_TOOLS,
    PERFORMANCE_DEMO_COUNT
};

PerformanceDemo currentDemo = DEMO_BATCH_OPERATIONS;
const char* performanceDemoNames[] = {
    "Batch Operations",
    "Dirty Rectangles",
    "Buffer Management",
    "FPS Optimization",
    "Memory Management",
    "Profiling Tools"
};

// Performance measurement
struct PerformanceStats {
    unsigned long frameStartTime;
    unsigned long drawTime;
    unsigned long updateTime;
    unsigned long totalTime;
    float fps;
    float avgFps;
    int frameCount;
    unsigned long lastSecond;
    int framesThisSecond;
    uint32_t freeHeap;
    uint32_t minFreeHeap;
    uint32_t maxUsedHeap;
};

PerformanceStats stats;

// Dirty rectangle system
struct DirtyRect {
    int x, y, width, height;
    bool active;
};

const int MAX_DIRTY_RECTS = 10;
DirtyRect dirtyRects[MAX_DIRTY_RECTS];
int dirtyRectCount = 0;

// Test objects for performance testing
struct TestObject {
    float x, y;
    float vx, vy;
    uint16_t color;
    int size;
    bool active;
    bool needsUpdate;
};

const int MAX_TEST_OBJECTS = 100;
TestObject testObjects[MAX_TEST_OBJECTS];

// Buffer management
LGFX_Sprite performanceBuffer(&M5.Display);
bool useBuffer = false;

// Animation variables
unsigned long lastUpdate = 0;
int animationStep = 0;
bool measurementMode = false;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize performance measurement
    initPerformanceStats();
    
    // Initialize test objects
    initTestObjects();
    
    // Initialize dirty rectangle system
    initDirtyRectSystem();
    
    // Create performance buffer
    performanceBuffer.createSprite(M5.Display.width(), M5.Display.height());
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void initPerformanceStats() {
    stats.frameStartTime = 0;
    stats.drawTime = 0;
    stats.updateTime = 0;
    stats.totalTime = 0;
    stats.fps = 0;
    stats.avgFps = 0;
    stats.frameCount = 0;
    stats.lastSecond = millis();
    stats.framesThisSecond = 0;
    stats.freeHeap = ESP.getFreeHeap();
    stats.minFreeHeap = stats.freeHeap;
    stats.maxUsedHeap = 0;
}

void initTestObjects() {
    for (int i = 0; i < MAX_TEST_OBJECTS; i++) {
        testObjects[i].x = random(M5.Display.width());
        testObjects[i].y = random(100, M5.Display.height() - 50);
        testObjects[i].vx = (random(200) - 100) / 100.0;
        testObjects[i].vy = (random(200) - 100) / 100.0;
        testObjects[i].color = M5.Display.color565(random(255), random(255), random(255));
        testObjects[i].size = 2 + random(8);
        testObjects[i].active = (i < 20); // Start with 20 active objects
        testObjects[i].needsUpdate = true;
    }
}

void initDirtyRectSystem() {
    for (int i = 0; i < MAX_DIRTY_RECTS; i++) {
        dirtyRects[i].active = false;
    }
    dirtyRectCount = 0;
}

void addDirtyRect(int x, int y, int width, int height) {
    if (dirtyRectCount < MAX_DIRTY_RECTS) {
        dirtyRects[dirtyRectCount].x = x;
        dirtyRects[dirtyRectCount].y = y;
        dirtyRects[dirtyRectCount].width = width;
        dirtyRects[dirtyRectCount].height = height;
        dirtyRects[dirtyRectCount].active = true;
        dirtyRectCount++;
    }
}

void clearDirtyRects() {
    for (int i = 0; i < MAX_DIRTY_RECTS; i++) {
        dirtyRects[i].active = false;
    }
    dirtyRectCount = 0;
}

void updatePerformanceStats() {
    unsigned long currentTime = millis();
    
    // Frame timing
    stats.totalTime = currentTime - stats.frameStartTime;
    if (stats.totalTime > 0) {
        stats.fps = 1000.0 / stats.totalTime;
    }
    
    // Average FPS over 1 second
    stats.framesThisSecond++;
    if (currentTime - stats.lastSecond >= 1000) {
        stats.avgFps = stats.framesThisSecond;
        stats.framesThisSecond = 0;
        stats.lastSecond = currentTime;
    }
    
    // Memory tracking
    stats.freeHeap = ESP.getFreeHeap();
    if (stats.freeHeap < stats.minFreeHeap) {
        stats.minFreeHeap = stats.freeHeap;
    }
    uint32_t usedHeap = ESP.getHeapSize() - stats.freeHeap;
    if (usedHeap > stats.maxUsedHeap) {
        stats.maxUsedHeap = usedHeap;
    }
    
    stats.frameCount++;
    stats.frameStartTime = currentTime;
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX", M5.Display.width()/2, 40);
    M5.Display.drawString("Performance", M5.Display.width()/2, 75);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Optimization", M5.Display.width()/2, 110);
    M5.Display.drawString("Techniques", M5.Display.width()/2, 135);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Learn to optimize graphics performance", M5.Display.width()/2, 170);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Batch Operations & Dirty Rectangles", M5.Display.width()/2, 190);
    M5.Display.drawString("• Buffer Management & FPS Optimization", M5.Display.width()/2, 205);
    M5.Display.drawString("• Memory Management & Profiling", M5.Display.width()/2, 220);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Performance", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.drawString(performanceDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(PERFORMANCE_DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Performance stats
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("FPS: " + String((int)stats.avgFps) + " | Heap: " + String(stats.freeHeap), M5.Display.width()/2, 65);
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("[A] Prev  [B] Toggle  [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Draw demo-specific content
    drawCurrentPerformanceDemo();
}

void drawCurrentPerformanceDemo() {
    switch(currentDemo) {
        case DEMO_BATCH_OPERATIONS:
            drawBatchOperationsDemo();
            break;
        case DEMO_DIRTY_RECTANGLES:
            drawDirtyRectanglesDemo();
            break;
        case DEMO_BUFFER_MANAGEMENT:
            drawBufferManagementDemo();
            break;
        case DEMO_FPS_OPTIMIZATION:
            drawFPSOptimizationDemo();
            break;
        case DEMO_MEMORY_MANAGEMENT:
            drawMemoryManagementDemo();
            break;
        case DEMO_PROFILING_TOOLS:
            drawProfilingToolsDemo();
            break;
    }
}

void drawBatchOperationsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Batch Operations Optimization", 10, startY);
    
    static bool useBatching = false;
    static unsigned long batchTime = 0;
    static unsigned long individualTime = 0;
    
    // Toggle batching with B button
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mode: " + String(useBatching ? "BATCHED" : "INDIVIDUAL"), 10, startY + 20);
    M5.Display.drawString("Press [B] to toggle batching", 10, startY + 35);
    
    // Performance comparison
    unsigned long startTime = micros();
    
    if (useBatching) {
        // Batched drawing with startWrite/endWrite
        M5.Display.startWrite();
        
        for (int i = 0; i < 50; i++) {
            int x = 50 + (i % 10) * 20;
            int y = startY + 60 + (i / 10) * 20;
            uint16_t color = M5.Display.color565(i * 5, 255 - i * 5, 128);
            M5.Display.fillRect(x, y, 15, 15, color);
        }
        
        M5.Display.endWrite();
        batchTime = micros() - startTime;
        
    } else {
        // Individual drawing operations
        for (int i = 0; i < 50; i++) {
            int x = 50 + (i % 10) * 20;
            int y = startY + 60 + (i / 10) * 20;
            uint16_t color = M5.Display.color565(i * 5, 255 - i * 5, 128);
            M5.Display.fillRect(x, y, 15, 15, color);
        }
        individualTime = micros() - startTime;
    }
    
    // Performance results
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance Results:", 300, startY + 60);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Batched: " + String(batchTime) + " μs", 300, startY + 80);
    M5.Display.drawString("Individual: " + String(individualTime) + " μs", 300, startY + 100);
    
    if (batchTime > 0 && individualTime > 0) {
        float improvement = (float)individualTime / batchTime;
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Speedup: " + String(improvement, 2) + "x", 300, startY + 120);
    }
    
    // Best practices
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Batching Best Practices:", 10, startY + 170);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Use startWrite() before multiple draws", 10, startY + 185);
    M5.Display.drawString("• Call endWrite() when finished", 10, startY + 200);
    M5.Display.drawString("• Batch similar operations together", 10, startY + 215);
    M5.Display.drawString("• Avoid mixing read/write operations", 10, startY + 230);
    
    // When to use batching
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("When to Batch:", 250, startY + 170);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Drawing many small objects", 250, startY + 185);
    M5.Display.drawString("• Updating UI elements", 250, startY + 200);
    M5.Display.drawString("• Particle systems", 250, startY + 215);
    M5.Display.drawString("• Animation frames", 250, startY + 230);
    
    // Handle B button for toggling
    static bool lastBtnB = false;
    bool currentBtnB = M5.BtnB.isPressed();
    if (currentBtnB && !lastBtnB) {
        useBatching = !useBatching;
    }
    lastBtnB = currentBtnB;
}

void drawDirtyRectanglesDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Dirty Rectangle Optimization", 10, startY);
    
    static bool useDirtyRects = false;
    static int objectCount = 10;
    static unsigned long fullRedrawTime = 0;
    static unsigned long dirtyRedrawTime = 0;
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mode: " + String(useDirtyRects ? "DIRTY RECTS" : "FULL REDRAW"), 10, startY + 20);
    M5.Display.drawString("Objects: " + String(objectCount), 10, startY + 35);
    M5.Display.drawString("Press [B] to toggle mode", 10, startY + 50);
    
    // Update test objects
    for (int i = 0; i < objectCount; i++) {
        if (testObjects[i].active) {
            // Store old position for dirty rect
            int oldX = testObjects[i].x;
            int oldY = testObjects[i].y;
            
            // Update position
            testObjects[i].x += testObjects[i].vx;
            testObjects[i].y += testObjects[i].vy;
            
            // Boundary checking
            if (testObjects[i].x <= testObjects[i].size || 
                testObjects[i].x >= M5.Display.width() - testObjects[i].size) {
                testObjects[i].vx = -testObjects[i].vx;
                testObjects[i].x = constrain(testObjects[i].x, testObjects[i].size, 
                                           M5.Display.width() - testObjects[i].size);
            }
            if (testObjects[i].y <= startY + 80 + testObjects[i].size || 
                testObjects[i].y >= M5.Display.height() - 50 - testObjects[i].size) {
                testObjects[i].vy = -testObjects[i].vy;
                testObjects[i].y = constrain(testObjects[i].y, startY + 80 + testObjects[i].size, 
                                           M5.Display.height() - 50 - testObjects[i].size);
            }
            
            // Add dirty rectangles for old and new positions
            if (useDirtyRects) {
                addDirtyRect(oldX - testObjects[i].size, oldY - testObjects[i].size,
                           testObjects[i].size * 2, testObjects[i].size * 2);
                addDirtyRect(testObjects[i].x - testObjects[i].size, testObjects[i].y - testObjects[i].size,
                           testObjects[i].size * 2, testObjects[i].size * 2);
            }
        }
    }
    
    // Drawing performance comparison
    unsigned long startTime = micros();
    
    if (useDirtyRects) {
        // Only redraw dirty rectangles
        for (int i = 0; i < dirtyRectCount; i++) {
            if (dirtyRects[i].active) {
                // Clear dirty area
                M5.Display.fillRect(dirtyRects[i].x, dirtyRects[i].y,
                                   dirtyRects[i].width, dirtyRects[i].height, TFT_BLACK);
            }
        }
        
        // Draw objects in dirty areas only
        for (int i = 0; i < objectCount; i++) {
            if (testObjects[i].active) {
                M5.Display.fillCircle(testObjects[i].x, testObjects[i].y, 
                                     testObjects[i].size, testObjects[i].color);
            }
        }
        
        dirtyRedrawTime = micros() - startTime;
        clearDirtyRects();
        
    } else {
        // Full screen redraw
        M5.Display.fillRect(10, startY + 80, M5.Display.width() - 20, 
                           M5.Display.height() - startY - 130, TFT_BLACK);
        
        for (int i = 0; i < objectCount; i++) {
            if (testObjects[i].active) {
                M5.Display.fillCircle(testObjects[i].x, testObjects[i].y, 
                                     testObjects[i].size, testObjects[i].color);
            }
        }
        
        fullRedrawTime = micros() - startTime;
    }
    
    // Performance comparison
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance:", 300, startY + 80);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Full: " + String(fullRedrawTime) + " μs", 300, startY + 100);
    M5.Display.drawString("Dirty: " + String(dirtyRedrawTime) + " μs", 300, startY + 120);
    
    if (fullRedrawTime > 0 && dirtyRedrawTime > 0) {
        float improvement = (float)fullRedrawTime / dirtyRedrawTime;
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Speedup: " + String(improvement, 2) + "x", 300, startY + 140);
    }
    
    // Visualize dirty rectangles
    if (useDirtyRects) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Dirty areas shown in red", 300, startY + 160);
        for (int i = 0; i < dirtyRectCount; i++) {
            if (dirtyRects[i].active) {
                M5.Display.drawRect(dirtyRects[i].x, dirtyRects[i].y,
                                   dirtyRects[i].width, dirtyRects[i].height, TFT_RED);
            }
        }
    }
    
    // Dirty rectangle theory
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Dirty Rectangle Theory:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Only redraw changed areas", 10, startY + 195);
    M5.Display.drawString("• Track object movement", 10, startY + 210);
    M5.Display.drawString("• Merge overlapping rectangles", 10, startY + 225);
    
    // Handle B button
    static bool lastBtnB = false;
    bool currentBtnB = M5.BtnB.isPressed();
    if (currentBtnB && !lastBtnB) {
        useDirtyRects = !useDirtyRects;
    }
    lastBtnB = currentBtnB;
}

void drawBufferManagementDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Buffer Management", 10, startY);
    
    static unsigned long bufferTime = 0;
    static unsigned long directTime = 0;
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mode: " + String(useBuffer ? "BUFFERED" : "DIRECT"), 10, startY + 20);
    M5.Display.drawString("Press [B] to toggle buffering", 10, startY + 35);
    
    // Complex drawing operation for testing
    unsigned long startTime = micros();
    
    if (useBuffer) {
        // Draw to buffer first
        performanceBuffer.fillScreen(TFT_BLACK);
        
        // Complex drawing operations
        for (int i = 0; i < 30; i++) {
            int x = 50 + i * 8;
            int y = startY + 60 + 50 * sin(i * 0.3 + animationStep * 0.1);
            uint16_t color = performanceBuffer.color565(i * 8, 255 - i * 8, 128);
            performanceBuffer.fillCircle(x, y, 5, color);
            
            // Draw connecting lines
            if (i > 0) {
                int prevX = 50 + (i-1) * 8;
                int prevY = startY + 60 + 50 * sin((i-1) * 0.3 + animationStep * 0.1);
                performanceBuffer.drawLine(prevX, prevY, x, y, TFT_WHITE);
            }
        }
        
        // Additional complex shapes
        for (int i = 0; i < 10; i++) {
            int centerX = 100 + i * 30;
            int centerY = startY + 150;
            int radius = 15 + 10 * sin(i * 0.5 + animationStep * 0.05);
            
            for (int j = 0; j < 8; j++) {
                float angle = j * PI / 4;
                int x = centerX + radius * cos(angle);
                int y = centerY + radius * sin(angle);
                performanceBuffer.fillCircle(x, y, 3, TFT_CYAN);
            }
        }
        
        // Push buffer to screen
        performanceBuffer.pushSprite(0, 0);
        bufferTime = micros() - startTime;
        
    } else {
        // Draw directly to screen
        M5.Display.fillRect(10, startY + 60, M5.Display.width() - 20, 120, TFT_BLACK);
        
        // Same complex drawing operations
        for (int i = 0; i < 30; i++) {
            int x = 50 + i * 8;
            int y = startY + 60 + 50 * sin(i * 0.3 + animationStep * 0.1);
            uint16_t color = M5.Display.color565(i * 8, 255 - i * 8, 128);
            M5.Display.fillCircle(x, y, 5, color);
            
            if (i > 0) {
                int prevX = 50 + (i-1) * 8;
                int prevY = startY + 60 + 50 * sin((i-1) * 0.3 + animationStep * 0.1);
                M5.Display.drawLine(prevX, prevY, x, y, TFT_WHITE);
            }
        }
        
        for (int i = 0; i < 10; i++) {
            int centerX = 100 + i * 30;
            int centerY = startY + 150;
            int radius = 15 + 10 * sin(i * 0.5 + animationStep * 0.05);
            
            for (int j = 0; j < 8; j++) {
                float angle = j * PI / 4;
                int x = centerX + radius * cos(angle);
                int y = centerY + radius * sin(angle);
                M5.Display.fillCircle(x, y, 3, TFT_CYAN);
            }
        }
        directTime = micros() - startTime;
    }
    
    // Performance results
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance:", 10, startY + 190);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Direct: " + String(directTime) + " μs", 10, startY + 205);
    M5.Display.drawString("Buffered: " + String(bufferTime) + " μs", 10, startY + 220);
    
    // Buffer information
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Buffer Info:", 200, startY + 190);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Size: " + String(M5.Display.width()) + "x" + String(M5.Display.height()), 200, startY + 205);
    M5.Display.drawString("Memory: " + String(M5.Display.width() * M5.Display.height() * 2) + " bytes", 200, startY + 220);
    
    // Buffer advantages/disadvantages
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Buffer Pros:", 350, startY + 60);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• No flicker", 350, startY + 75);
    M5.Display.drawString("• Complex compositing", 350, startY + 90);
    M5.Display.drawString("• Consistent timing", 350, startY + 105);
    
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("Buffer Cons:", 350, startY + 125);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Memory usage", 350, startY + 140);
    M5.Display.drawString("• Copy overhead", 350, startY + 155);
    M5.Display.drawString("• RAM constraints", 350, startY + 170);
    
    // Handle B button
    static bool lastBtnB = false;
    bool currentBtnB = M5.BtnB.isPressed();
    if (currentBtnB && !lastBtnB) {
        useBuffer = !useBuffer;
    }
    lastBtnB = currentBtnB;
}

void drawFPSOptimizationDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("FPS Optimization Strategies", 10, startY);
    
    static int targetFPS = 30;
    static bool vsyncEnabled = false;
    static int activeObjects = 20;
    
    // FPS control
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Target FPS: " + String(targetFPS), 10, startY + 20);
    M5.Display.drawString("VSync: " + String(vsyncEnabled ? "ON" : "OFF"), 10, startY + 35);
    M5.Display.drawString("Objects: " + String(activeObjects), 10, startY + 50);
    
    // Adjust active objects based on performance
    if (stats.avgFps < targetFPS - 5 && activeObjects > 5) {
        activeObjects -= 2;
    } else if (stats.avgFps > targetFPS + 5 && activeObjects < MAX_TEST_OBJECTS) {
        activeObjects += 2;
    }
    
    // Enable/disable objects
    for (int i = 0; i < MAX_TEST_OBJECTS; i++) {
        testObjects[i].active = (i < activeObjects);
    }
    
    // Frame timing control
    static unsigned long lastFrameTime = 0;
    unsigned long targetFrameTime = 1000 / targetFPS;
    unsigned long currentTime = millis();
    
    if (vsyncEnabled && (currentTime - lastFrameTime) < targetFrameTime) {
        return; // Skip this frame to maintain target FPS
    }
    lastFrameTime = currentTime;
    
    // Clear and draw active objects
    M5.Display.fillRect(10, startY + 70, M5.Display.width() - 20, 100, TFT_BLACK);
    
    for (int i = 0; i < activeObjects; i++) {
        if (testObjects[i].active) {
            // Update position
            testObjects[i].x += testObjects[i].vx;
            testObjects[i].y += testObjects[i].vy;
            
            // Boundary check
            if (testObjects[i].x < 15 || testObjects[i].x > M5.Display.width() - 15) {
                testObjects[i].vx = -testObjects[i].vx;
            }
            if (testObjects[i].y < startY + 75 || testObjects[i].y > startY + 165) {
                testObjects[i].vy = -testObjects[i].vy;
            }
            
            // Draw object
            M5.Display.fillCircle(testObjects[i].x, testObjects[i].y, 
                                 testObjects[i].size, testObjects[i].color);
        }
    }
    
    // Performance metrics
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance Metrics:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Current FPS: " + String((int)stats.fps), 10, startY + 195);
    M5.Display.drawString("Average FPS: " + String((int)stats.avgFps), 10, startY + 210);
    M5.Display.drawString("Frame Time: " + String(stats.totalTime) + "ms", 10, startY + 225);
    
    // FPS graph
    static float fpsHistory[100];
    static int historyIndex = 0;
    
    fpsHistory[historyIndex] = stats.fps;
    historyIndex = (historyIndex + 1) % 100;
    
    // Draw FPS graph
    M5.Display.drawRect(250, startY + 180, 100, 60, TFT_WHITE);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("FPS Graph", 300, startY + 170);
    
    for (int i = 1; i < 100; i++) {
        int prev = (historyIndex - i + 100) % 100;
        int curr = (historyIndex - i + 1 + 100) % 100;
        
        int y1 = startY + 240 - (fpsHistory[prev] * 60 / 60); // Scale to 60 FPS max
        int y2 = startY + 240 - (fpsHistory[curr] * 60 / 60);
        
        y1 = constrain(y1, startY + 180, startY + 240);
        y2 = constrain(y2, startY + 180, startY + 240);
        
        M5.Display.drawLine(250 + 99 - i, y1, 250 + 100 - i, y2, TFT_GREEN);
    }
    
    // Target FPS line
    int targetY = startY + 240 - (targetFPS * 60 / 60);
    M5.Display.drawLine(250, targetY, 350, targetY, TFT_RED);
    
    // Optimization techniques
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Optimization Techniques:", 360, startY + 80);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Dynamic LOD (Level of Detail)", 360, startY + 95);
    M5.Display.drawString("• Adaptive object count", 360, startY + 110);
    M5.Display.drawString("• Frame rate limiting", 360, startY + 125);
    M5.Display.drawString("• Skip frame rendering", 360, startY + 140);
    M5.Display.drawString("• Reduce complexity", 360, startY + 155);
    
    // Handle B button to toggle settings
    static bool lastBtnB = false;
    bool currentBtnB = M5.BtnB.isPressed();
    if (currentBtnB && !lastBtnB) {
        static int setting = 0;
        switch(setting) {
            case 0: targetFPS = (targetFPS == 30) ? 60 : 30; break;
            case 1: vsyncEnabled = !vsyncEnabled; break;
            case 2: activeObjects = (activeObjects < 50) ? 50 : 20; break;
        }
        setting = (setting + 1) % 3;
    }
    lastBtnB = currentBtnB;
}

void drawMemoryManagementDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Memory Management", 10, startY);
    
    // Memory statistics
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t usedHeap = totalHeap - freeHeap;
    uint32_t largestFreeBlock = ESP.getMaxAllocHeap();
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Heap Information:", 10, startY + 20);
    M5.Display.drawString("Total: " + String(totalHeap) + " bytes", 10, startY + 35);
    M5.Display.drawString("Free: " + String(freeHeap) + " bytes", 10, startY + 50);
    M5.Display.drawString("Used: " + String(usedHeap) + " bytes", 10, startY + 65);
    M5.Display.drawString("Largest Block: " + String(largestFreeBlock), 10, startY + 80);
    
    // Memory usage percentage
    float memoryUsagePercent = (float)usedHeap / totalHeap * 100;
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Usage: " + String(memoryUsagePercent, 1) + "%", 10, startY + 95);
    
    // Memory usage bar
    M5.Display.drawRect(10, startY + 110, 200, 20, TFT_WHITE);
    int usageWidth = (usedHeap * 198) / totalHeap;
    uint16_t usageColor = (memoryUsagePercent > 80) ? TFT_RED : 
                         (memoryUsagePercent > 60) ? TFT_YELLOW : TFT_GREEN;
    M5.Display.fillRect(11, startY + 111, usageWidth, 18, usageColor);
    
    // Memory allocation test
    static LGFX_Sprite* testSprites[10];
    static int allocatedSprites = 0;
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Memory Allocation Test:", 10, startY + 140);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Allocated sprites: " + String(allocatedSprites), 10, startY + 155);
    M5.Display.drawString("Press [B] to allocate/free", 10, startY + 170);
    
    // Draw allocated sprites
    for (int i = 0; i < allocatedSprites; i++) {
        if (testSprites[i]) {
            int x = 10 + (i % 5) * 32;
            int y = startY + 185 + (i / 5) * 32;
            M5.Display.fillRect(x, y, 30, 30, TFT_BLUE);
            M5.Display.drawRect(x, y, 30, 30, TFT_WHITE);
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString(String(i), x + 15, y + 15);
        }
    }
    
    // Memory optimization tips
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Memory Optimization:", 250, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Always deleteSprite() when done", 250, startY + 35);
    M5.Display.drawString("• Use smallest possible sprites", 250, startY + 50);
    M5.Display.drawString("• Reuse sprites when possible", 250, startY + 65);
    M5.Display.drawString("• Monitor heap fragmentation", 250, startY + 80);
    M5.Display.drawString("• Pool memory for frequent allocs", 250, startY + 95);
    
    // Memory leak detection
    static uint32_t previousFreeHeap = freeHeap;
    static int framesSinceCheck = 0;
    framesSinceCheck++;
    
    if (framesSinceCheck > 60) { // Check every 60 frames
        if (freeHeap < previousFreeHeap - 1000) { // Significant decrease
            M5.Display.setTextColor(TFT_RED);
            M5.Display.drawString("Possible memory leak detected!", 250, startY + 115);
        } else {
            M5.Display.setTextColor(TFT_GREEN);
            M5.Display.drawString("Memory stable", 250, startY + 115);
        }
        previousFreeHeap = freeHeap;
        framesSinceCheck = 0;
    }
    
    // Heap history graph
    static uint32_t heapHistory[80];
    static int heapHistoryIndex = 0;
    
    if (frameCount % 5 == 0) { // Update every 5 frames
        heapHistory[heapHistoryIndex] = freeHeap;
        heapHistoryIndex = (heapHistoryIndex + 1) % 80;
    }
    
    // Draw heap graph
    M5.Display.drawRect(250, startY + 140, 80, 60, TFT_WHITE);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Heap History", 290, startY + 130);
    
    uint32_t maxHeap = totalHeap;
    for (int i = 1; i < 80; i++) {
        int prev = (heapHistoryIndex - i + 80) % 80;
        int curr = (heapHistoryIndex - i + 1 + 80) % 80;
        
        int y1 = startY + 200 - (heapHistory[prev] * 60 / maxHeap);
        int y2 = startY + 200 - (heapHistory[curr] * 60 / maxHeap);
        
        y1 = constrain(y1, startY + 140, startY + 200);
        y2 = constrain(y2, startY + 140, startY + 200);
        
        M5.Display.drawLine(250 + 79 - i, y1, 250 + 80 - i, y2, TFT_CYAN);
    }
    
    // Handle B button for sprite allocation
    static bool lastBtnB = false;
    bool currentBtnB = M5.BtnB.isPressed();
    if (currentBtnB && !lastBtnB) {
        if (allocatedSprites < 10) {
            // Allocate new sprite
            testSprites[allocatedSprites] = new LGFX_Sprite(&M5.Display);
            if (testSprites[allocatedSprites]->createSprite(30, 30)) {
                testSprites[allocatedSprites]->fillScreen(TFT_BLUE);
                allocatedSprites++;
            } else {
                delete testSprites[allocatedSprites];
                testSprites[allocatedSprites] = nullptr;
            }
        } else {
            // Free all sprites
            for (int i = 0; i < allocatedSprites; i++) {
                if (testSprites[i]) {
                    testSprites[i]->deleteSprite();
                    delete testSprites[i];
                    testSprites[i] = nullptr;
                }
            }
            allocatedSprites = 0;
        }
    }
    lastBtnB = currentBtnB;
}

void drawProfilingToolsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Performance Profiling Tools", 10, startY);
    
    // Timing different operations
    static struct {
        const char* name;
        unsigned long time;
    } operations[6];
    
    // Measure drawing operations
    unsigned long startTime;
    
    // 1. Pixel drawing
    startTime = micros();
    for (int i = 0; i < 100; i++) {
        M5.Display.drawPixel(50 + i % 10, startY + 30 + i / 10, TFT_WHITE);
    }
    operations[0] = {"Pixels", micros() - startTime};
    
    // 2. Line drawing
    startTime = micros();
    for (int i = 0; i < 20; i++) {
        M5.Display.drawLine(80 + i, startY + 30, 80 + i, startY + 50, TFT_GREEN);
    }
    operations[1] = {"Lines", micros() - startTime};
    
    // 3. Rectangle filling
    startTime = micros();
    for (int i = 0; i < 10; i++) {
        M5.Display.fillRect(120 + i * 3, startY + 30 + i, 2, 2, TFT_BLUE);
    }
    operations[2] = {"Rects", micros() - startTime};
    
    // 4. Circle drawing
    startTime = micros();
    for (int i = 0; i < 10; i++) {
        M5.Display.drawCircle(170 + i * 4, startY + 40, 2, TFT_CYAN);
    }
    operations[3] = {"Circles", micros() - startTime};
    
    // 5. Text rendering
    startTime = micros();
    M5.Display.setTextColor(TFT_YELLOW);
    for (int i = 0; i < 5; i++) {
        M5.Display.drawString("T" + String(i), 220 + i * 10, startY + 30 + i * 4);
    }
    operations[4] = {"Text", micros() - startTime};
    
    // 6. Sprite operation
    static LGFX_Sprite profSprite(&M5.Display);
    static bool spriteCreated = false;
    if (!spriteCreated) {
        profSprite.createSprite(20, 20);
        profSprite.fillScreen(TFT_MAGENTA);
        spriteCreated = true;
    }
    
    startTime = micros();
    for (int i = 0; i < 5; i++) {
        profSprite.pushSprite(270 + i * 5, startY + 30 + i * 2);
    }
    operations[5] = {"Sprites", micros() - startTime};
    
    // Display timing results
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Operation Timing (μs):", 10, startY + 70);
    
    for (int i = 0; i < 6; i++) {
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString(String(operations[i].name) + ":", 10, startY + 90 + i * 15);
        M5.Display.drawString(String(operations[i].time), 80, startY + 90 + i * 15);
        
        // Visual bar
        int barLength = operations[i].time / 10; // Scale for display
        barLength = constrain(barLength, 1, 100);
        uint16_t barColor = (i % 2) ? TFT_CYAN : TFT_YELLOW;
        M5.Display.fillRect(120, startY + 90 + i * 15, barLength, 10, barColor);
    }
    
    // Frame timing breakdown
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Frame Timing Breakdown:", 250, startY + 70);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Draw: " + String(stats.drawTime) + "μs", 250, startY + 90);
    M5.Display.drawString("Update: " + String(stats.updateTime) + "μs", 250, startY + 105);
    M5.Display.drawString("Total: " + String(stats.totalTime) + "ms", 250, startY + 120);
    M5.Display.drawString("FPS: " + String((int)stats.fps), 250, startY + 135);
    
    // Performance recommendations
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Optimization Priority:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Find slowest operation
    int slowest = 0;
    for (int i = 1; i < 6; i++) {
        if (operations[i].time > operations[slowest].time) {
            slowest = i;
        }
    }
    
    M5.Display.drawString("Slowest: " + String(operations[slowest].name), 10, startY + 195);
    
    if (stats.fps < 30) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Performance Issue Detected!", 10, startY + 210);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("Consider reducing complexity", 10, startY + 225);
    } else {
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Performance Good", 10, startY + 210);
    }
    
    // Profiling best practices
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Profiling Tips:", 250, startY + 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Profile on target hardware", 250, startY + 175);
    M5.Display.drawString("• Test with real-world data", 250, startY + 190);
    M5.Display.drawString("• Measure consistently", 250, startY + 205);
    M5.Display.drawString("• Focus on bottlenecks", 250, startY + 220);
}

void loop() {
    stats.frameStartTime = millis();
    M5.update();
    
    // Update performance stats
    updatePerformanceStats();
    
    // Handle button presses
    if (M5.BtnA.wasPressed()) {
        currentDemo = (PerformanceDemo)((currentDemo - 1 + PERFORMANCE_DEMO_COUNT) % PERFORMANCE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (PerformanceDemo)((currentDemo + 1) % PERFORMANCE_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS base
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw current demo
        drawCurrentPerformanceDemo();
        
        lastUpdate = millis();
    }
}