/*
 * M5GFX Tutorial - 10_advanced_effects
 * 
 * This demo demonstrates complex graphics effects in M5GFX:
 * - Plasma effects with mathematical algorithms
 * - 3D wireframe objects and rotation
 * - Fractal generation (Mandelbrot, Julia sets)
 * - Fire simulation and particle effects
 * - Matrix rain digital effect
 * - Water ripple simulations
 * - Tunnel and warping effects
 * - Real-time procedural generation
 * 
 * Key concepts:
 * - Mathematical visualization
 * - Real-time effect generation
 * - Color palette manipulation
 * - Algorithmic art techniques
 * - Performance-optimized effects
 */

#include <M5Unified.h>
#include <math.h>

// Demo modes for different advanced effects
enum EffectDemo {
    DEMO_PLASMA_EFFECTS,
    DEMO_3D_WIREFRAME,
    DEMO_FRACTALS,
    DEMO_FIRE_SIMULATION,
    DEMO_MATRIX_RAIN,
    DEMO_WATER_RIPPLES,
    EFFECT_DEMO_COUNT
};

EffectDemo currentDemo = DEMO_PLASMA_EFFECTS;
const char* effectDemoNames[] = {
    "Plasma Effects",
    "3D Wireframe",
    "Fractals",
    "Fire Simulation",
    "Matrix Rain",
    "Water Ripples"
};

// Animation variables
unsigned long lastUpdate = 0;
float animationTime = 0;
int animationStep = 0;

// Plasma effect variables
float plasmaTime = 0;

// 3D wireframe variables
struct Point3D {
    float x, y, z;
};

struct Point2D {
    int x, y;
};

// Cube vertices for 3D demo
Point3D cubeVertices[8] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
};

int cubeEdges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // back face
    {4,5}, {5,6}, {6,7}, {7,4}, // front face
    {0,4}, {1,5}, {2,6}, {3,7}  // connecting edges
};

// Fire simulation
struct FireParticle {
    float x, y;
    float vy;
    float temperature;
    int life;
    bool active;
};

const int FIRE_WIDTH = 60;
const int FIRE_HEIGHT = 40;
const int MAX_FIRE_PARTICLES = 200;
uint8_t fireBuffer[FIRE_WIDTH * FIRE_HEIGHT];
FireParticle fireParticles[MAX_FIRE_PARTICLES];

// Matrix rain
struct MatrixDrop {
    int x, y;
    int speed;
    int length;
    char character;
    bool active;
    uint8_t brightness;
};

const int MAX_MATRIX_DROPS = 30;
MatrixDrop matrixDrops[MAX_MATRIX_DROPS];

// Water ripples
struct Ripple {
    float x, y;
    float radius;
    float amplitude;
    bool active;
};

const int MAX_RIPPLES = 10;
Ripple ripples[MAX_RIPPLES];
float waterTime = 0;

// Color palettes
uint16_t fireColors[256];
uint16_t plasmaColors[256];
uint16_t fractalColors[256];

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize color palettes
    initColorPalettes();
    
    // Initialize effects
    initFireSimulation();
    initMatrixRain();
    initWaterRipples();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void initColorPalettes() {
    // Fire palette: black -> red -> yellow -> white
    for (int i = 0; i < 256; i++) {
        uint8_t r, g, b;
        if (i < 64) {
            r = i * 4;
            g = 0;
            b = 0;
        } else if (i < 128) {
            r = 255;
            g = (i - 64) * 4;
            b = 0;
        } else if (i < 192) {
            r = 255;
            g = 255;
            b = (i - 128) * 4;
        } else {
            r = 255;
            g = 255;
            b = 255;
        }
        fireColors[i] = M5.Display.color565(r, g, b);
    }
    
    // Plasma palette: smooth color cycling
    for (int i = 0; i < 256; i++) {
        uint8_t r = 128 + 127 * sin(i * 0.024);
        uint8_t g = 128 + 127 * sin(i * 0.024 + 2.1);
        uint8_t b = 128 + 127 * sin(i * 0.024 + 4.2);
        plasmaColors[i] = M5.Display.color565(r, g, b);
    }
    
    // Fractal palette: cool blues to hot reds
    for (int i = 0; i < 256; i++) {
        uint8_t r = (i * 2) % 256;
        uint8_t g = (i * 3) % 256;
        uint8_t b = 255 - i;
        fractalColors[i] = M5.Display.color565(r, g, b);
    }
}

void initFireSimulation() {
    // Initialize fire buffer
    for (int i = 0; i < FIRE_WIDTH * FIRE_HEIGHT; i++) {
        fireBuffer[i] = 0;
    }
    
    // Initialize fire particles
    for (int i = 0; i < MAX_FIRE_PARTICLES; i++) {
        fireParticles[i].active = false;
    }
}

