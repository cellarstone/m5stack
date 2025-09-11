/*
 * M5GFX Tutorial - 06_transformations
 * 
 * This demo demonstrates coordinate transformation techniques in M5GFX:
 * - Rotation around different pivot points
 * - Scaling from various origins
 * - Translation and coordinate mapping
 * - Combining multiple transformations
 * - Matrix transformation concepts
 * - 3D projection basics
 * - Viewport and camera transformations
 * - Performance considerations
 * 
 * Key concepts:
 * - Coordinate system transformations
 * - Matrix operations for graphics
 * - Homogeneous coordinates
 * - Transformation chaining
 * - Efficient transform calculations
 */

#include <M5Unified.h>
#include <math.h>

// Forward declarations
void displayWelcome();
void displayCurrentDemo();
void drawTouchButtons();
void drawCurrentTransformDemo();
void drawRotationDemo();
void drawScalingDemo();
void drawTranslationDemo();
void drawCombinedTransformsDemo();
void draw3DProjectionDemo();
void drawMatrixOperationsDemo();

// Demo modes for different transformation features
enum TransformDemo {
    DEMO_ROTATION,
    DEMO_SCALING,
    DEMO_TRANSLATION,
    DEMO_COMBINED_TRANSFORMS,
    DEMO_3D_PROJECTION,
    DEMO_MATRIX_OPERATIONS,
    TRANSFORM_DEMO_COUNT
};

TransformDemo currentDemo = DEMO_ROTATION;
const char* transformDemoNames[] = {
    "Rotation",
    "Scaling", 
    "Translation",
    "Combined Transforms",
    "3D Projection",
    "Matrix Operations"
};

// Animation variables
unsigned long lastUpdate = 0;
float animationAngle = 0;
int animationStep = 0;

// 3D point structure
struct Point3D {
    float x, y, z;
};

struct Point2D {
    float x, y;
};

// Simple 2D transformation matrix
struct Transform2D {
    float m[3][3]; // 3x3 homogeneous transformation matrix
};

// 3D vertices for wireframe cube
Point3D cubeVertices[8] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
};

int cubeEdges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // back face
    {4,5}, {5,6}, {6,7}, {7,4}, // front face
    {0,4}, {1,5}, {2,6}, {3,7}  // connecting edges
};

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
    M5.Display.drawString("M5GFX", M5.Display.width()/2, 150);
    M5.Display.drawString("Transforms", M5.Display.width()/2, 230);
    
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Coordinate", M5.Display.width()/2, 330);
    M5.Display.drawString("Mathematics", M5.Display.width()/2, 390);
    
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Master 2D/3D coordinate transformations", M5.Display.width()/2, 480);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Rotation, Scaling, Translation", M5.Display.width()/2, 540);
    M5.Display.drawString("• Matrix Operations", M5.Display.width()/2, 590);
    M5.Display.drawString("• 3D Projection Basics", M5.Display.width()/2, 640);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header - scaled for 1280x720
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(4);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Transformations", M5.Display.width()/2, 30);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(3);
    M5.Display.drawString(transformDemoNames[currentDemo], M5.Display.width()/2, 90);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(TRANSFORM_DEMO_COUNT), M5.Display.width()/2, 130);
    
    // Draw touch buttons
    drawTouchButtons();
    
    // Draw demo-specific content
    drawCurrentTransformDemo();
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
    
    // Reset button
    M5.Display.fillRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_NAVY);
    M5.Display.drawRoundRect(startX + btnWidth + spacing, btnY, btnWidth, btnHeight, 15, TFT_BLUE);
    M5.Display.drawString("RESET", startX + btnWidth + spacing + btnWidth/2, btnY + btnHeight/2);
    
    // Next button
    M5.Display.fillRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_DARKGREEN);
    M5.Display.drawRoundRect(startX + (btnWidth + spacing) * 2, btnY, btnWidth, btnHeight, 15, TFT_GREEN);
    M5.Display.drawString("NEXT >", startX + (btnWidth + spacing) * 2 + btnWidth/2, btnY + btnHeight/2);
}

