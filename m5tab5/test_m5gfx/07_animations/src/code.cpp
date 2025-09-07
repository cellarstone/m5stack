/*
 * M5GFX Tutorial - 07_animations
 * 
 * This demo demonstrates advanced animation techniques in M5GFX:
 * - Frame timing and smooth animation
 * - Easing functions for natural motion
 * - Particle systems and physics
 * - Scrolling effects and parallax
 * - Smooth transitions and interpolation
 * - Animation states and sequencing
 * - Performance optimization for animations
 * - Interactive and responsive animations
 * 
 * Key concepts:
 * - Frame rate control and timing
 * - Mathematical easing curves
 * - State machines for animation
 * - Memory-efficient particle systems
 * - Smooth interpolation techniques
 */

#include <M5Unified.h>
#include <math.h>

// Demo modes for different animation techniques
enum AnimationDemo {
    DEMO_EASING_FUNCTIONS,
    DEMO_PARTICLE_SYSTEMS,
    DEMO_SCROLLING_PARALLAX,
    DEMO_SMOOTH_TRANSITIONS,
    DEMO_PHYSICS_SIMULATION,
    DEMO_SEQUENCED_ANIMATIONS,
    ANIMATION_DEMO_COUNT
};

AnimationDemo currentDemo = DEMO_EASING_FUNCTIONS;
const char* animationDemoNames[] = {
    "Easing Functions",
    "Particle Systems",
    "Scrolling & Parallax",
    "Smooth Transitions",
    "Physics Simulation",
    "Sequenced Animations"
};

// Animation timing
unsigned long lastUpdate = 0;
unsigned long frameStartTime = 0;
float deltaTime = 0;
int frameCount = 0;
float fps = 0;

// Animation parameters
float animationTime = 0;
int animationStep = 0;
bool animationDirection = true;

// Easing functions
float easeLinear(float t) { return t; }
float easeInQuad(float t) { return t * t; }
float easeOutQuad(float t) { return t * (2 - t); }
float easeInOutQuad(float t) { return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t; }
float easeInCubic(float t) { return t * t * t; }
float easeOutCubic(float t) { return (--t) * t * t + 1; }
float easeInOutCubic(float t) { return t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1; }
float easeInSine(float t) { return 1 - cos(t * PI / 2); }
float easeOutSine(float t) { return sin(t * PI / 2); }
float easeInOutSine(float t) { return -(cos(PI * t) - 1) / 2; }
float easeInElastic(float t) { 
    if (t == 0) return 0;
    if (t == 1) return 1;
    float p = 0.3;
    float s = p / 4;
    return -(pow(2, 10 * (t -= 1)) * sin((t - s) * (2 * PI) / p));
}
float easeOutBounce(float t) {
    if (t < (1/2.75)) return (7.5625 * t * t);
    else if (t < (2/2.75)) return (7.5625 * (t -= (1.5/2.75)) * t + 0.75);
    else if (t < (2.5/2.75)) return (7.5625 * (t -= (2.25/2.75)) * t + 0.9375);
    else return (7.5625 * (t -= (2.625/2.75)) * t + 0.984375);
}

// Particle system
struct Particle {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    uint16_t color;
    float size;
    bool active;
};

const int MAX_PARTICLES = 50;
Particle particles[MAX_PARTICLES];
int activeParticles = 0;

// Physics objects
struct PhysicsObject {
    float x, y;
    float vx, vy;
    float ax, ay;
    float mass;
    float bounce;
    uint16_t color;
    float size;
    bool active;
};

const int MAX_PHYSICS_OBJECTS = 10;
PhysicsObject physicsObjects[MAX_PHYSICS_OBJECTS];

// Scrolling background layers
struct ScrollLayer {
    float x;
    float speed;
    uint16_t color;
    int height;
    int pattern;
};

ScrollLayer scrollLayers[4];

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize particle system
    initParticleSystem();
    
    // Initialize physics objects
    initPhysicsObjects();
    
    // Initialize scroll layers
    initScrollLayers();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Start with first demo
    displayCurrentDemo();
}

void initParticleSystem() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

void initPhysicsObjects() {
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        physicsObjects[i].x = 50 + i * 30;
        physicsObjects[i].y = 100 + random(50);
        physicsObjects[i].vx = (random(200) - 100) / 50.0;
        physicsObjects[i].vy = (random(100) - 50) / 50.0;
        physicsObjects[i].ax = 0;
        physicsObjects[i].ay = 0.2; // Gravity
        physicsObjects[i].mass = 1 + random(3);
        physicsObjects[i].bounce = 0.7 + random(30) / 100.0;
        physicsObjects[i].color = M5.Display.color565(random(255), random(255), random(255));
        physicsObjects[i].size = 3 + random(7);
        physicsObjects[i].active = true;
    }
}