void initMatrixRain() {
    for (int i = 0; i < MAX_MATRIX_DROPS; i++) {
        matrixDrops[i].active = false;
    }
}

void initWaterRipples() {
    for (int i = 0; i < MAX_RIPPLES; i++) {
        ripples[i].active = false;
    }
}

Point2D project3D(Point3D point, float distance) {
    Point2D result;
    if (point.z + distance != 0) {
        result.x = (point.x * distance) / (point.z + distance);
        result.y = (point.y * distance) / (point.z + distance);
    } else {
        result.x = point.x * 1000;
        result.y = point.y * 1000;
    }
    return result;
}

Point3D rotateX(Point3D p, float angle) {
    Point3D result;
    result.x = p.x;
    result.y = p.y * cos(angle) - p.z * sin(angle);
    result.z = p.y * sin(angle) + p.z * cos(angle);
    return result;
}

Point3D rotateY(Point3D p, float angle) {
    Point3D result;
    result.x = p.x * cos(angle) + p.z * sin(angle);
    result.y = p.y;
    result.z = -p.x * sin(angle) + p.z * cos(angle);
    return result;
}

Point3D rotateZ(Point3D p, float angle) {
    Point3D result;
    result.x = p.x * cos(angle) - p.y * sin(angle);
    result.y = p.x * sin(angle) + p.y * cos(angle);
    result.z = p.z;
    return result;
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX", M5.Display.width()/2, 40);
    M5.Display.drawString("Advanced", M5.Display.width()/2, 75);
    M5.Display.drawString("Effects", M5.Display.width()/2, 110);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Complex", M5.Display.width()/2, 145);
    M5.Display.drawString("Graphics", M5.Display.width()/2, 170);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Stunning visual effects and mathematical art", M5.Display.width()/2, 200);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Plasma & Fire Effects", M5.Display.width()/2, 220);
    M5.Display.drawString("• 3D Graphics & Fractals", M5.Display.width()/2, 235);
    M5.Display.drawString("• Matrix Rain & Water Ripples", M5.Display.width()/2, 250);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Advanced Effects", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.drawString(effectDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(EFFECT_DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("[A] Prev  [B] Reset  [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Draw demo-specific content
    drawCurrentEffectDemo();
}

void drawCurrentEffectDemo() {
    switch(currentDemo) {
        case DEMO_PLASMA_EFFECTS:
            drawPlasmaEffectsDemo();
            break;
        case DEMO_3D_WIREFRAME:
            draw3DWireframeDemo();
            break;
        case DEMO_FRACTALS:
            drawFractalsDemo();
            break;
        case DEMO_FIRE_SIMULATION:
            drawFireSimulationDemo();
            break;
        case DEMO_MATRIX_RAIN:
            drawMatrixRainDemo();
            break;
        case DEMO_WATER_RIPPLES:
            drawWaterRipplesDemo();
            break;
    }
}

void drawPlasmaEffectsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Plasma Effects", 10, startY);
    
    // Main plasma effect
    int plasmaWidth = 120;
    int plasmaHeight = 80;
    int plasmaStartX = 10;
    int plasmaStartY = startY + 20;
    
    M5.Display.startWrite(); // Batch for performance
    
    for (int y = 0; y < plasmaHeight; y++) {
        for (int x = 0; x < plasmaWidth; x++) {
            // Multiple sine waves create plasma effect
            float value1 = sin(x * 0.1 + plasmaTime);
            float value2 = sin(y * 0.1 + plasmaTime * 1.3);
            float value3 = sin((x + y) * 0.08 + plasmaTime * 0.7);
            float value4 = sin(sqrt((x - plasmaWidth/2) * (x - plasmaWidth/2) + 
                                  (y - plasmaHeight/2) * (y - plasmaHeight/2)) * 0.15 + plasmaTime * 2);
            
            float plasma = (value1 + value2 + value3 + value4) * 0.25;
            
            // Convert to color index
            int colorIndex = (int)((plasma + 1) * 127.5);
            colorIndex = constrain(colorIndex, 0, 255);
            
            M5.Display.drawPixel(plasmaStartX + x, plasmaStartY + y, plasmaColors[colorIndex]);
        }
    }
    
    M5.Display.endWrite();
    
    // Smaller plasma variants
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Variants:", 150, startY + 20);
    
    // Circular plasma
    int circleX = 170, circleY = startY + 40;
    int radius = 30;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                float distance = sqrt(x*x + y*y);
                float angle = atan2(y, x);
                float plasma = sin(distance * 0.3 + plasmaTime * 2) * 
                              cos(angle * 4 + plasmaTime);
                
                int colorIndex = (int)((plasma + 1) * 127.5);
                colorIndex = constrain(colorIndex, 0, 255);
                
                M5.Display.drawPixel(circleX + x, circleY + y, plasmaColors[colorIndex]);
            }
        }
    }
    
    // Tunnel effect
    int tunnelX = 250, tunnelY = startY + 40;
    int tunnelSize = 40;
    for (int y = -tunnelSize/2; y <= tunnelSize/2; y++) {
        for (int x = -tunnelSize/2; x <= tunnelSize/2; x++) {
            float distance = sqrt(x*x + y*y);
            float angle = atan2(y, x);
            
            if (distance > 5) { // Avoid division by zero
                float tunnel = sin(32/distance + plasmaTime * 3) + 
                              sin(angle * 8 + plasmaTime * 2);
                
                int colorIndex = (int)((tunnel + 2) * 63.75);
                colorIndex = constrain(colorIndex, 0, 255);
                
                M5.Display.drawPixel(tunnelX + x, tunnelY + y, plasmaColors[colorIndex]);
            }
        }
    }
    
    // Interference pattern
    int intX = 320, intY = startY + 40;
    int intSize = 50;
    for (int y = 0; y < intSize; y++) {
        for (int x = 0; x < intSize; x++) {
            float dist1 = sqrt((x-15)*(x-15) + (y-15)*(y-15));
            float dist2 = sqrt((x-35)*(x-35) + (y-25)*(y-25));
            
            float interference = sin(dist1 * 0.5 + plasmaTime * 4) + 
                               sin(dist2 * 0.5 + plasmaTime * 4);
            
            int colorIndex = (int)((interference + 2) * 63.75);
            colorIndex = constrain(colorIndex, 0, 255);
            
            M5.Display.drawPixel(intX + x, intY + y, plasmaColors[colorIndex]);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Plasma Mathematics:", 10, startY + 110);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Multiple sine wave interference", 10, startY + 125);
    M5.Display.drawString("• Distance-based color mapping", 10, startY + 140);
    M5.Display.drawString("• Real-time parameter animation", 10, startY + 155);
    M5.Display.drawString("• Color palette interpolation", 10, startY + 170);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Effects shown:", 250, startY + 110);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Classic plasma (top left)", 250, startY + 125);
    M5.Display.drawString("• Circular plasma (center)", 250, startY + 140);
    M5.Display.drawString("• Tunnel effect (right)", 250, startY + 155);
    M5.Display.drawString("• Wave interference (far right)", 250, startY + 170);
    
    plasmaTime += 0.1;
}

