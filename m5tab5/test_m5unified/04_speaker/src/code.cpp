/*
 * M5Unified Tutorial - 04_speaker
 * 
 * This demo demonstrates audio output capabilities of M5Stack devices:
 * - Basic tone generation
 * - Volume control
 * - Simple melodies
 * - Frequency sweeps
 * - Interactive sound effects
 * - Different waveform types (if supported)
 * 
 * Key concepts:
 * - M5.Speaker.tone() for simple tones
 * - M5.Speaker.setVolume() for volume control
 * - M5.Speaker.playWAV() for audio file playback
 * - Non-blocking vs blocking audio playback
 * - Audio buffer management
 */

#include <M5Unified.h>

// Menu states for different audio demos
enum AudioDemo {
    DEMO_TONES,
    DEMO_MELODY,
    DEMO_VOLUME_TEST,
    DEMO_FREQUENCY_SWEEP,
    DEMO_SOUND_EFFECTS,
    DEMO_COUNT
};

AudioDemo currentDemo = DEMO_TONES;
const char* demoNames[] = {
    "Basic Tones",
    "Simple Melody",
    "Volume Test",
    "Frequency Sweep",
    "Sound Effects"
};

// Music notes frequencies (in Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

// Simple melody: Twinkle Twinkle Little Star
int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
    NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
};
int noteDurations[] = {
    4, 4, 4, 4, 4, 4, 2,
    4, 4, 4, 4, 4, 4, 2
};
int melodyLength = sizeof(melody) / sizeof(melody[0]);

// UI Variables
int currentVolume = 128;  // 0-255
int currentNote = 0;
unsigned long lastNoteTime = 0;
bool melodyPlaying = false;
int sweepFreq = 100;
bool sweepUp = true;

// Virtual button structure for Tab5
struct NavButton {
    int x, y, w, h;
    const char* label;
    uint16_t color;
};

NavButton navButtons[3];

// Forward declarations
void displayWelcome();
void displayCurrentDemo();
void handleToneDemo();
void handleMelodyDemo();
void handleVolumeTestDemo();
void handleFrequencySweepDemo();
void handleSoundEffectsDemo();
void drawNavigationButtons();
void checkNavigationButtons();

void setup() {
    auto cfg = M5.config();
    cfg.internal_spk = true;  // Enable speaker
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize speaker
    M5.Speaker.begin();
    M5.Speaker.setVolume(currentVolume);
    
    // Check if speaker is available
    if (!M5.Speaker.isEnabled()) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("No Speaker Available", M5.Display.width()/2, M5.Display.height()/2);
        while(true) {
            M5.update();
            delay(100);
        }
    }
    
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
    M5.Display.drawString("M5 Speaker", M5.Display.width()/2, 30);
    M5.Display.drawString("Demo", M5.Display.width()/2, 70);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Audio Output Tutorial", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Features:", M5.Display.width()/2, 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• Tone Generation", M5.Display.width()/2, 180);
    M5.Display.drawString("• Volume Control", M5.Display.width()/2, 195);
    M5.Display.drawString("• Melodies", M5.Display.width()/2, 210);
    M5.Display.drawString("• Sound Effects", M5.Display.width()/2, 225);
    
    // Play startup sound
    M5.Speaker.tone(440, 100);
    delay(100);
    M5.Speaker.tone(554, 100);
    delay(100);
    M5.Speaker.tone(659, 200);
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5 Audio Demo", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString(demoNames[currentDemo], M5.Display.width()/2, 40);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(DEMO_COUNT), M5.Display.width()/2, 65);
    
    // Volume indicator
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Volume: " + String((currentVolume * 100) / 255) + "%", 10, 85);
    
    // Volume bar
    int barWidth = 200;
    int barHeight = 10;
    int barX = (M5.Display.width() - barWidth) / 2;
    int barY = 105;
    
    M5.Display.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
    int fillWidth = (currentVolume * barWidth) / 255;
    M5.Display.fillRect(barX, barY, fillWidth, barHeight, TFT_GREEN);
    
    // Instructions based on current demo
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Instructions:", 10, 130);
    
    switch(currentDemo) {
        case DEMO_TONES:
            M5.Display.drawString("• Touch screen above buttons to play tones", 10, 150);
            M5.Display.drawString("• Different Y position = different frequency", 10, 165);
            M5.Display.drawString("• Different X position = different duration", 10, 180);
            M5.Display.drawString("• Use navigation buttons below to control", 10, 195);
            break;
            
        case DEMO_MELODY:
            M5.Display.drawString("• Touch screen to play melody", 10, 150);
            M5.Display.drawString("• Touch '< Prev' to stop melody", 10, 165);
            M5.Display.drawString("• Plays 'Twinkle Twinkle Little Star'", 10, 180);
            break;
            
        case DEMO_VOLUME_TEST:
            M5.Display.drawString("• Touch screen to test current volume", 10, 150);
            M5.Display.drawString("• Touch 'Volume' button to adjust", 10, 165);
            M5.Display.drawString("• Volume cycles: 25% -> 50% -> 75% -> 100%", 10, 180);
            break;
            
        case DEMO_FREQUENCY_SWEEP:
            M5.Display.drawString("• Touch to start frequency sweep", 10, 150);
            M5.Display.drawString("• Sweeps from low to high frequency", 10, 165);
            M5.Display.drawString("• Good for testing speaker range", 10, 185);
            break;
            
        case DEMO_SOUND_EFFECTS:
            M5.Display.drawString("• Different touch areas = different effects", 10, 150);
            M5.Display.drawString("• Top: Beep, Middle: Chirp, Bottom: Buzz", 10, 165);
            M5.Display.drawString("• Interactive sound playground", 10, 185);
            break;
    }
    
    // Draw virtual navigation buttons
    drawNavigationButtons();
}