void initScrollLayers() {
    scrollLayers[0] = {0, 0.5, TFT_DARKBLUE, 20, 0};    // Sky
    scrollLayers[1] = {0, 1.0, TFT_DARKGREEN, 30, 1};   // Mountains
    scrollLayers[2] = {0, 2.0, TFT_GREEN, 40, 2};       // Hills
    scrollLayers[3] = {0, 4.0, TFT_BROWN, 25, 3};       // Ground
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5GFX", M5.Display.width()/2, 40);
    M5.Display.drawString("Animation", M5.Display.width()/2, 75);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Advanced", M5.Display.width()/2, 110);
    M5.Display.drawString("Techniques", M5.Display.width()/2, 135);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Professional animation and motion graphics", M5.Display.width()/2, 170);
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Easing Functions & Smooth Motion", M5.Display.width()/2, 190);
    M5.Display.drawString("• Particle Systems & Physics", M5.Display.width()/2, 205);
    M5.Display.drawString("• Parallax & Advanced Effects", M5.Display.width()/2, 220);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Animation", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1);
    M5.Display.drawString(animationDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(ANIMATION_DEMO_COUNT), M5.Display.width()/2, 50);
    
    // Performance info
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("FPS: " + String((int)fps) + " | Frame: " + String(frameCount), M5.Display.width()/2, 65);
    
    // Navigation hint
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("[A] Prev  [B] Reset  [C] Next", M5.Display.width()/2, M5.Display.height() - 10);
    
    // Draw demo-specific content
    drawCurrentAnimationDemo();
}

void drawCurrentAnimationDemo() {
    switch(currentDemo) {
        case DEMO_EASING_FUNCTIONS:
            drawEasingFunctionsDemo();
            break;
        case DEMO_PARTICLE_SYSTEMS:
            drawParticleSystemsDemo();
            break;
        case DEMO_SCROLLING_PARALLAX:
            drawScrollingParallaxDemo();
            break;
        case DEMO_SMOOTH_TRANSITIONS:
            drawSmoothTransitionsDemo();
            break;
        case DEMO_PHYSICS_SIMULATION:
            drawPhysicsSimulationDemo();
            break;
        case DEMO_SEQUENCED_ANIMATIONS:
            drawSequencedAnimationsDemo();
            break;
    }
}

void drawEasingFunctionsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Easing Function Comparison", 10, startY);
    
    // Animation parameter (0 to 1 and back)
    float t = (sin(animationTime * 2) + 1) / 2; // 0 to 1
    
    // Draw easing curves and moving objects
    const int numEasings = 6;
    const char* easingNames[] = {"Linear", "Quad In", "Quad Out", "Cubic", "Sine", "Bounce"};
    float (*easingFunctions[])(float) = {easeLinear, easeInQuad, easeOutQuad, easeInCubic, easeInSine, easeOutBounce};
    uint16_t easingColors[] = {TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_MAGENTA};
    
    // Draw reference timeline
    int timelineY = startY + 30;
    int timelineLength = M5.Display.width() - 40;
    M5.Display.drawLine(20, timelineY, 20 + timelineLength, timelineY, TFT_DARKGREY);
    
    // Draw time marker
    int timeMarker = 20 + t * timelineLength;
    M5.Display.drawLine(timeMarker, timelineY - 5, timeMarker, timelineY + 5, TFT_CYAN);
    
    // Draw easing demonstrations
    for (int i = 0; i < numEasings; i++) {
        int y = startY + 50 + i * 25;
        
        // Function name
        M5.Display.setTextColor(easingColors[i]);
        M5.Display.drawString(easingNames[i], 10, y);
        
        // Draw path line
        M5.Display.drawLine(100, y + 8, 100 + timelineLength - 80, y + 8, TFT_DARKGREY);
        
        // Calculate eased position
        float easedT = easingFunctions[i](t);
        int objX = 100 + easedT * (timelineLength - 80);
        
        // Draw moving object
        M5.Display.fillCircle(objX, y + 8, 5, easingColors[i]);
        
        // Draw mini curve visualization
        for (int x = 0; x < 50; x++) {
            float curveT = x / 49.0;
            float curveValue = easingFunctions[i](curveT);
            int curveY = y + 8 - curveValue * 15;
            M5.Display.drawPixel(300 + x, curveY, easingColors[i]);
        }
    }
    
    // Information panel
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Easing Benefits:", 360, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Natural motion feel", 360, startY + 35);
    M5.Display.drawString("• Professional animations", 360, startY + 50);
    M5.Display.drawString("• UI/UX improvements", 360, startY + 65);
    M5.Display.drawString("• Attention guidance", 360, startY + 80);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Time: " + String(t, 3), 360, startY + 100);
    M5.Display.drawString("Cycle: " + String(animationTime, 1), 360, startY + 115);
}