void draw3DWireframeDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("3D Wireframe Graphics", 10, startY);
    
    // Rotation angles
    float angleX = animationTime * 0.7;
    float angleY = animationTime * 1.2;
    float angleZ = animationTime * 0.5;
    
    // Draw rotating cube
    Point2D centerCube = {100, startY + 80};
    float scaleCube = 40;
    float distanceCube = 4;
    
    Point2D projectedVertices[8];
    float vertexDepths[8];
    
    for (int i = 0; i < 8; i++) {
        Point3D vertex = cubeVertices[i];
        
        // Apply rotations
        vertex = rotateX(vertex, angleX);
        vertex = rotateY(vertex, angleY);
        vertex = rotateZ(vertex, angleZ);
        
        // Scale and position
        vertex.x *= scaleCube;
        vertex.y *= scaleCube;
        vertex.z *= scaleCube;
        vertex.z += 200; // Move away from camera
        
        // Project to 2D
        Point2D projected = project3D(vertex, distanceCube * scaleCube);
        projectedVertices[i].x = centerCube.x + projected.x;
        projectedVertices[i].y = centerCube.y + projected.y;
        vertexDepths[i] = vertex.z;
    }
    
    // Draw edges with depth-based coloring
    for (int i = 0; i < 12; i++) {
        int v1 = cubeEdges[i][0];
        int v2 = cubeEdges[i][1];
        
        float avgDepth = (vertexDepths[v1] + vertexDepths[v2]) / 2;
        uint8_t intensity = 100 + 155 * (300 - avgDepth) / 200; // Closer = brighter
        intensity = constrain(intensity, 50, 255);
        uint16_t color = M5.Display.color565(intensity, intensity, intensity);
        
        M5.Display.drawLine(projectedVertices[v1].x, projectedVertices[v1].y,
                           projectedVertices[v2].x, projectedVertices[v2].y, color);
    }
    
    // Draw vertices
    for (int i = 0; i < 8; i++) {
        uint8_t intensity = 100 + 155 * (300 - vertexDepths[i]) / 200;
        intensity = constrain(intensity, 50, 255);
        uint16_t color = M5.Display.color565(intensity, 0, 0);
        M5.Display.fillCircle(projectedVertices[i].x, projectedVertices[i].y, 2, color);
    }
    
    // Draw pyramid
    Point2D centerPyramid = {250, startY + 80};
    Point3D pyramidVertices[5] = {
        {0, -1, 0},   // Top
        {-1, 1, -1},  // Base corners
        {1, 1, -1},
        {1, 1, 1},
        {-1, 1, 1}
    };
    
    int pyramidEdges[8][2] = {
        {0,1}, {0,2}, {0,3}, {0,4}, // Top to base
        {1,2}, {2,3}, {3,4}, {4,1}  // Base edges
    };
    
    Point2D pyramidProjected[5];
    for (int i = 0; i < 5; i++) {
        Point3D vertex = pyramidVertices[i];
        vertex = rotateX(vertex, angleX * 0.5);
        vertex = rotateY(vertex, angleY * 0.8);
        
        vertex.x *= 30;
        vertex.y *= 30;
        vertex.z *= 30;
        vertex.z += 150;
        
        Point2D projected = project3D(vertex, 120);
        pyramidProjected[i].x = centerPyramid.x + projected.x;
        pyramidProjected[i].y = centerPyramid.y + projected.y;
    }
    
    // Draw pyramid edges
    for (int i = 0; i < 8; i++) {
        int v1 = pyramidEdges[i][0];
        int v2 = pyramidEdges[i][1];
        M5.Display.drawLine(pyramidProjected[v1].x, pyramidProjected[v1].y,
                           pyramidProjected[v2].x, pyramidProjected[v2].y, TFT_CYAN);
    }
    
    // Draw torus (donut shape)
    Point2D centerTorus = {370, startY + 80};
    float R = 20; // Major radius
    float r = 8;  // Minor radius
    
    // Draw torus wireframe
    for (int u = 0; u < 16; u++) {
        for (int v = 0; v < 12; v++) {
            float theta = u * 2 * PI / 16;
            float phi = v * 2 * PI / 12;
            
            Point3D torusPoint;
            torusPoint.x = (R + r * cos(phi)) * cos(theta);
            torusPoint.y = (R + r * cos(phi)) * sin(theta);
            torusPoint.z = r * sin(phi);
            
            torusPoint = rotateX(torusPoint, angleX * 0.3);
            torusPoint = rotateY(torusPoint, angleY * 0.6);
            torusPoint.z += 120;
            
            Point2D projected = project3D(torusPoint, 100);
            int screenX = centerTorus.x + projected.x;
            int screenY = centerTorus.y + projected.y;
            
            if (screenX >= 0 && screenX < M5.Display.width() && 
                screenY >= 0 && screenY < M5.Display.height()) {
                M5.Display.drawPixel(screenX, screenY, TFT_YELLOW);
            }
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("3D Objects:", 10, startY + 160);
    M5.Display.drawString("Cube", 50, startY + 175);
    M5.Display.drawString("Pyramid", 180, startY + 175);
    M5.Display.drawString("Torus", 320, startY + 175);
    
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("3D Techniques:", 10, startY + 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Perspective projection", 10, startY + 215);
    M5.Display.drawString("• Depth-based shading", 10, startY + 230);
    M5.Display.drawString("• Multiple rotation axes", 10, startY + 245);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Rendering:", 200, startY + 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Wireframe display", 200, startY + 215);
    M5.Display.drawString("• Real-time rotation", 200, startY + 230);
    M5.Display.drawString("• Parametric surfaces", 200, startY + 245);
}

int mandelbrot(float x0, float y0, int maxIter) {
    float x = 0, y = 0;
    int iteration = 0;
    
    while (x*x + y*y <= 4 && iteration < maxIter) {
        float xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iteration++;
    }
    
    return iteration;
}

void drawFractalsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Fractal Generation", 10, startY);
    
    // Mandelbrot set
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mandelbrot Set:", 10, startY + 20);
    
    int mandelbrotWidth = 120;
    int mandelbrotHeight = 80;
    int mandelbrotX = 10;
    int mandelbrotY = startY + 35;
    
    float zoom = 1 + 0.5 * sin(animationTime * 0.5);
    float centerX = -0.5 + 0.3 * cos(animationTime * 0.3);
    float centerY = 0 + 0.3 * sin(animationTime * 0.4);
    
    M5.Display.startWrite();
    
    for (int py = 0; py < mandelbrotHeight; py++) {
        for (int px = 0; px < mandelbrotWidth; px++) {
            float x = centerX + (px - mandelbrotWidth/2) * 0.01 / zoom;
            float y = centerY + (py - mandelbrotHeight/2) * 0.01 / zoom;
            
            int iteration = mandelbrot(x, y, 32);
            
            uint16_t color;
            if (iteration == 32) {
                color = TFT_BLACK;
            } else {
                int colorIndex = (iteration * 8) % 256;
                color = fractalColors[colorIndex];
            }
            
            M5.Display.drawPixel(mandelbrotX + px, mandelbrotY + py, color);
        }
    }
    
    M5.Display.endWrite();
    
    // Julia set
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Julia Set:", 150, startY + 20);
    
    int juliaWidth = 80;
    int juliaHeight = 80;
    int juliaX = 150;
    int juliaY = startY + 35;
    
    // Animated Julia set parameters
    float cReal = 0.3 * cos(animationTime * 0.7);
    float cImag = 0.3 * sin(animationTime * 0.5);
    
    for (int py = 0; py < juliaHeight; py++) {
        for (int px = 0; px < juliaWidth; px++) {
            float x = (px - juliaWidth/2) * 0.04;
            float y = (py - juliaHeight/2) * 0.04;
            
            int iteration = 0;
            int maxIter = 32;
            
            while (x*x + y*y <= 4 && iteration < maxIter) {
                float xtemp = x*x - y*y + cReal;
                y = 2*x*y + cImag;
                x = xtemp;
                iteration++;
            }
            
            uint16_t color;
            if (iteration == maxIter) {
                color = TFT_BLACK;
            } else {
                int colorIndex = (iteration * 8) % 256;
                color = fractalColors[colorIndex];
            }
            
            M5.Display.drawPixel(juliaX + px, juliaY + py, color);
        }
    }
    
    // Burning Ship fractal
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Burning Ship:", 250, startY + 20);
    
    int shipWidth = 80;
    int shipHeight = 80;
    int shipX = 250;
    int shipY = startY + 35;
    
    float shipZoom = 1 + 0.3 * sin(animationTime * 0.3);
    
    for (int py = 0; py < shipHeight; py++) {
        for (int px = 0; px < shipWidth; px++) {
            float x0 = -1.8 + (px - shipWidth/2) * 0.02 / shipZoom;
            float y0 = -0.08 + (py - shipHeight/2) * 0.02 / shipZoom;
            
            float x = 0, y = 0;
            int iteration = 0;
            int maxIter = 32;
            
            while (x*x + y*y <= 4 && iteration < maxIter) {
                float xtemp = x*x - y*y + x0;
                y = abs(2*x*y) + y0; // abs() makes it "burning"
                x = abs(xtemp);      // abs() here too
                iteration++;
            }
            
            uint16_t color;
            if (iteration == maxIter) {
                color = TFT_BLACK;
            } else {
                int colorIndex = (iteration * 6) % 256;
                color = fractalColors[colorIndex];
            }
            
            M5.Display.drawPixel(shipX + px, shipY + py, color);
        }
    }
    
    // Sierpinski triangle
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Sierpinski:", 350, startY + 20);
    
    int sierpX = 350;
    int sierpY = startY + 35;
    int sierpSize = 64;
    
    for (int y = 0; y < sierpSize; y++) {
        for (int x = 0; x < sierpSize; x++) {
            // Sierpinski triangle rule
            if ((x & y) == 0) {
                uint16_t color = M5.Display.color565(x * 4, y * 4, 128);
                M5.Display.drawPixel(sierpX + x, sierpY + y, color);
            }
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Fractal Mathematics:", 10, startY + 125);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Complex number iteration", 10, startY + 140);
    M5.Display.drawString("• Escape-time algorithms", 10, startY + 155);
    M5.Display.drawString("• Self-similarity at all scales", 10, startY + 170);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Parameters:", 250, startY + 125);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Mandelbrot zoom: " + String(zoom, 2), 250, startY + 140);
    M5.Display.drawString("Julia c: " + String(cReal, 2) + "+" + String(cImag, 2) + "i", 250, startY + 155);
    M5.Display.drawString("Max iterations: 32", 250, startY + 170);
}