void drawCurrentTransformDemo() {
    switch(currentDemo) {
        case DEMO_ROTATION:
            drawRotationDemo();
            break;
        case DEMO_SCALING:
            drawScalingDemo();
            break;
        case DEMO_TRANSLATION:
            drawTranslationDemo();
            break;
        case DEMO_COMBINED_TRANSFORMS:
            drawCombinedTransformsDemo();
            break;
        case DEMO_3D_PROJECTION:
            draw3DProjectionDemo();
            break;
        case DEMO_MATRIX_OPERATIONS:
            drawMatrixOperationsDemo();
            break;
    }
}

Point2D rotatePoint(Point2D point, float angle, Point2D pivot) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    // Translate to origin
    float dx = point.x - pivot.x;
    float dy = point.y - pivot.y;
    
    // Rotate
    Point2D result;
    result.x = dx * cosA - dy * sinA + pivot.x;
    result.y = dx * sinA + dy * cosA + pivot.y;
    
    return result;
}

Point2D scalePoint(Point2D point, float scaleX, float scaleY, Point2D origin) {
    Point2D result;
    result.x = origin.x + (point.x - origin.x) * scaleX;
    result.y = origin.y + (point.y - origin.y) * scaleY;
    return result;
}

void drawRotationDemo() {
    int startY = 180;  // Adjusted for header and 1280x720 display
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Rotation Transformations", 50, startY);
    
    // Simple rotation around center
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawString("Center Rotation:", 50, startY + 60);
    
    Point2D center = {200, startY + 180};
    Point2D corners[4] = {{150, startY + 130}, {250, startY + 130}, {250, startY + 230}, {150, startY + 230}};
    
    // Draw coordinate axes
    M5.Display.drawLine(center.x - 80, center.y, center.x + 80, center.y, TFT_DARKGREY);
    M5.Display.drawLine(center.x, center.y - 80, center.x, center.y + 80, TFT_DARKGREY);
    M5.Display.fillCircle(center.x, center.y, 6, TFT_RED);
    
    // Draw original rectangle
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        M5.Display.drawLine(corners[i].x, corners[i].y, corners[next].x, corners[next].y, TFT_NAVY);
    }
    
    // Draw rotated rectangle
    float angle = animationAngle * PI / 180.0;
    for (int i = 0; i < 4; i++) {
        Point2D rotated = rotatePoint(corners[i], angle, center);
        int next = (i + 1) % 4;
        Point2D rotatedNext = rotatePoint(corners[next], angle, center);
        M5.Display.drawLine(rotated.x, rotated.y, rotatedNext.x, rotatedNext.y, TFT_CYAN);
    }
    
    // Multiple pivot points
    M5.Display.drawString("Multiple Pivots:", 150, startY + 20);
    
    Point2D shape[3] = {{180, startY + 50}, {200, startY + 70}, {160, startY + 70}};
    uint16_t colors[3] = {TFT_RED, TFT_GREEN, TFT_BLUE};
    
    for (int p = 0; p < 3; p++) {
        Point2D pivot = shape[p];
        M5.Display.fillCircle(pivot.x, pivot.y, 2, colors[p]);
        
        for (int i = 0; i < 3; i++) {
            Point2D rotated = rotatePoint(shape[i], angle, pivot);
            int next = (i + 1) % 3;
            Point2D rotatedNext = rotatePoint(shape[next], angle, pivot);
            M5.Display.drawLine(rotated.x, rotated.y, rotatedNext.x, rotatedNext.y, colors[p]);
        }
    }
    
    // Rotation with trail effect
    M5.Display.drawString("Rotation Trail:", 10, startY + 120);
    
    Point2D trailCenter = {80, startY + 170};
    M5.Display.fillCircle(trailCenter.x, trailCenter.y, 2, TFT_WHITE);
    
    for (int i = 0; i < 12; i++) {
        float trailAngle = (animationAngle - i * 15) * PI / 180.0;
        Point2D start = {trailCenter.x + 20, trailCenter.y};
        Point2D end = {trailCenter.x + 35, trailCenter.y};
        
        Point2D rotatedStart = rotatePoint(start, trailAngle, trailCenter);
        Point2D rotatedEnd = rotatePoint(end, trailAngle, trailCenter);
        
        uint8_t alpha = 255 - (i * 20);
        uint16_t color = M5.Display.color565(alpha, alpha/2, alpha/4);
        M5.Display.drawLine(rotatedStart.x, rotatedStart.y, rotatedEnd.x, rotatedEnd.y, color);
    }
    
    // Information panel
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Rotation Math:", 220, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("x' = (x-px)*cos(θ) - (y-py)*sin(θ) + px", 220, startY + 35);
    M5.Display.drawString("y' = (x-px)*sin(θ) + (y-py)*cos(θ) + py", 220, startY + 50);
    M5.Display.drawString("where (px,py) is pivot point", 220, startY + 65);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Current angle: " + String((int)animationAngle) + "°", 220, startY + 85);
    M5.Display.drawString("Radians: " + String(angle, 3), 220, startY + 100);
}

void drawScalingDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Scaling Transformations", 10, startY);
    
    // Uniform scaling
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Uniform Scaling:", 10, startY + 20);
    
    Point2D scaleCenter = {80, startY + 70};
    Point2D rect[4] = {{60, startY + 50}, {100, startY + 50}, {100, startY + 90}, {60, startY + 90}};
    
    // Draw origin
    M5.Display.fillCircle(scaleCenter.x, scaleCenter.y, 3, TFT_RED);
    
    // Draw original
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        M5.Display.drawLine(rect[i].x, rect[i].y, rect[next].x, rect[next].y, TFT_NAVY);
    }
    
    // Draw scaled versions
    float scale = 0.5 + 0.8 * sin(animationStep * 0.1);
    for (int i = 0; i < 4; i++) {
        Point2D scaled = scalePoint(rect[i], scale, scale, scaleCenter);
        int next = (i + 1) % 4;
        Point2D scaledNext = scalePoint(rect[next], scale, scale, scaleCenter);
        M5.Display.drawLine(scaled.x, scaled.y, scaledNext.x, scaledNext.y, TFT_GREEN);
    }
    
    // Non-uniform scaling
    M5.Display.drawString("Non-uniform Scaling:", 150, startY + 20);
    
    Point2D shape2[4] = {{170, startY + 50}, {210, startY + 50}, {210, startY + 90}, {170, startY + 90}};
    Point2D scaleCenter2 = {190, startY + 70};
    M5.Display.fillCircle(scaleCenter2.x, scaleCenter2.y, 2, TFT_YELLOW);
    
    // Original
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        M5.Display.drawLine(shape2[i].x, shape2[i].y, shape2[next].x, shape2[next].y, TFT_NAVY);
    }
    
    // Non-uniform scaled
    float scaleX = 1.5 + 0.5 * sin(animationStep * 0.08);
    float scaleY = 0.5 + 0.5 * cos(animationStep * 0.12);
    
    for (int i = 0; i < 4; i++) {
        Point2D scaled = scalePoint(shape2[i], scaleX, scaleY, scaleCenter2);
        int next = (i + 1) % 4;
        Point2D scaledNext = scalePoint(shape2[next], scaleX, scaleY, scaleCenter2);
        M5.Display.drawLine(scaled.x, scaled.y, scaledNext.x, scaledNext.y, TFT_MAGENTA);
    }
    
    // Scale from different origins
    M5.Display.drawString("Different Origins:", 10, startY + 120);
    
    Point2D triangle[3] = {{40, startY + 150}, {80, startY + 140}, {60, startY + 180}};
    Point2D origins[3] = {{30, startY + 160}, {90, startY + 160}, {60, startY + 140}};
    uint16_t originColors[3] = {TFT_RED, TFT_GREEN, TFT_BLUE};
    
    for (int o = 0; o < 3; o++) {
        M5.Display.fillCircle(origins[o].x, origins[o].y, 2, originColors[o]);
        
        float localScale = 0.7 + 0.5 * sin(animationStep * 0.1 + o * 2);
        for (int i = 0; i < 3; i++) {
            Point2D scaled = scalePoint(triangle[i], localScale, localScale, origins[o]);
            int next = (i + 1) % 3;
            Point2D scaledNext = scalePoint(triangle[next], localScale, localScale, origins[o]);
            M5.Display.drawLine(scaled.x, scaled.y, scaledNext.x, scaledNext.y, originColors[o]);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Scaling Math:", 220, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("x' = ox + (x - ox) * sx", 220, startY + 35);
    M5.Display.drawString("y' = oy + (y - oy) * sy", 220, startY + 50);
    M5.Display.drawString("where (ox,oy) is origin", 220, startY + 65);
    M5.Display.drawString("sx, sy are scale factors", 220, startY + 80);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Uniform: " + String(scale, 2), 220, startY + 100);
    M5.Display.drawString("X-Scale: " + String(scaleX, 2), 220, startY + 115);
    M5.Display.drawString("Y-Scale: " + String(scaleY, 2), 220, startY + 130);
}

void drawTranslationDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Translation (Movement)", 10, startY);
    
    // Basic translation
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Linear Movement:", 10, startY + 20);
    
    // Moving object
    float offsetX = 50 * sin(animationStep * 0.1);
    float offsetY = 20 * cos(animationStep * 0.15);
    
    Point2D basePos = {80, startY + 70};
    Point2D translatedPos = {basePos.x + offsetX, basePos.y + offsetY};
    
    // Draw path
    for (int i = 0; i < 360; i += 10) {
        float pathX = basePos.x + 50 * sin(i * PI / 180);
        float pathY = basePos.y + 20 * cos(i * 0.15 * PI / 180);
        M5.Display.drawPixel(pathX, pathY, TFT_DARKGREY);
    }
    
    // Draw object
    M5.Display.fillCircle(translatedPos.x, translatedPos.y, 8, TFT_CYAN);
    M5.Display.drawLine(basePos.x, basePos.y, translatedPos.x, translatedPos.y, TFT_YELLOW);
    M5.Display.fillCircle(basePos.x, basePos.y, 3, TFT_RED);
    
    // Circular motion
    M5.Display.drawString("Circular Motion:", 180, startY + 20);
    
    Point2D circleCenter = {240, startY + 70};
    float radius = 30;
    float circleAngle = animationAngle * PI / 180;
    
    Point2D circlePos = {
        circleCenter.x + radius * cos(circleAngle),
        circleCenter.y + radius * sin(circleAngle)
    };
    
    // Draw circle path
    for (int i = 0; i < 360; i += 5) {
        float pathAngle = i * PI / 180;
        float px = circleCenter.x + radius * cos(pathAngle);
        float py = circleCenter.y + radius * sin(pathAngle);
        M5.Display.drawPixel(px, py, TFT_DARKGREY);
    }
    
    M5.Display.fillCircle(circleCenter.x, circleCenter.y, 2, TFT_WHITE);
    M5.Display.fillCircle(circlePos.x, circlePos.y, 6, TFT_GREEN);
    M5.Display.drawLine(circleCenter.x, circleCenter.y, circlePos.x, circlePos.y, TFT_WHITE);
    
    // Lissajous curves
    M5.Display.drawString("Lissajous Pattern:", 10, startY + 120);
    
    Point2D lissCenter = {80, startY + 170};
    
    // Draw the curve
    for (int i = 0; i < 360; i += 2) {
        float t = i * PI / 180;
        float lx = lissCenter.x + 40 * sin(3 * t);
        float ly = lissCenter.y + 30 * sin(2 * t);
        M5.Display.drawPixel(lx, ly, TFT_NAVY);
    }
    
    // Current position on curve
    float t = animationStep * 0.1;
    Point2D lissPos = {
        lissCenter.x + 40 * sin(3 * t),
        lissCenter.y + 30 * sin(2 * t)
    };
    M5.Display.fillCircle(lissPos.x, lissPos.y, 4, TFT_ORANGE);
    
    // Vector field visualization
    M5.Display.drawString("Vector Field:", 180, startY + 120);
    
    for (int x = 190; x < 290; x += 20) {
        for (int y = startY + 140; y < startY + 200; y += 15) {
            float vx = sin((x + animationStep) * 0.05) * 8;
            float vy = cos((y + animationStep) * 0.08) * 6;
            
            M5.Display.drawLine(x, y, x + vx, y + vy, TFT_CYAN);
            M5.Display.fillCircle(x + vx, y + vy, 1, TFT_WHITE);
        }
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Translation Math:", 300, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("x' = x + dx", 300, startY + 35);
    M5.Display.drawString("y' = y + dy", 300, startY + 50);
    M5.Display.drawString("Simple addition!", 300, startY + 65);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Offset X: " + String(offsetX, 1), 300, startY + 85);
    M5.Display.drawString("Offset Y: " + String(offsetY, 1), 300, startY + 100);
    M5.Display.drawString("Angle: " + String((int)animationAngle) + "°", 300, startY + 115);
}

void drawCombinedTransformsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Combined Transformations", 10, startY);
    
    // Order matters demo
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Order Matters:", 10, startY + 20);
    
    Point2D baseShape[4] = {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}};
    Point2D center1 = {80, startY + 70};
    Point2D center2 = {180, startY + 70};
    
    float angle = animationAngle * PI / 180;
    float scale = 0.8 + 0.4 * sin(animationStep * 0.1);
    Point2D translate = {20 * cos(animationStep * 0.05), 15 * sin(animationStep * 0.08)};
    
    // Method 1: Rotate -> Scale -> Translate
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("R->S->T", center1.x - 15, startY + 35);
    
    for (int i = 0; i < 4; i++) {
        Point2D p = baseShape[i];
        
        // Rotate first
        Point2D rotated;
        rotated.x = p.x * cos(angle) - p.y * sin(angle);
        rotated.y = p.x * sin(angle) + p.y * cos(angle);
        
        // Then scale
        Point2D scaled;
        scaled.x = rotated.x * scale;
        scaled.y = rotated.y * scale;
        
        // Then translate
        Point2D final;
        final.x = scaled.x + translate.x + center1.x;
        final.y = scaled.y + translate.y + center1.y;
        
        int next = (i + 1) % 4;
        Point2D pNext = baseShape[next];
        
        Point2D rotatedNext;
        rotatedNext.x = pNext.x * cos(angle) - pNext.y * sin(angle);
        rotatedNext.y = pNext.x * sin(angle) + pNext.y * cos(angle);
        
        Point2D scaledNext;
        scaledNext.x = rotatedNext.x * scale;
        scaledNext.y = rotatedNext.y * scale;
        
        Point2D finalNext;
        finalNext.x = scaledNext.x + translate.x + center1.x;
        finalNext.y = scaledNext.y + translate.y + center1.y;
        
        M5.Display.drawLine(final.x, final.y, finalNext.x, finalNext.y, TFT_GREEN);
    }
    
    // Method 2: Scale -> Rotate -> Translate
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("S->R->T", center2.x - 15, startY + 35);
    
    for (int i = 0; i < 4; i++) {
        Point2D p = baseShape[i];
        
        // Scale first
        Point2D scaled;
        scaled.x = p.x * scale;
        scaled.y = p.y * scale;
        
        // Then rotate
        Point2D rotated;
        rotated.x = scaled.x * cos(angle) - scaled.y * sin(angle);
        rotated.y = scaled.x * sin(angle) + scaled.y * cos(angle);
        
        // Then translate
        Point2D final;
        final.x = rotated.x + translate.x + center2.x;
        final.y = rotated.y + translate.y + center2.y;
        
        int next = (i + 1) % 4;
        Point2D pNext = baseShape[next];
        
        Point2D scaledNext;
        scaledNext.x = pNext.x * scale;
        scaledNext.y = pNext.y * scale;
        
        Point2D rotatedNext;
        rotatedNext.x = scaledNext.x * cos(angle) - scaledNext.y * sin(angle);
        rotatedNext.y = scaledNext.x * sin(angle) + scaledNext.y * cos(angle);
        
        Point2D finalNext;
        finalNext.x = rotatedNext.x + translate.x + center2.x;
        finalNext.y = rotatedNext.y + translate.y + center2.y;
        
        M5.Display.drawLine(final.x, final.y, finalNext.x, finalNext.y, TFT_MAGENTA);
    }
    
    // Hierarchical transformations
    M5.Display.drawString("Hierarchical Transform:", 10, startY + 120);
    
    Point2D parentCenter = {80, startY + 170};
    float parentAngle = animationStep * 0.02;
    
    // Parent object
    Point2D parentShape[3] = {{-15, -10}, {15, -10}, {0, 15}};
    for (int i = 0; i < 3; i++) {
        Point2D rotated = rotatePoint(parentShape[i], parentAngle, {0, 0});
        rotated.x += parentCenter.x;
        rotated.y += parentCenter.y;
        
        int next = (i + 1) % 3;
        Point2D rotatedNext = rotatePoint(parentShape[next], parentAngle, {0, 0});
        rotatedNext.x += parentCenter.x;
        rotatedNext.y += parentCenter.y;
        
        M5.Display.drawLine(rotated.x, rotated.y, rotatedNext.x, rotatedNext.y, TFT_BLUE);
    }
    
    // Child object (orbiting around parent tip)
    Point2D localChildPos = {0, -20}; // Relative to parent tip
    Point2D parentTip = rotatePoint({0, 15}, parentAngle, {0, 0});
    parentTip.x += parentCenter.x;
    parentTip.y += parentCenter.y;
    
    float childAngle = animationStep * 0.1;
    Point2D childShape[4] = {{-5, -5}, {5, -5}, {5, 5}, {-5, 5}};
    
    for (int i = 0; i < 4; i++) {
        // Transform child relative to parent
        Point2D childPoint = rotatePoint(childShape[i], childAngle, {0, 0});
        childPoint.x += localChildPos.x;
        childPoint.y += localChildPos.y;
        childPoint = rotatePoint(childPoint, parentAngle, {0, 0});
        childPoint.x += parentCenter.x;
        childPoint.y += parentCenter.y;
        
        int next = (i + 1) % 4;
        Point2D childPointNext = rotatePoint(childShape[next], childAngle, {0, 0});
        childPointNext.x += localChildPos.x;
        childPointNext.y += localChildPos.y;
        childPointNext = rotatePoint(childPointNext, parentAngle, {0, 0});
        childPointNext.x += parentCenter.x;
        childPointNext.y += parentCenter.y;
        
        M5.Display.drawLine(childPoint.x, childPoint.y, childPointNext.x, childPointNext.y, TFT_RED);
    }
    
    // Draw connection
    M5.Display.drawLine(parentTip.x, parentTip.y, 
                       parentTip.x + rotatePoint(localChildPos, parentAngle, {0, 0}).x,
                       parentTip.y + rotatePoint(localChildPos, parentAngle, {0, 0}).y, TFT_YELLOW);
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Transform Tips:", 220, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Order of operations matters", 220, startY + 35);
    M5.Display.drawString("• Use matrices for efficiency", 220, startY + 50);
    M5.Display.drawString("• Hierarchical = parent-child", 220, startY + 65);
    M5.Display.drawString("• Combine transforms carefully", 220, startY + 80);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Scale: " + String(scale, 2), 220, startY + 100);
    M5.Display.drawString("Angle: " + String((int)animationAngle) + "°", 220, startY + 115);
    M5.Display.drawString("Parent: " + String(parentAngle, 2), 220, startY + 130);
    M5.Display.drawString("Child: " + String(childAngle, 2), 220, startY + 145);
}