void spawnParticle(float x, float y, float vx, float vy, float life, uint16_t color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = vx;
            particles[i].vy = vy;
            particles[i].life = life;
            particles[i].maxLife = life;
            particles[i].color = color;
            particles[i].size = 1 + random(4);
            particles[i].active = true;
            activeParticles++;
            break;
        }
    }
}

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            // Update position
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            
            // Apply gravity
            particles[i].vy += 0.1;
            
            // Apply air resistance
            particles[i].vx *= 0.99;
            particles[i].vy *= 0.99;
            
            // Update life
            particles[i].life -= deltaTime * 20;
            
            // Boundary checking
            if (particles[i].x < 0 || particles[i].x >= M5.Display.width() ||
                particles[i].y < 85 || particles[i].y >= M5.Display.height() - 20 ||
                particles[i].life <= 0) {
                particles[i].active = false;
                activeParticles--;
            }
        }
    }
}

void drawParticleSystemsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Particle Systems", 10, startY);
    
    // Spawn particles from different emitters
    if (frameCount % 3 == 0) {
        // Fire effect
        spawnParticle(80, M5.Display.height() - 50, 
                     (random(40) - 20) / 10.0, -random(30) / 10.0 - 2,
                     30 + random(20), 
                     M5.Display.color565(255, random(100) + 100, 0));
        
        // Fountain effect
        float angle = random(60) - 30 + 270; // -30 to +30 degrees from up
        float speed = 3 + random(20) / 10.0;
        spawnParticle(200, M5.Display.height() - 50,
                     speed * cos(angle * PI / 180),
                     speed * sin(angle * PI / 180),
                     40 + random(30),
                     TFT_CYAN);
        
        // Explosion effect (triggered periodically)
        if (frameCount % 60 == 0) {
            for (int i = 0; i < 15; i++) {
                float expAngle = random(360) * PI / 180;
                float expSpeed = 1 + random(40) / 10.0;
                spawnParticle(320, startY + 80,
                             expSpeed * cos(expAngle),
                             expSpeed * sin(expAngle),
                             20 + random(20),
                             M5.Display.color565(255, random(255), 0));
            }
        }
    }
    
    // Update and draw particles
    updateParticles();
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            // Calculate alpha based on life
            float alpha = particles[i].life / particles[i].maxLife;
            
            // Fade color based on life
            uint8_t r = ((particles[i].color >> 11) & 0x1F) * 8 * alpha;
            uint8_t g = ((particles[i].color >> 5) & 0x3F) * 4 * alpha;
            uint8_t b = (particles[i].color & 0x1F) * 8 * alpha;
            
            uint16_t fadedColor = M5.Display.color565(r, g, b);
            
            // Draw particle
            if (particles[i].size > 2) {
                M5.Display.fillCircle(particles[i].x, particles[i].y, particles[i].size * alpha, fadedColor);
            } else {
                M5.Display.drawPixel(particles[i].x, particles[i].y, fadedColor);
            }
        }
    }
    
    // Draw emitter labels and info
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Fire", 60, M5.Display.height() - 40);
    M5.Display.drawString("Fountain", 170, M5.Display.height() - 40);
    M5.Display.drawString("Explosion", 280, startY + 60);
    
    // Particle system info
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Active Particles: " + String(activeParticles), 10, startY + 20);
    M5.Display.drawString("Max Particles: " + String(MAX_PARTICLES), 10, startY + 35);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Particle Effects:", 200, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Physics simulation", 200, startY + 35);
    M5.Display.drawString("• Life/death cycles", 200, startY + 50);
    M5.Display.drawString("• Color fading", 200, startY + 65);
    M5.Display.drawString("• Multiple emitters", 200, startY + 80);
}

void drawScrollingParallaxDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Scrolling & Parallax", 10, startY);
    
    // Update scroll positions
    for (int i = 0; i < 4; i++) {
        scrollLayers[i].x -= scrollLayers[i].speed;
        if (scrollLayers[i].x <= -M5.Display.width()) {
            scrollLayers[i].x = 0;
        }
    }
    
    // Draw scrolling layers (background to foreground)
    int layerY = startY + 30;
    
    for (int layer = 0; layer < 4; layer++) {
        int y = layerY + layer * (scrollLayers[layer].height + 5);
        
        // Draw two copies for seamless scrolling
        for (int copy = 0; copy < 2; copy++) {
            int baseX = scrollLayers[layer].x + copy * M5.Display.width();
            
            // Draw layer pattern based on type
            switch (scrollLayers[layer].pattern) {
                case 0: // Sky - stars
                    for (int i = 0; i < 20; i++) {
                        int starX = baseX + (i * 23) % M5.Display.width();
                        int starY = y + (i * 7) % scrollLayers[layer].height;
                        M5.Display.drawPixel(starX, starY, TFT_WHITE);
                    }
                    break;
                    
                case 1: // Mountains - triangular peaks
                    for (int x = 0; x < M5.Display.width(); x += 40) {
                        int peakX = baseX + x;
                        int peakHeight = 10 + (x % 20);
                        for (int h = 0; h < peakHeight; h++) {
                            int width = (peakHeight - h) * 2;
                            M5.Display.drawLine(peakX - width/2, y + h, 
                                              peakX + width/2, y + h, 
                                              scrollLayers[layer].color);
                        }
                    }
                    break;
                    
                case 2: // Hills - sine wave
                    for (int x = baseX; x < baseX + M5.Display.width(); x++) {
                        int hillHeight = 15 + 10 * sin((x + animationTime * 10) * 0.02);
                        M5.Display.drawLine(x, y, x, y + hillHeight, scrollLayers[layer].color);
                    }
                    break;
                    
                case 3: // Ground - blocks
                    for (int x = baseX; x < baseX + M5.Display.width(); x += 15) {
                        M5.Display.fillRect(x, y, 12, scrollLayers[layer].height, scrollLayers[layer].color);
                        M5.Display.drawRect(x, y, 12, scrollLayers[layer].height, TFT_BLACK);
                    }
                    break;
            }
        }
        
        // Layer label
        M5.Display.setTextColor(TFT_WHITE);
        String label = "Layer " + String(layer + 1) + " (x" + String(scrollLayers[layer].speed, 1) + ")";
        M5.Display.drawString(label, 10, y - 15);
    }
    
    // Infinite scrolling text
    int textY = startY + 200;
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Infinite Scroll:", 10, textY - 15);
    
    String scrollText = "This text scrolls infinitely across the screen demonstrating smooth scrolling techniques... ";
    int textWidth = M5.Display.textWidth(scrollText);
    int scrollOffset = (-animationStep * 2) % (textWidth + M5.Display.width());
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString(scrollText, scrollOffset, textY);
    M5.Display.drawString(scrollText, scrollOffset + textWidth + M5.Display.width(), textY);
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Parallax Principles:", 350, startY + 30);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Distant = slower", 350, startY + 45);
    M5.Display.drawString("• Near = faster", 350, startY + 60);
    M5.Display.drawString("• Creates depth illusion", 350, startY + 75);
    M5.Display.drawString("• Seamless wrapping", 350, startY + 90);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Scroll Speed:", 350, startY + 110);
    for (int i = 0; i < 4; i++) {
        M5.Display.drawString("L" + String(i+1) + ": " + String(scrollLayers[i].speed, 1) + "px/f", 
                             350, startY + 125 + i * 12);
    }
}

void drawSmoothTransitionsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Smooth Transitions", 10, startY);
    
    // State transition demo
    static int currentState = 0;
    static float transitionProgress = 0;
    static bool transitioning = false;
    
    if (frameCount % 180 == 0 && !transitioning) {
        transitioning = true;
        transitionProgress = 0;
    }
    
    if (transitioning) {
        transitionProgress += 0.02;
        if (transitionProgress >= 1.0) {
            transitionProgress = 1.0;
            transitioning = false;
            currentState = (currentState + 1) % 4;
        }
    }
    
    // Color transition
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Color Interpolation:", 10, startY + 20);
    
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW};
    uint16_t currentColor = colors[currentState];
    uint16_t nextColor = colors[(currentState + 1) % 4];
    
    // Interpolate RGB components
    uint8_t r1 = (currentColor >> 11) & 0x1F;
    uint8_t g1 = (currentColor >> 5) & 0x3F;
    uint8_t b1 = currentColor & 0x1F;
    
    uint8_t r2 = (nextColor >> 11) & 0x1F;
    uint8_t g2 = (nextColor >> 5) & 0x3F;
    uint8_t b2 = nextColor & 0x1F;
    
    float easedProgress = easeInOutCubic(transitionProgress);
    uint8_t r = r1 + (r2 - r1) * easedProgress;
    uint8_t g = g1 + (g2 - g1) * easedProgress;
    uint8_t b = b1 + (b2 - b1) * easedProgress;
    
    uint16_t interpolatedColor = (r << 11) | (g << 5) | b;
    M5.Display.fillRoundRect(10, startY + 40, 80, 30, 5, interpolatedColor);
    
    // Size transition
    M5.Display.drawString("Size Transition:", 110, startY + 20);
    float size = 10 + 20 * easeInOutSine(sin(animationTime * 3) * 0.5 + 0.5);
    M5.Display.fillCircle(150, startY + 55, size, TFT_CYAN);
    
    // Position transition
    M5.Display.drawString("Position Smoothing:", 200, startY + 20);
    
    static float targetX = 250;
    static float currentX = 250;
    static float targetY = startY + 55;
    static float currentY = startY + 55;
    
    // Move target occasionally
    if (frameCount % 120 == 0) {
        targetX = 220 + random(60);
        targetY = startY + 40 + random(30);
    }
    
    // Smooth following with lerp
    float lerpSpeed = 0.05;
    currentX += (targetX - currentX) * lerpSpeed;
    currentY += (targetY - currentY) * lerpSpeed;
    
    M5.Display.fillCircle(targetX, targetY, 3, TFT_RED);      // Target
    M5.Display.fillCircle(currentX, currentY, 8, TFT_GREEN);  // Follower
    M5.Display.drawLine(targetX, targetY, currentX, currentY, TFT_YELLOW);
    
    // Opacity transition
    M5.Display.drawString("Opacity Fade:", 10, startY + 90);
    
    float opacity = (sin(animationTime * 2) + 1) / 2; // 0 to 1
    for (int i = 0; i < 5; i++) {
        uint8_t alpha = 255 * opacity;
        uint16_t fadeColor = M5.Display.color565(alpha, alpha/2, alpha/4);
        M5.Display.fillRect(10 + i * 16, startY + 110, 12, 20, fadeColor);
    }
    
    // Morphing shapes
    M5.Display.drawString("Shape Morphing:", 150, startY + 90);
    
    float morphProgress = (sin(animationTime) + 1) / 2;
    int centerX = 190, centerY = startY + 120;
    int numPoints = 8;
    
    for (int i = 0; i < numPoints; i++) {
        float angle = i * 2 * PI / numPoints;
        float radius1 = 15; // Circle
        float radius2 = 10 + 10 * (i % 2); // Star
        
        float currentRadius = radius1 + (radius2 - radius1) * easeInOutSine(morphProgress);
        int x = centerX + currentRadius * cos(angle);
        int y = centerY + currentRadius * sin(angle);
        
        int nextI = (i + 1) % numPoints;
        float nextAngle = nextI * 2 * PI / numPoints;
        float nextRadius = radius1 + (radius2 - radius1) * easeInOutSine(morphProgress);
        int nextX = centerX + nextRadius * cos(nextAngle);
        int nextY = centerY + nextRadius * sin(nextAngle);
        
        M5.Display.drawLine(x, y, nextX, nextY, TFT_MAGENTA);
    }
    
    // UI State transitions
    M5.Display.drawString("UI State Changes:", 250, startY + 90);
    
    const char* states[] = {"Loading", "Ready", "Active", "Complete"};
    uint16_t stateColors[] = {TFT_ORANGE, TFT_GREEN, TFT_BLUE, TFT_PURPLE};
    
    M5.Display.fillRoundRect(250, startY + 110, 100, 20, 3, stateColors[currentState]);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(states[currentState], 300, startY + 120);
    
    // Progress bar for transition
    if (transitioning) {
        int barWidth = 100 * transitionProgress;
        M5.Display.fillRect(250, startY + 135, barWidth, 4, TFT_CYAN);
    }
    
    // Information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Transition Types:", 10, startY + 150);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Color interpolation", 10, startY + 165);
    M5.Display.drawString("• Size/scale changes", 10, startY + 180);
    M5.Display.drawString("• Position smoothing", 10, startY + 195);
    M5.Display.drawString("• Opacity fading", 10, startY + 210);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Progress: " + String((int)(transitionProgress * 100)) + "%", 180, startY + 150);
    M5.Display.drawString("State: " + String(states[currentState]), 180, startY + 165);
    M5.Display.drawString("Lerp Speed: 0.05", 180, startY + 180);
    M5.Display.drawString("Opacity: " + String(opacity, 2), 180, startY + 195);
}

