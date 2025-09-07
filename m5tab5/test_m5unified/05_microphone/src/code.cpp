/*
 * M5Unified Tutorial - 05_microphone
 * 
 * This demo explores audio input capabilities:
 * - Microphone initialization
 * - Audio level monitoring (VU meter)
 * - Simple waveform display
 * - Voice detection
 * - Recording to buffer
 * 
 * Note: M5Stack Tab5 may not have a microphone
 * This demo shows the API usage for devices that do
 */

#include <M5Unified.h>

const int SAMPLES = 256;
int16_t audioBuffer[SAMPLES];
float audioLevel = 0;
float peakLevel = 0;
unsigned long peakTime = 0;

// Forward declarations
void drawInterface();
void showSimulatedDemo();

void setup() {
    auto cfg = M5.config();
    cfg.internal_mic = true;  // Enable microphone if available
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Check if microphone is available
    if (!M5.Mic.isEnabled()) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Microphone Not Available", M5.Display.width()/2, M5.Display.height()/2 - 20);
        M5.Display.setTextSize(1);
        M5.Display.drawString("This demo shows mic API usage", M5.Display.width()/2, M5.Display.height()/2 + 20);
        M5.Display.drawString("for compatible M5Stack devices", M5.Display.width()/2, M5.Display.height()/2 + 40);
        
        // Show simulated data instead
        showSimulatedDemo();
        return;
    }
    
    // Initialize microphone
    M5.Mic.begin();
    
    drawInterface();
}

void drawInterface() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Title
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("Microphone Demo", M5.Display.width()/2, 10);
    
    // VU Meter background
    M5.Display.drawRect(20, 50, M5.Display.width() - 40, 30, TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Audio Level", 20, 35);
    
    // Waveform area
    M5.Display.drawRect(20, 100, M5.Display.width() - 40, 100, TFT_WHITE);
    M5.Display.drawString("Waveform", 20, 85);
    
    // Status area
    M5.Display.drawString("Status:", 20, 210);
}

void showSimulatedDemo() {
    float time = 0;
    
    while(1) {
        M5.update();
        
        // Simulate audio data
        time += 0.05;
        audioLevel = (sin(time) + 1) * 50;
        
        // Draw VU meter
        M5.Display.fillRect(21, 51, M5.Display.width() - 42, 28, TFT_BLACK);
        int barWidth = (audioLevel * (M5.Display.width() - 42)) / 100;
        uint16_t color = audioLevel > 75 ? TFT_RED : (audioLevel > 50 ? TFT_YELLOW : TFT_GREEN);
        M5.Display.fillRect(21, 51, barWidth, 28, color);
        
        // Draw simulated waveform
        M5.Display.fillRect(21, 101, M5.Display.width() - 42, 98, TFT_BLACK);
        for (int i = 0; i < SAMPLES; i += 2) {
            int y = 150 + sin(time + i * 0.1) * 30 * (audioLevel / 100);
            M5.Display.drawPixel(21 + i, y, TFT_CYAN);
            if (i > 0) {
                int prevY = 150 + sin(time + (i-2) * 0.1) * 30 * (audioLevel / 100);
                M5.Display.drawLine(21 + i - 2, prevY, 21 + i, y, TFT_CYAN);
            }
        }
        
        // Status
        M5.Display.fillRect(70, 210, 200, 20, TFT_BLACK);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("Simulated Audio Data", 70, 210);
        
        // Check for touch to exit simulation
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            break;
        }
        
        delay(50);
    }
}

void loop() {
    M5.update();
    
    if (!M5.Mic.isEnabled()) {
        return;
    }
    
    // Record audio samples
    if (M5.Mic.record(audioBuffer, SAMPLES)) {
        // Calculate audio level
        float sum = 0;
        int16_t maxVal = 0;
        int16_t minVal = 0;
        
        for (int i = 0; i < SAMPLES; i++) {
            sum += abs(audioBuffer[i]);
            if (audioBuffer[i] > maxVal) maxVal = audioBuffer[i];
            if (audioBuffer[i] < minVal) minVal = audioBuffer[i];
        }
        
        audioLevel = (sum / SAMPLES) / 327.68; // Normalize to 0-100
        
        // Track peak level
        if (audioLevel > peakLevel) {
            peakLevel = audioLevel;
            peakTime = millis();
        }
        
        // Decay peak level
        if (millis() - peakTime > 1000) {
            peakLevel *= 0.95;
        }
        
        // Draw VU meter
        M5.Display.fillRect(21, 51, M5.Display.width() - 42, 28, TFT_BLACK);
        int barWidth = (audioLevel * (M5.Display.width() - 42)) / 100;
        uint16_t color = audioLevel > 75 ? TFT_RED : (audioLevel > 50 ? TFT_YELLOW : TFT_GREEN);
        M5.Display.fillRect(21, 51, barWidth, 28, color);
        
        // Draw peak indicator
        int peakPos = (peakLevel * (M5.Display.width() - 42)) / 100;
        M5.Display.drawLine(21 + peakPos, 51, 21 + peakPos, 79, TFT_WHITE);
        
        // Draw waveform
        M5.Display.fillRect(21, 101, M5.Display.width() - 42, 98, TFT_BLACK);
        int step = SAMPLES / (M5.Display.width() - 42);
        for (int x = 0; x < M5.Display.width() - 42; x++) {
            int sampleIndex = x * step;
            int y = 150 + (audioBuffer[sampleIndex] * 40) / 32768;
            M5.Display.drawPixel(21 + x, y, TFT_CYAN);
            
            if (x > 0) {
                int prevIndex = (x - 1) * step;
                int prevY = 150 + (audioBuffer[prevIndex] * 40) / 32768;
                M5.Display.drawLine(21 + x - 1, prevY, 21 + x, y, TFT_CYAN);
            }
        }
        
        // Voice detection
        M5.Display.fillRect(70, 210, 200, 20, TFT_BLACK);
        if (audioLevel > 10) {
            M5.Display.setTextColor(TFT_GREEN);
            M5.Display.drawString("Voice Detected! Level: " + String((int)audioLevel), 70, 210);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.drawString("Silent", 70, 210);
        }
    }
    
    delay(10);
}