void updateFireSimulation() {
    // Add heat source at bottom
    for (int x = 0; x < FIRE_WIDTH; x++) {
        fireBuffer[x + (FIRE_HEIGHT-1) * FIRE_WIDTH] = 255;
    }
    
    // Propagate fire upward with cooling
    for (int y = 0; y < FIRE_HEIGHT - 1; y++) {
        for (int x = 0; x < FIRE_WIDTH; x++) {
            int index = x + y * FIRE_WIDTH;
            int belowIndex = x + (y + 1) * FIRE_WIDTH;
            
            // Average with neighbors and cool down
            int heat = fireBuffer[belowIndex];
            if (x > 0) heat += fireBuffer[belowIndex - 1];
            if (x < FIRE_WIDTH - 1) heat += fireBuffer[belowIndex + 1];
            heat /= 3;
            
            // Random cooling
            heat -= random(8);
            if (heat < 0) heat = 0;
            
            fireBuffer[index] = heat;
        }
    }
    
    // Update fire particles
    for (int i = 0; i < MAX_FIRE_PARTICLES; i++) {
        if (fireParticles[i].active) {
            fireParticles[i].y -= fireParticles[i].vy;
            fireParticles[i].vy += 0.1; // Gravity
            fireParticles[i].temperature -= 2;
            fireParticles[i].life--;
            
            if (fireParticles[i].life <= 0 || fireParticles[i].temperature <= 0) {
                fireParticles[i].active = false;
            }
        }
    }
    
    // Spawn new particles
    if (random(100) < 20) {
        for (int i = 0; i < MAX_FIRE_PARTICLES; i++) {
            if (!fireParticles[i].active) {
                fireParticles[i].x = random(FIRE_WIDTH);
                fireParticles[i].y = FIRE_HEIGHT - 1;
                fireParticles[i].vy = 1 + random(20) / 10.0;
                fireParticles[i].temperature = 200 + random(55);
                fireParticles[i].life = 30 + random(20);
                fireParticles[i].active = true;
                break;
            }
        }
    }
}

void drawFireSimulationDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Fire Simulation", 10, startY);
    
    // Update fire simulation
    updateFireSimulation();
    
    // Draw fire buffer
    int fireDisplayX = 10;
    int fireDisplayY = startY + 20;
    int pixelSize = 3;
    
    M5.Display.startWrite();
    
    for (int y = 0; y < FIRE_HEIGHT; y++) {
        for (int x = 0; x < FIRE_WIDTH; x++) {
            int heat = fireBuffer[x + y * FIRE_WIDTH];
            uint16_t color = fireColors[heat];
            
            M5.Display.fillRect(fireDisplayX + x * pixelSize, 
                               fireDisplayY + y * pixelSize,
                               pixelSize, pixelSize, color);
        }
    }
    
    M5.Display.endWrite();
    
    // Draw fire particles
    for (int i = 0; i < MAX_FIRE_PARTICLES; i++) {
        if (fireParticles[i].active) {
            int x = fireDisplayX + fireParticles[i].x * pixelSize;
            int y = fireDisplayY + fireParticles[i].y * pixelSize;
            
            uint8_t temp = constrain(fireParticles[i].temperature, 0, 255);
            uint16_t color = fireColors[temp];
            
            M5.Display.fillCircle(x, y, 2, color);
        }
    }
    
    // Side fire effects
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Candle:", 220, startY + 20);
    
    // Simple candle flame
    int candleX = 250;
    int candleY = startY + 100;
    int flameHeight = 30 + 10 * sin(animationTime * 8);
    
    for (int y = 0; y < flameHeight; y++) {
        int width = 8 - (y * 6 / flameHeight);
        for (int x = -width/2; x <= width/2; x++) {
            float heat = (flameHeight - y) * 255.0 / flameHeight;
            heat += random(50) - 25; // Flicker
            heat = constrain(heat, 0, 255);
            
            M5.Display.drawPixel(candleX + x + random(3) - 1, 
                               candleY - y, fireColors[(int)heat]);
        }
    }
    
    // Torch effect
    M5.Display.drawString("Torch:", 300, startY + 20);
    
    int torchX = 330;
    int torchY = startY + 100;
    
    for (int i = 0; i < 20; i++) {
        float angle = (i * 18 + animationTime * 100) * PI / 180;
        int radius = 15 + 5 * sin(animationTime * 6 + i);
        int x = torchX + radius * cos(angle);
        int y = torchY - 10 + radius * sin(angle) * 0.3;
        
        uint8_t heat = 200 - i * 8 + random(50);
        heat = constrain(heat, 0, 255);
        
        M5.Display.fillCircle(x, y, 3, fireColors[heat]);
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Fire Physics:", 10, startY + 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Heat propagation upward", 10, startY + 175);
    M5.Display.drawString("• Random cooling effects", 10, startY + 190);
    M5.Display.drawString("• Particle-based embers", 10, startY + 205);
    M5.Display.drawString("• Gradient color mapping", 10, startY + 220);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Effects:", 250, startY + 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Main fire (left)", 250, startY + 175);
    M5.Display.drawString("• Candle flame", 250, startY + 190);
    M5.Display.drawString("• Torch effect", 250, startY + 205);
    M5.Display.drawString("Active particles: " + String(MAX_FIRE_PARTICLES), 250, startY + 220);
}

void updateMatrixRain() {
    // Spawn new drops
    if (random(100) < 10) {
        for (int i = 0; i < MAX_MATRIX_DROPS; i++) {
            if (!matrixDrops[i].active) {
                matrixDrops[i].x = random(M5.Display.width()/8) * 8;
                matrixDrops[i].y = 70;
                matrixDrops[i].speed = 1 + random(3);
                matrixDrops[i].length = 5 + random(10);
                matrixDrops[i].character = '!' + random(94); // Printable ASCII
                matrixDrops[i].brightness = 255;
                matrixDrops[i].active = true;
                break;
            }
        }
    }
    
    // Update existing drops
    for (int i = 0; i < MAX_MATRIX_DROPS; i++) {
        if (matrixDrops[i].active) {
            matrixDrops[i].y += matrixDrops[i].speed;
            
            if (matrixDrops[i].y > M5.Display.height()) {
                matrixDrops[i].active = false;
            }
        }
    }
}

void drawMatrixRainDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Matrix Rain Effect", 10, startY);
    
    // Fade screen slightly for trail effect
    for (int y = startY + 20; y < M5.Display.height() - 30; y += 2) {
        for (int x = 0; x < M5.Display.width(); x += 2) {
            uint16_t color = M5.Display.readPixel(x, y);
            if (color != TFT_BLACK) {
                // Darken the pixel
                uint8_t r = ((color >> 11) & 0x1F) * 8;
                uint8_t g = ((color >> 5) & 0x3F) * 4;
                uint8_t b = (color & 0x1F) * 8;
                
                r = r * 0.9;
                g = g * 0.9;
                b = b * 0.9;
                
                uint16_t newColor = M5.Display.color565(r, g, b);
                M5.Display.drawPixel(x, y, newColor);
            }
        }
    }
    
    // Update and draw matrix drops
    updateMatrixRain();
    
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    
    for (int i = 0; i < MAX_MATRIX_DROPS; i++) {
        if (matrixDrops[i].active) {
            // Draw the character trail
            for (int j = 0; j < matrixDrops[i].length; j++) {
                int y = matrixDrops[i].y - j * 8;
                if (y >= startY + 20 && y < M5.Display.height() - 30) {
                    uint8_t brightness;
                    uint16_t color;
                    
                    if (j == 0) {
                        // Head of the drop - brightest white
                        brightness = 255;
                        color = TFT_WHITE;
                    } else {
                        // Trail - green with fading
                        brightness = 255 - (j * 255 / matrixDrops[i].length);
                        color = M5.Display.color565(0, brightness, 0);
                    }
                    
                    M5.Display.setTextColor(color);
                    M5.Display.drawString(String((char)matrixDrops[i].character), 
                                         matrixDrops[i].x, y);
                    
                    // Change character occasionally
                    if (random(100) < 5) {
                        matrixDrops[i].character = '!' + random(94);
                    }
                }
            }
        }
    }
    
    // Add some binary numbers floating around
    static int binaryChars[20][3]; // x, y, char
    static bool binaryInitialized = false;
    
    if (!binaryInitialized) {
        for (int i = 0; i < 20; i++) {
            binaryChars[i][0] = random(M5.Display.width());
            binaryChars[i][1] = random(startY + 20, M5.Display.height() - 50);
            binaryChars[i][2] = random(2); // 0 or 1
        }
        binaryInitialized = true;
    }
    
    // Update and draw binary characters
    for (int i = 0; i < 20; i++) {
        binaryChars[i][0] -= 1;
        if (binaryChars[i][0] < 0) {
            binaryChars[i][0] = M5.Display.width();
            binaryChars[i][1] = random(startY + 20, M5.Display.height() - 50);
            binaryChars[i][2] = random(2);
        }
        
        // Draw binary digit with low opacity
        uint16_t color = M5.Display.color565(0, 100, 0);
        M5.Display.setTextColor(color);
        M5.Display.drawString(String(binaryChars[i][2]), binaryChars[i][0], binaryChars[i][1]);
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Matrix Effect:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Falling character streams", 10, startY + 195);
    M5.Display.drawString("• Trail fading effect", 10, startY + 210);
    M5.Display.drawString("• Random character changes", 10, startY + 225);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Active drops: " + String(MAX_MATRIX_DROPS), 250, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• White leading character", 250, startY + 195);
    M5.Display.drawString("• Green fading trail", 250, startY + 210);
    M5.Display.drawString("• Background binary flow", 250, startY + 225);
}

void updateWaterRipples() {
    // Update existing ripples
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (ripples[i].active) {
            ripples[i].radius += 2;
            ripples[i].amplitude *= 0.98; // Fade out
            
            if (ripples[i].amplitude < 0.1 || ripples[i].radius > 100) {
                ripples[i].active = false;
            }
        }
    }
    
    // Occasionally create new ripple
    if (random(100) < 3) {
        for (int i = 0; i < MAX_RIPPLES; i++) {
            if (!ripples[i].active) {
                ripples[i].x = 50 + random(300);
                ripples[i].y = 120 + random(100);
                ripples[i].radius = 0;
                ripples[i].amplitude = 1.0;
                ripples[i].active = true;
                break;
            }
        }
    }
}

void drawWaterRipplesDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Water Ripple Simulation", 10, startY);
    
    // Update ripples
    updateWaterRipples();
    waterTime += 0.1;
    
    // Draw water surface with ripples
    int waterWidth = 400;
    int waterHeight = 120;
    int waterX = 10;
    int waterY = startY + 30;
    
    // Background water color
    M5.Display.fillRect(waterX, waterY, waterWidth, waterHeight, M5.Display.color565(0, 50, 100));
    
    // Calculate water surface with ripples
    M5.Display.startWrite();
    
    for (int y = 0; y < waterHeight; y++) {
        for (int x = 0; x < waterWidth; x++) {
            float surfaceHeight = 0;
            
            // Base wave pattern
            surfaceHeight += 5 * sin((x + waterTime * 20) * 0.02);
            surfaceHeight += 3 * sin((x + waterTime * 15) * 0.05 + 1);
            
            // Add ripple effects
            for (int r = 0; r < MAX_RIPPLES; r++) {
                if (ripples[r].active) {
                    float dx = x - (ripples[r].x - waterX);
                    float dy = y - (ripples[r].y - waterY);
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    if (distance < ripples[r].radius && distance > ripples[r].radius - 20) {
                        float rippleHeight = ripples[r].amplitude * 
                                           sin((distance - ripples[r].radius) * 0.5) *
                                           exp(-(distance - ripples[r].radius) * 0.1);
                        surfaceHeight += rippleHeight * 10;
                    }
                }
            }
            
            // Convert height to color
            int baseHeight = waterHeight / 2;
            if (y < baseHeight + surfaceHeight) {
                // Above water - lighter blue
                uint16_t color = M5.Display.color565(100, 150, 255);
                M5.Display.drawPixel(waterX + x, waterY + y, color);
            } else if (y < baseHeight + surfaceHeight + 5) {
                // Water surface - white foam
                uint16_t color = M5.Display.color565(200, 220, 255);
                M5.Display.drawPixel(waterX + x, waterY + y, color);
            }
        }
    }
    
    M5.Display.endWrite();
    
    // Draw ripple circles
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (ripples[i].active) {
            uint8_t alpha = 255 * ripples[i].amplitude;
            uint16_t color = M5.Display.color565(alpha, alpha, 255);
            
            M5.Display.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius, color);
            if (ripples[i].radius > 5) {
                M5.Display.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius - 3, color);
            }
        }
    }
    
    // Reflection effect
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Reflections:", 10, startY + 160);
    
    // Simple reflection of some objects above water
    int objY = waterY - 30;
    M5.Display.fillCircle(100, objY, 8, TFT_YELLOW); // "Sun"
    
    // Reflected sun (distorted)
    int refY = waterY + 30;
    for (int i = 0; i < 5; i++) {
        int offset = 2 * sin(waterTime + i) + i;
        M5.Display.fillCircle(100 + offset, refY + i * 3, 8 - i, 
                             M5.Display.color565(255 - i*30, 255 - i*30, 0));
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Water Physics:", 10, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Wave interference patterns", 10, startY + 195);
    M5.Display.drawString("• Ripple propagation", 10, startY + 210);
    M5.Display.drawString("• Surface height mapping", 10, startY + 225);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Effects:", 250, startY + 180);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Active ripples: " + String(MAX_RIPPLES), 250, startY + 195);
    M5.Display.drawString("• Expanding circles", 250, startY + 210);
    M5.Display.drawString("• Amplitude decay", 250, startY + 225);
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Touch screen to create ripples!", waterX + 50, waterY - 15);
    
    // Handle touch input for creating ripples
    M5.update();
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed() && 
            touch.x >= waterX && touch.x <= waterX + waterWidth &&
            touch.y >= waterY && touch.y <= waterY + waterHeight) {
            
            // Create new ripple at touch point
            for (int i = 0; i < MAX_RIPPLES; i++) {
                if (!ripples[i].active) {
                    ripples[i].x = touch.x;
                    ripples[i].y = touch.y;
                    ripples[i].radius = 0;
                    ripples[i].amplitude = 2.0;
                    ripples[i].active = true;
                    break;
                }
            }
        }
    }
}

void loop() {
    M5.update();
    
    // Handle button presses
    if (M5.BtnA.wasPressed()) {
        currentDemo = (EffectDemo)((currentDemo - 1 + EFFECT_DEMO_COUNT) % EFFECT_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        // Reset current demo
        animationTime = 0;
        animationStep = 0;
        plasmaTime = 0;
        waterTime = 0;
        
        // Reset specific demo states
        if (currentDemo == DEMO_FIRE_SIMULATION) {
            initFireSimulation();
        } else if (currentDemo == DEMO_MATRIX_RAIN) {
            initMatrixRain();
        } else if (currentDemo == DEMO_WATER_RIPPLES) {
            initWaterRipples();
        }
        
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (EffectDemo)((currentDemo + 1) % EFFECT_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 100) { // 10 FPS for complex effects
        animationTime += 0.1;
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw current demo
        drawCurrentEffectDemo();
        
        lastUpdate = millis();
    }
}