void updatePhysics() {
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (physicsObjects[i].active) {
            // Update velocity with acceleration
            physicsObjects[i].vx += physicsObjects[i].ax;
            physicsObjects[i].vy += physicsObjects[i].ay;
            
            // Update position
            physicsObjects[i].x += physicsObjects[i].vx;
            physicsObjects[i].y += physicsObjects[i].vy;
            
            // Boundary collisions
            if (physicsObjects[i].x - physicsObjects[i].size <= 0) {
                physicsObjects[i].x = physicsObjects[i].size;
                physicsObjects[i].vx = -physicsObjects[i].vx * physicsObjects[i].bounce;
            }
            if (physicsObjects[i].x + physicsObjects[i].size >= M5.Display.width()) {
                physicsObjects[i].x = M5.Display.width() - physicsObjects[i].size;
                physicsObjects[i].vx = -physicsObjects[i].vx * physicsObjects[i].bounce;
            }
            if (physicsObjects[i].y - physicsObjects[i].size <= 85) {
                physicsObjects[i].y = 85 + physicsObjects[i].size;
                physicsObjects[i].vy = -physicsObjects[i].vy * physicsObjects[i].bounce;
            }
            if (physicsObjects[i].y + physicsObjects[i].size >= M5.Display.height() - 20) {
                physicsObjects[i].y = M5.Display.height() - 20 - physicsObjects[i].size;
                physicsObjects[i].vy = -physicsObjects[i].vy * physicsObjects[i].bounce;
                // Ground friction
                physicsObjects[i].vx *= 0.9;
            }
            
            // Object-to-object collisions
            for (int j = i + 1; j < MAX_PHYSICS_OBJECTS; j++) {
                if (physicsObjects[j].active) {
                    float dx = physicsObjects[j].x - physicsObjects[i].x;
                    float dy = physicsObjects[j].y - physicsObjects[i].y;
                    float distance = sqrt(dx*dx + dy*dy);
                    float minDistance = physicsObjects[i].size + physicsObjects[j].size;
                    
                    if (distance < minDistance && distance > 0) {
                        // Normalize collision vector
                        dx /= distance;
                        dy /= distance;
                        
                        // Separate objects
                        float overlap = minDistance - distance;
                        physicsObjects[i].x -= dx * overlap * 0.5;
                        physicsObjects[i].y -= dy * overlap * 0.5;
                        physicsObjects[j].x += dx * overlap * 0.5;
                        physicsObjects[j].y += dy * overlap * 0.5;
                        
                        // Calculate relative velocity
                        float relVelX = physicsObjects[j].vx - physicsObjects[i].vx;
                        float relVelY = physicsObjects[j].vy - physicsObjects[i].vy;
                        float speed = relVelX * dx + relVelY * dy;
                        
                        // Do not resolve if velocities are separating
                        if (speed < 0) continue;
                        
                        // Collision impulse
                        float totalMass = physicsObjects[i].mass + physicsObjects[j].mass;
                        float impulse = 2 * speed / totalMass;
                        
                        physicsObjects[i].vx += impulse * physicsObjects[j].mass * dx;
                        physicsObjects[i].vy += impulse * physicsObjects[j].mass * dy;
                        physicsObjects[j].vx -= impulse * physicsObjects[i].mass * dx;
                        physicsObjects[j].vy -= impulse * physicsObjects[i].mass * dy;
                    }
                }
            }
        }
    }
}

void drawPhysicsSimulationDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Physics Simulation", 10, startY);
    
    // Update physics
    updatePhysics();
    
    // Draw physics objects
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (physicsObjects[i].active) {
            // Draw object
            M5.Display.fillCircle(physicsObjects[i].x, physicsObjects[i].y, 
                                 physicsObjects[i].size, physicsObjects[i].color);
            
            // Draw velocity vector (scaled down)
            int velX = physicsObjects[i].x + physicsObjects[i].vx * 5;
            int velY = physicsObjects[i].y + physicsObjects[i].vy * 5;
            M5.Display.drawLine(physicsObjects[i].x, physicsObjects[i].y, velX, velY, TFT_YELLOW);
            
            // Draw mass indicator
            M5.Display.setTextColor(TFT_BLACK);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString(String((int)physicsObjects[i].mass), 
                                 physicsObjects[i].x, physicsObjects[i].y);
        }
    }
    
    // Add force interactions
    M5.update();
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed()) {
            // Apply attractive force to objects
            for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
                if (physicsObjects[i].active) {
                    float dx = touch.x - physicsObjects[i].x;
                    float dy = touch.y - physicsObjects[i].y;
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    if (distance > 0) {
                        float force = 0.1 / (distance * distance + 1); // Inverse square with min
                        physicsObjects[i].ax = dx * force;
                        physicsObjects[i].ay = dy * force;
                    }
                }
            }
            
            // Draw attraction point
            M5.Display.fillCircle(touch.x, touch.y, 8, TFT_WHITE);
            M5.Display.drawCircle(touch.x, touch.y, 20, TFT_CYAN);
        } else {
            // Reset accelerations (only gravity)
            for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
                physicsObjects[i].ax = 0;
                physicsObjects[i].ay = 0.2; // Gravity
            }
        }
    }
    
    // Physics information
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Physics Features:", 10, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Gravity simulation", 10, startY + 35);
    M5.Display.drawString("• Collision detection", 10, startY + 50);
    M5.Display.drawString("• Elastic bouncing", 10, startY + 65);
    M5.Display.drawString("• Mass-based physics", 10, startY + 80);
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Controls:", 150, startY + 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Touch to attract objects", 150, startY + 35);
    
    // Calculate system energy
    float totalKineticEnergy = 0;
    for (int i = 0; i < MAX_PHYSICS_OBJECTS; i++) {
        if (physicsObjects[i].active) {
            float vel = sqrt(physicsObjects[i].vx * physicsObjects[i].vx + 
                           physicsObjects[i].vy * physicsObjects[i].vy);
            totalKineticEnergy += 0.5 * physicsObjects[i].mass * vel * vel;
        }
    }
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("System Energy: " + String(totalKineticEnergy, 1), 250, startY + 20);
    M5.Display.drawString("Active Objects: " + String(MAX_PHYSICS_OBJECTS), 250, startY + 35);
}