Point2D project3D(Point3D point3d, float distance) {
    Point2D result;
    if (point3d.z + distance != 0) {
        result.x = (point3d.x * distance) / (point3d.z + distance);
        result.y = (point3d.y * distance) / (point3d.z + distance);
    } else {
        result.x = point3d.x * 1000; // Very far away
        result.y = point3d.y * 1000;
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

void draw3DProjectionDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("3D Projection", 10, startY);
    
    // 3D wireframe cube
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Wireframe Cube:", 10, startY + 20);
    
    Point2D center = {120, startY + 100};
    float distance = 4; // Viewer distance
    
    // Rotate the cube
    float angleX = animationStep * 0.02;
    float angleY = animationStep * 0.03;
    float angleZ = animationStep * 0.01;
    
    // Transform and project vertices
    Point2D projectedVertices[8];
    for (int i = 0; i < 8; i++) {
        Point3D vertex = cubeVertices[i];
        
        // Apply rotations
        vertex = rotateX(vertex, angleX);
        vertex = rotateY(vertex, angleY);
        vertex = rotateZ(vertex, angleZ);
        
        // Scale for display
        vertex.x *= 50;
        vertex.y *= 50;
        vertex.z *= 50;
        vertex.z += 200; // Move away from camera
        
        // Project to 2D
        projectedVertices[i] = project3D(vertex, distance * 50);
        projectedVertices[i].x += center.x;
        projectedVertices[i].y += center.y;
    }
    
    // Draw edges
    for (int i = 0; i < 12; i++) {
        int v1 = cubeEdges[i][0];
        int v2 = cubeEdges[i][1];
        
        // Color based on depth
        float avgZ = (cubeVertices[v1].z + cubeVertices[v2].z) / 2;
        uint8_t intensity = 100 + 155 * (avgZ + 1) / 2;
        uint16_t color = M5.Display.color565(intensity, intensity, intensity);
        
        M5.Display.drawLine(projectedVertices[v1].x, projectedVertices[v1].y,
                           projectedVertices[v2].x, projectedVertices[v2].y, color);
    }
    
    // Draw vertices
    for (int i = 0; i < 8; i++) {
        M5.Display.fillCircle(projectedVertices[i].x, projectedVertices[i].y, 2, TFT_RED);
    }
    
    // Perspective vs orthographic
    M5.Display.drawString("Orthographic:", 10, startY + 190);
    
    Point2D orthoCenter = {80, startY + 230};
    for (int i = 0; i < 8; i++) {
        Point3D vertex = cubeVertices[i];
        vertex = rotateY(vertex, angleY);
        
        // Orthographic projection (just ignore Z)
        Point2D orthoVertex;
        orthoVertex.x = vertex.x * 20 + orthoCenter.x;
        orthoVertex.y = vertex.y * 20 + orthoCenter.y;
        
        M5.Display.fillCircle(orthoVertex.x, orthoVertex.y, 1, TFT_BLUE);
    }
    
    // Multiple objects at different depths
    M5.Display.drawString("Depth Test:", 180, startY + 190);
    
    for (int depth = 0; depth < 5; depth++) {
        Point3D obj = {0, 0, depth * 50 + 100};
        obj = rotateY(obj, angleY + depth * 0.5);
        obj.x += sin(angleY + depth) * 30;
        obj.y += cos(angleY + depth) * 20;
        
        Point2D proj = project3D(obj, 200);
        proj.x += 220;
        proj.y += startY + 220;
        
        uint8_t size = 10 - depth * 2;
        uint8_t intensity = 255 - depth * 40;
        uint16_t color = M5.Display.color565(intensity, intensity/2, intensity/4);
        
        M5.Display.fillCircle(proj.x, proj.y, size, color);
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("3D Projection Math:", 280, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Perspective:", 280, startY + 35);
    M5.Display.drawString("x' = (x * d) / (z + d)", 280, startY + 50);
    M5.Display.drawString("y' = (y * d) / (z + d)", 280, startY + 65);
    M5.Display.drawString("where d = viewer distance", 280, startY + 80);
    
    M5.Display.drawString("Orthographic:", 280, startY + 100);
    M5.Display.drawString("x' = x", 280, startY + 115);
    M5.Display.drawString("y' = y", 280, startY + 130);
    M5.Display.drawString("(ignore z)", 280, startY + 145);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Rotation X: " + String(angleX, 2), 280, startY + 165);
    M5.Display.drawString("Rotation Y: " + String(angleY, 2), 280, startY + 180);
    M5.Display.drawString("Rotation Z: " + String(angleZ, 2), 280, startY + 195);
}

void drawMatrixOperationsDemo() {
    int startY = 70;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Matrix Operations", 10, startY);
    
    // Show transformation matrices
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Transformation Matrices:", 10, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Translation matrix
    M5.Display.drawString("Translation:", 10, startY + 40);
    M5.Display.drawString("[1  0  tx]", 10, startY + 55);
    M5.Display.drawString("[0  1  ty]", 10, startY + 70);
    M5.Display.drawString("[0  0  1 ]", 10, startY + 85);
    
    // Rotation matrix
    M5.Display.drawString("Rotation:", 120, startY + 40);
    M5.Display.drawString("[cos -sin 0]", 120, startY + 55);
    M5.Display.drawString("[sin  cos 0]", 120, startY + 70);
    M5.Display.drawString("[0    0   1]", 120, startY + 85);
    
    // Scale matrix
    M5.Display.drawString("Scale:", 220, startY + 40);
    M5.Display.drawString("[sx  0  0]", 220, startY + 55);
    M5.Display.drawString("[0  sy  0]", 220, startY + 70);
    M5.Display.drawString("[0   0  1]", 220, startY + 85);
    
    // Matrix multiplication example
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Matrix Multiplication:", 10, startY + 110);
    M5.Display.setTextColor(TFT_WHITE);
    
    // Show combined matrix (simplified example)
    float angle = animationAngle * PI / 180;
    float tx = 20, ty = 10;
    float scale = 1.5;
    
    M5.Display.drawString("Combined (S*R*T):", 10, startY + 130);
    String row1 = "[" + String(scale * cos(angle), 2) + " " + String(-scale * sin(angle), 2) + " " + String(tx, 1) + "]";
    String row2 = "[" + String(scale * sin(angle), 2) + " " + String(scale * cos(angle), 2) + " " + String(ty, 1) + "]";
    String row3 = "[0.00  0.00  1.0]";
    
    M5.Display.drawString(row1, 10, startY + 145);
    M5.Display.drawString(row2, 10, startY + 160);
    M5.Display.drawString(row3, 10, startY + 175);
    
    // Demonstrate matrix transform on shape
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Matrix Transform Demo:", 200, startY + 110);
    
    Point2D center = {280, startY + 160};
    Point2D square[4] = {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}};
    
    // Apply matrix transformation
    for (int i = 0; i < 4; i++) {
        Point2D p = square[i];
        
        // Manual matrix multiplication [x' y' 1] = [x y 1] * Matrix
        float x_new = p.x * scale * cos(angle) - p.y * scale * sin(angle) + tx;
        float y_new = p.x * scale * sin(angle) + p.y * scale * cos(angle) + ty;
        
        int next = (i + 1) % 4;
        Point2D p_next = square[next];
        float x_new_next = p_next.x * scale * cos(angle) - p_next.y * scale * sin(angle) + tx;
        float y_new_next = p_next.x * scale * sin(angle) + p_next.y * scale * cos(angle) + ty;
        
        M5.Display.drawLine(center.x + x_new, center.y + y_new,
                           center.x + x_new_next, center.y + y_new_next, TFT_GREEN);
    }
    
    // Original for comparison
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        M5.Display.drawLine(center.x + square[i].x, center.y + square[i].y,
                           center.x + square[next].x, center.y + square[next].y, TFT_NAVY);
    }
    
    // Performance comparison
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Performance Benefits:", 10, startY + 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Single matrix vs multiple operations", 10, startY + 215);
    M5.Display.drawString("• GPU hardware acceleration", 10, startY + 230);
    M5.Display.drawString("• Consistent numerical precision", 10, startY + 245);
    
    // Current values
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Current Values:", 250, startY + 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Angle: " + String((int)animationAngle) + "°", 250, startY + 215);
    M5.Display.drawString("Scale: " + String(scale), 250, startY + 230);
    M5.Display.drawString("Translate: " + String(tx) + "," + String(ty), 250, startY + 245);
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
                    currentDemo = (TransformDemo)((currentDemo - 1 + TRANSFORM_DEMO_COUNT) % TRANSFORM_DEMO_COUNT);
                    displayCurrentDemo();
                } else if (touch.x >= startX + btnWidth + spacing && 
                          touch.x <= startX + btnWidth + spacing + btnWidth) {
                    // Reset button
                    animationStep = 0;
                    animationAngle = 0;
                    displayCurrentDemo();
                } else if (touch.x >= startX + (btnWidth + spacing) * 2 && 
                          touch.x <= startX + (btnWidth + spacing) * 2 + btnWidth) {
                    // Next button
                    currentDemo = (TransformDemo)((currentDemo + 1) % TRANSFORM_DEMO_COUNT);
                    displayCurrentDemo();
                }
            }
        }
    }
    
    // Also handle physical buttons if available
    if (M5.BtnA.wasPressed()) {
        currentDemo = (TransformDemo)((currentDemo - 1 + TRANSFORM_DEMO_COUNT) % TRANSFORM_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        animationStep = 0;
        animationAngle = 0;
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (TransformDemo)((currentDemo + 1) % TRANSFORM_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS
        animationAngle += 3;
        if (animationAngle >= 360) animationAngle = 0;
        
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        
        // Redraw all demos as they all have animation
        drawCurrentTransformDemo();
        
        lastUpdate = millis();
    }
}