void handleToneDemo() {
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.isPressed()) {
            int x = touch.x;
            int y = touch.y;
            
            // Map touch position to frequency (200Hz to 2000Hz)
            int frequency = map(y, 0, M5.Display.height(), 2000, 200);
            
            // Map touch position to tone duration (50ms to 500ms)
            int duration = map(x, 0, M5.Display.width(), 50, 500);
            
            // Play tone
            M5.Speaker.tone(frequency, duration);
            
            // Visual feedback
            M5.Display.fillCircle(x, y, 10, TFT_YELLOW);
            M5.Display.setTextColor(TFT_BLACK);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString(String(frequency) + "Hz", x, y);
            
            // Clear circle after a delay
            delay(100);
            M5.Display.fillCircle(x, y, 15, TFT_BLACK);
        }
    }
}

void handleMelodyDemo() {
    auto touch = M5.Touch.getDetail();
    if (M5.Touch.isEnabled() && touch.wasPressed() && !melodyPlaying) {
        melodyPlaying = true;
        currentNote = 0;
        lastNoteTime = millis();
        
        M5.Display.fillRect(10, 240, 300, 30, TFT_BLUE);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Playing Melody...", M5.Display.width()/2, 255);
    }
    
    if (melodyPlaying) {
        if (millis() - lastNoteTime > (1000 / noteDurations[currentNote])) {
            if (currentNote < melodyLength) {
                M5.Speaker.tone(melody[currentNote], 1000 / noteDurations[currentNote]);
                
                // Show current note
                M5.Display.fillRect(250, 240, 80, 15, TFT_GREEN);
                M5.Display.setTextColor(TFT_BLACK);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("Note " + String(currentNote + 1), 290, 247);
                
                currentNote++;
                lastNoteTime = millis();
            } else {
                melodyPlaying = false;
                currentNote = 0;
                M5.Display.fillRect(10, 240, 300, 30, TFT_BLACK);
                M5.Display.setTextColor(TFT_GREEN);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("Melody Complete!", M5.Display.width()/2, 255);
            }
        }
    }
}

void handleVolumeTestDemo() {
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        // Play test tone at current volume
        M5.Speaker.tone(1000, 500);
        
        // Visual feedback
        M5.Display.fillRect(10, 240, 300, 30, TFT_PURPLE);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Testing Volume: " + String((currentVolume * 100) / 255) + "%", 
                             M5.Display.width()/2, 255);
        
        delay(500);
        M5.Display.fillRect(10, 240, 300, 30, TFT_BLACK);
    }
}

void handleFrequencySweepDemo() {
    static bool sweeping = false;
    static unsigned long lastSweepTime = 0;
    
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed() && !sweeping) {
        sweeping = true;
        sweepFreq = 100;
        sweepUp = true;
        lastSweepTime = millis();
        
        M5.Display.fillRect(10, 240, 300, 30, TFT_ORANGE);
        M5.Display.setTextColor(TFT_BLACK);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("Frequency Sweep Started...", M5.Display.width()/2, 255);
    }
    
    if (sweeping && millis() - lastSweepTime > 50) {
        M5.Speaker.tone(sweepFreq, 50);
        
        // Update frequency
        if (sweepUp) {
            sweepFreq += 50;
            if (sweepFreq >= 3000) {
                sweepUp = false;
            }
        } else {
            sweepFreq -= 50;
            if (sweepFreq <= 100) {
                sweeping = false;
                M5.Display.fillRect(10, 240, 300, 30, TFT_BLACK);
                M5.Display.setTextColor(TFT_GREEN);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("Sweep Complete!", M5.Display.width()/2, 255);
            }
        }
        
        // Show current frequency
        if (sweeping) {
            M5.Display.fillRect(250, 240, 80, 15, TFT_RED);
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString(String(sweepFreq) + "Hz", 290, 247);
        }
        
        lastSweepTime = millis();
    }
}