void drawSequencedAnimationsDemo() {
    int startY = 85;
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Sequenced Animations", 10, startY);
    
    // Animation sequence state machine
    static int sequenceState = 0;
    static float sequenceTime = 0;
    static float stateDuration = 3.0; // 3 seconds per state
    
    sequenceTime += deltaTime * 20;
    if (sequenceTime >= stateDuration) {
        sequenceTime = 0;
        sequenceState = (sequenceState + 1) % 6;
    }
    
    float stateProgress = sequenceTime / stateDuration;
    
    // State names
    const char* stateNames[] = {"Fade In", "Move Right", "Scale Up", "Rotate", "Move Down", "Fade Out"};
    
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("Current: " + String(stateNames[sequenceState]), 10, startY + 20);
    M5.Display.drawString("Progress: " + String((int)(stateProgress * 100)) + "%", 10, startY + 35);
    
    // Progress bar
    M5.Display.drawRect(120, startY + 22, 100, 8, TFT_WHITE);
    M5.Display.fillRect(121, startY + 23, stateProgress * 98, 6, TFT_GREEN);
    
    // Animated object based on sequence state
    float objX = 50, objY = startY + 80;
    float objScale = 1.0;
    float objRotation = 0;
    float objOpacity = 1.0;
    uint16_t objColor = TFT_CYAN;
    
    switch(sequenceState) {
        case 0: // Fade In
            objOpacity = easeOutCubic(stateProgress);
            break;
        case 1: // Move Right
            objX = 50 + easeInOutQuad(stateProgress) * 200;
            break;
        case 2: // Scale Up
            objScale = 1.0 + easeOutBounce(stateProgress) * 1.5;
            objX = 250;
            break;
        case 3: // Rotate
            objRotation = easeInOutSine(stateProgress) * 360;
            objScale = 2.5;
            objX = 250;
            break;
        case 4: // Move Down
            objY = startY + 80 + easeInCubic(stateProgress) * 100;
            objScale = 2.5;
            objX = 250;
            break;
        case 5: // Fade Out
            objOpacity = 1.0 - easeInQuad(stateProgress);
            objY = startY + 180;
            objScale = 2.5;
            objX = 250;
            break;
    }
    
    // Apply opacity to color
    uint8_t r = ((objColor >> 11) & 0x1F) * 8 * objOpacity;
    uint8_t g = ((objColor >> 5) & 0x3F) * 4 * objOpacity;
    uint8_t b = (objColor & 0x1F) * 8 * objOpacity;
    uint16_t fadedColor = M5.Display.color565(r, g, b);
    
    // Draw object (simplified rotation with multiple circles)
    int size = 10 * objScale;
    if (objRotation == 0) {
        M5.Display.fillCircle(objX, objY, size, fadedColor);
    } else {
        // Simulate rotation with multiple offset circles
        for (int i = 0; i < 8; i++) {
            float angle = (objRotation + i * 45) * PI / 180;
            int offsetX = objX + 5 * objScale * cos(angle);
            int offsetY = objY + 5 * objScale * sin(angle);
            M5.Display.fillCircle(offsetX, offsetY, size * 0.3, fadedColor);
        }
    }
    
    // Timeline visualization
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Animation Timeline:", 10, startY + 120);
    
    int timelineY = startY + 140;
    int timelineWidth = 300;
    int stateWidth = timelineWidth / 6;
    
    // Draw timeline
    M5.Display.drawLine(50, timelineY, 50 + timelineWidth, timelineY, TFT_WHITE);
    
    // Draw state markers
    for (int i = 0; i < 6; i++) {
        int x = 50 + i * stateWidth;
        uint16_t color = (i == sequenceState) ? TFT_GREEN : TFT_DARKGREY;
        M5.Display.drawLine(x, timelineY - 5, x, timelineY + 5, color);
        
        // State labels
        M5.Display.setTextColor(color);
        M5.Display.setTextDatum(TC_DATUM);
        M5.Display.drawString(String(i+1), x, timelineY + 10);
    }
    
    // Current position marker
    int currentX = 50 + sequenceState * stateWidth + stateProgress * stateWidth;
    M5.Display.fillTriangle(currentX, timelineY - 8, currentX - 4, timelineY - 15, currentX + 4, timelineY - 15, TFT_RED);
    
    // Parallel animations
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Parallel Animations:", 10, startY + 170);
    
    // Multiple objects with different timing
    for (int i = 0; i < 5; i++) {
        float delay = i * 0.2;
        float localTime = sequenceTime - delay;
        if (localTime < 0) localTime = 0;
        float localProgress = localTime / stateDuration;
        if (localProgress > 1) localProgress = 1;
        
        float x = 50 + i * 20;
        float y = startY + 190 + easeOutBounce(localProgress) * 30;
        
        uint8_t intensity = 50 + 200 * localProgress;
        uint16_t color = M5.Display.color565(intensity, intensity/2, i * 50);
        M5.Display.fillCircle(x, y, 4, color);
    }
    
    // Information
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Sequence Benefits:", 300, startY + 80);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Complex choreography", 300, startY + 95);
    M5.Display.drawString("• State management", 300, startY + 110);
    M5.Display.drawString("• Timing control", 300, startY + 125);
    M5.Display.drawString("• Parallel execution", 300, startY + 140);
    M5.Display.drawString("• Event triggering", 300, startY + 155);
    
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("State: " + String(sequenceState + 1) + "/6", 300, startY + 175);
    M5.Display.drawString("Time: " + String(sequenceTime, 1) + "s", 300, startY + 190);
    M5.Display.drawString("Total: " + String(stateDuration * 6, 0) + "s loop", 300, startY + 205);
}

void loop() {
    frameStartTime = millis();
    M5.update();
    
    // Calculate frame timing
    unsigned long currentTime = millis();
    deltaTime = (currentTime - lastUpdate) / 1000.0;
    if (deltaTime > 0.1) deltaTime = 0.1; // Cap at 100ms to prevent large jumps
    
    // Update global animation time
    animationTime += deltaTime;
    if (animationTime > TWO_PI) animationTime -= TWO_PI;
    
    // Calculate FPS
    frameCount++;
    if (frameCount % 30 == 0) {
        fps = 30000.0 / (currentTime - frameStartTime + (29 * 50)); // Approximate
    }
    
    // Handle button presses
    if (M5.BtnA.wasPressed()) {
        currentDemo = (AnimationDemo)((currentDemo - 1 + ANIMATION_DEMO_COUNT) % ANIMATION_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    if (M5.BtnB.wasPressed()) {
        // Reset animations
        animationTime = 0;
        animationStep = 0;
        frameCount = 0;
        
        // Reset particle system
        initParticleSystem();
        
        // Reset physics
        initPhysicsObjects();
        
        // Reset scroll layers
        initScrollLayers();
        
        displayCurrentDemo();
    }
    
    if (M5.BtnC.wasPressed()) {
        currentDemo = (AnimationDemo)((currentDemo + 1) % ANIMATION_DEMO_COUNT);
        displayCurrentDemo();
    }
    
    // Animation updates
    if (millis() - lastUpdate > 50) { // 20 FPS target
        animationStep++;
        if (animationStep > 10000) animationStep = 0;
        
        // Redraw current demo
        drawCurrentAnimationDemo();
        
        lastUpdate = millis();
    }
}