void handleSoundEffectsDemo() {
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        if (touch.wasPressed()) {
            int y = touch.y;
            int screenThird = M5.Display.height() / 3;
            
            if (y < screenThird) {
                // Top area: Beep effect
                M5.Speaker.tone(1000, 100);
                delay(50);
                M5.Speaker.tone(1200, 100);
                M5.Display.fillRect(10, 240, 100, 20, TFT_CYAN);
                M5.Display.setTextColor(TFT_BLACK);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("BEEP", 60, 250);
                
            } else if (y < screenThird * 2) {
                // Middle area: Chirp effect
                for (int freq = 500; freq < 1500; freq += 100) {
                    M5.Speaker.tone(freq, 50);
                    delay(30);
                }
                M5.Display.fillRect(120, 240, 100, 20, TFT_GREEN);
                M5.Display.setTextColor(TFT_BLACK);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("CHIRP", 170, 250);
                
            } else {
                // Bottom area: Buzz effect
                for (int i = 0; i < 5; i++) {
                    M5.Speaker.tone(200, 100);
                    delay(50);
                    M5.Speaker.tone(150, 100);
                    delay(50);
                }
                M5.Display.fillRect(230, 240, 100, 20, TFT_RED);
                M5.Display.setTextColor(TFT_WHITE);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString("BUZZ", 280, 250);
            }
            
            delay(500);
            M5.Display.fillRect(10, 240, 320, 30, TFT_BLACK);
        }
    }
}

void drawNavigationButtons() {
    // Initialize button positions if not done
    if (navButtons[0].w == 0) {
        int buttonWidth = 80;
        int buttonHeight = 35;
        int buttonY = M5.Display.height() - 45;
        int spacing = (M5.Display.width() - 3 * buttonWidth) / 4;
        
        navButtons[0] = {spacing, buttonY, buttonWidth, buttonHeight, "< Prev", TFT_DARKGREY};
        navButtons[1] = {spacing * 2 + buttonWidth, buttonY, buttonWidth, buttonHeight, "Volume", TFT_DARKGREY};
        navButtons[2] = {spacing * 3 + buttonWidth * 2, buttonY, buttonWidth, buttonHeight, "Next >", TFT_DARKGREY};
    }
    
    // Draw the buttons
    for (int i = 0; i < 3; i++) {
        M5.Display.fillRoundRect(navButtons[i].x, navButtons[i].y, 
                                navButtons[i].w, navButtons[i].h, 5, navButtons[i].color);
        M5.Display.drawRoundRect(navButtons[i].x, navButtons[i].y, 
                                navButtons[i].w, navButtons[i].h, 5, TFT_WHITE);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(1);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString(navButtons[i].label, 
                             navButtons[i].x + navButtons[i].w/2, 
                             navButtons[i].y + navButtons[i].h/2);
    }
    
    // Show current volume
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.drawString("Vol: " + String((currentVolume * 100) / 255) + "%", 
                         navButtons[1].x + navButtons[1].w/2, navButtons[1].y - 5);
}

void checkNavigationButtons() {
    if (!M5.Touch.isEnabled()) return;
    
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        int x = touch.x;
        int y = touch.y;
        
        // Check if touch is on any navigation button
        for (int i = 0; i < 3; i++) {
            if (x >= navButtons[i].x && x < navButtons[i].x + navButtons[i].w &&
                y >= navButtons[i].y && y < navButtons[i].y + navButtons[i].h) {
                
                // Visual feedback
                M5.Display.fillRoundRect(navButtons[i].x, navButtons[i].y, 
                                       navButtons[i].w, navButtons[i].h, 5, TFT_GREEN);
                M5.Display.setTextColor(TFT_BLACK);
                M5.Display.setTextDatum(MC_DATUM);
                M5.Display.drawString(navButtons[i].label, 
                                     navButtons[i].x + navButtons[i].w/2, 
                                     navButtons[i].y + navButtons[i].h/2);
                
                switch(i) {
                    case 0: // Previous demo
                        currentDemo = (AudioDemo)((currentDemo - 1 + DEMO_COUNT) % DEMO_COUNT);
                        displayCurrentDemo();
                        M5.Speaker.tone(400, 100);
                        break;
                        
                    case 1: // Volume control
                        currentVolume += 64;
                        if (currentVolume > 255) currentVolume = 64;
                        M5.Speaker.setVolume(currentVolume);
                        displayCurrentDemo();
                        M5.Speaker.tone(800, 100);
                        break;
                        
                    case 2: // Next demo
                        currentDemo = (AudioDemo)((currentDemo + 1) % DEMO_COUNT);
                        displayCurrentDemo();
                        M5.Speaker.tone(600, 100);
                        break;
                }
                
                delay(100); // Brief delay for visual feedback
                drawNavigationButtons(); // Redraw buttons in normal state
                break;
            }
        }
    }
}

void loop() {
    M5.update();
    
    // Check virtual navigation buttons for Tab5
    checkNavigationButtons();
    
    // Handle current demo
    switch(currentDemo) {
        case DEMO_TONES:
            handleToneDemo();
            break;
        case DEMO_MELODY:
            handleMelodyDemo();
            break;
        case DEMO_VOLUME_TEST:
            handleVolumeTestDemo();
            break;
        case DEMO_FREQUENCY_SWEEP:
            handleFrequencySweepDemo();
            break;
        case DEMO_SOUND_EFFECTS:
            handleSoundEffectsDemo();
            break;
    }
    
    // Note: On Tab5, touch "< Prev" button to stop melody during playback
    
    delay(10);
}