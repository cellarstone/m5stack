/*
 * M5Unified Tutorial - 08_sd_card
 * 
 * This demo demonstrates SD card file operations with M5Stack devices:
 * - SD card initialization and detection
 * - File and directory operations (create, read, write, delete)
 * - File browser with navigation
 * - Data logging capabilities
 * - File transfer and management
 * - Storage space monitoring
 * 
 * Key concepts:
 * - SD.begin() for SD card initialization
 * - File system operations (open, close, read, write)
 * - Directory listing and navigation
 * - Error handling for file operations
 * - Data persistence and logging
 */

#include <M5Unified.h>
#include <SD.h>
#include <FS.h>

// Demo modes
enum SDDemo {
    DEMO_SD_STATUS,
    DEMO_FILE_BROWSER,
    DEMO_FILE_OPERATIONS,
    DEMO_DATA_LOGGER,
    SD_DEMO_COUNT
};

// Touch button definitions
struct TouchButton {
    int x, y, w, h;
    String label;
    uint16_t color;
    bool pressed;
};

// Define touch buttons - make them larger and more visible
TouchButton btnPrev = {10, 260, 95, 45, "< PREV", TFT_DARKGREEN, false};
TouchButton btnAction = {112, 260, 95, 45, "ACTION", TFT_BLUE, false};
TouchButton btnNext = {214, 260, 95, 45, "NEXT >", TFT_DARKGREEN, false};

SDDemo currentDemo = DEMO_SD_STATUS;
const char* sdDemoNames[] = {
    "SD Card Status",
    "File Browser",
    "File Operations",
    "Data Logger"
};

// SD card variables
bool sdCardPresent = false;
bool simulationMode = false;
uint64_t totalBytes = 0;
uint64_t usedBytes = 0;
String currentPath = "/";
std::vector<String> fileList;
std::vector<bool> isDirectory;
int selectedFile = 0;
int displayOffset = 0;
const int maxDisplayFiles = 8;

// Data logger variables
bool loggingActive = false;
String logFileName = "";
unsigned long lastLogTime = 0;
int logInterval = 1000;  // 1 second
int logCounter = 0;

// Forward declarations
void initializeSD();
void displayWelcome();
void displayCurrentDemo();
void loadFileList(String path);
void drawSDStatusDemo();
void drawFileBrowserDemo();
void drawFileOperationsDemo();
void drawDataLoggerDemo();
void handleFileBrowser();
void handleFileOperations();
void handleDataLogger();
void drawTouchButton(TouchButton& btn);
bool checkTouchButton(TouchButton& btn, int touchX, int touchY);
void drawAllButtons();

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    
    M5.Display.setRotation(3);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Debug: Check if touch is available after initialization
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    if (M5.Touch.isEnabled()) {
        M5.Display.drawString("Touch: Enabled", 5, 5);
    } else {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Touch: NOT AVAILABLE", 5, 5);
    }
    delay(1000);
    
    // Initialize SD card
    initializeSD();
    
    // Welcome screen
    displayWelcome();
    delay(2000);
    
    // Load initial file list
    if (sdCardPresent) {
        loadFileList(currentPath);
    }
    
    // Start with first demo
    displayCurrentDemo();
}

void initializeSD() {
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("Initializing SD...", M5.Display.width()/2, M5.Display.height()/2);
    
    // For M5TAB5/ESP32P4, we need to specify the correct CS pin
    // Try with different configurations
    sdCardPresent = SD.begin(4);  // Try with CS pin 4
    
    if (!sdCardPresent) {
        // Try with auto-detection via M5.getBoard()
        auto board = M5.getBoard();
        if (board == m5::board_t::board_M5StackCore2 || 
            board == m5::board_t::board_M5Stack) {
            sdCardPresent = SD.begin(4);
        } else {
            // For M5TAB5 or unknown boards, try common CS pins
            const int cs_pins[] = {4, 5, 13, 15, 33};
            for (int cs : cs_pins) {
                sdCardPresent = SD.begin(cs);
                if (sdCardPresent) break;
            }
        }
    }
    
    // If still failed, enable simulation mode for testing
    if (!sdCardPresent) {
        // We'll simulate SD card with fake data for demo purposes
        simulationMode = true;
        sdCardPresent = true; // Enable simulation mode
        totalBytes = 8ULL * 1024 * 1024 * 1024; // Simulate 8GB card
        usedBytes = 2ULL * 1024 * 1024 * 1024;  // Simulate 2GB used
        
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("SD Simulation Mode", M5.Display.width()/2, M5.Display.height()/2 + 30);
        return;
    }
    
    if (sdCardPresent) {
        // Get SD card info
        totalBytes = SD.totalBytes();
        usedBytes = SD.usedBytes();
        
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.drawString("SD Card Ready", M5.Display.width()/2, M5.Display.height()/2 + 30);
    } else {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("SD Card Error", M5.Display.width()/2, M5.Display.height()/2 + 30);
    }
    
    delay(1000);
}

void displayWelcome() {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("M5 SD Card", M5.Display.width()/2, 30);
    M5.Display.drawString("Demo", M5.Display.width()/2, 70);
    
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("File System Tutorial", M5.Display.width()/2, 120);
    
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.drawString("Features:", M5.Display.width()/2, 160);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("• SD Card Status Monitoring", M5.Display.width()/2, 180);
    M5.Display.drawString("• File Browser & Navigation", M5.Display.width()/2, 195);
    M5.Display.drawString("• File Operations (CRUD)", M5.Display.width()/2, 210);
    M5.Display.drawString("• Data Logging System", M5.Display.width()/2, 225);
    
    if (!sdCardPresent) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Insert SD card and restart", M5.Display.width()/2, 250);
    }
}

void loadFileList(String path) {
    fileList.clear();
    isDirectory.clear();
    
    if (!sdCardPresent) return;
    
    // Handle simulation mode
    if (simulationMode) {
        // Add parent directory option if not at root
        if (path != "/") {
            fileList.push_back("..");
            isDirectory.push_back(true);
        }
        
        // Simulate some files and directories
        if (path == "/") {
            fileList.push_back("Documents");
            isDirectory.push_back(true);
            fileList.push_back("Pictures");
            isDirectory.push_back(true);
            fileList.push_back("Music");
            isDirectory.push_back(true);
            fileList.push_back("readme.txt");
            isDirectory.push_back(false);
            fileList.push_back("config.ini");
            isDirectory.push_back(false);
            fileList.push_back("data.csv");
            isDirectory.push_back(false);
        } else if (path == "/Documents") {
            fileList.push_back("report.pdf");
            isDirectory.push_back(false);
            fileList.push_back("notes.txt");
            isDirectory.push_back(false);
        } else if (path == "/Pictures") {
            fileList.push_back("photo1.jpg");
            isDirectory.push_back(false);
            fileList.push_back("photo2.jpg");
            isDirectory.push_back(false);
        } else if (path == "/Music") {
            fileList.push_back("song1.mp3");
            isDirectory.push_back(false);
            fileList.push_back("song2.mp3");
            isDirectory.push_back(false);
        }
    } else {
        // Real SD card access
        File root = SD.open(path);
        if (!root || !root.isDirectory()) {
            return;
        }
        
        // Add parent directory option if not at root
        if (path != "/") {
            fileList.push_back("..");
            isDirectory.push_back(true);
        }
        
        File file = root.openNextFile();
        while (file) {
            String fileName = file.name();
            fileList.push_back(fileName);
            isDirectory.push_back(file.isDirectory());
            file.close();
            file = root.openNextFile();
        }
        root.close();
    }
    
    selectedFile = 0;
    displayOffset = 0;
}

void displayCurrentDemo() {
    M5.Display.fillScreen(TFT_BLACK);
    
    // Header
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(TC_DATUM);
    M5.Display.drawString("SD Card Demo", M5.Display.width()/2, 10);
    
    // Current demo name
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(2);
    M5.Display.drawString(sdDemoNames[currentDemo], M5.Display.width()/2, 35);
    
    // Demo counter
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Demo " + String(currentDemo + 1) + " of " + String(SD_DEMO_COUNT), M5.Display.width()/2, 60);
    
    // SD status indicator
    M5.Display.setTextDatum(TR_DATUM);
    if (simulationMode) {
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.drawString("SD: SIM", M5.Display.width() - 5, 10);
    } else {
        M5.Display.setTextColor(sdCardPresent ? TFT_GREEN : TFT_RED);
        M5.Display.drawString(sdCardPresent ? "SD: OK" : "SD: ERROR", M5.Display.width() - 5, 10);
    }
    
    if (!sdCardPresent) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setTextDatum(MC_DATUM);
        M5.Display.drawString("No SD Card", M5.Display.width()/2, M5.Display.height()/2);
        M5.Display.setTextSize(1);
        M5.Display.drawString("Insert SD card and restart", M5.Display.width()/2, M5.Display.height()/2 + 30);
        
        // Show auto-retry message
        M5.Display.setTextColor(TFT_YELLOW);
        M5.Display.setTextDatum(BC_DATUM);
        M5.Display.setTextSize(1);
        M5.Display.drawString("Will retry automatically...", M5.Display.width()/2, M5.Display.height() - 10);
        return;
    }
    
    // Draw demo-specific content
    switch(currentDemo) {
        case DEMO_SD_STATUS:
            drawSDStatusDemo();
            break;
        case DEMO_FILE_BROWSER:
            drawFileBrowserDemo();
            break;
        case DEMO_FILE_OPERATIONS:
            drawFileOperationsDemo();
            break;
        case DEMO_DATA_LOGGER:
            drawDataLoggerDemo();
            break;
    }
    
    // Show auto-cycle mode indicator instead of buttons
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setTextDatum(BC_DATUM);
    M5.Display.setTextSize(1);
    M5.Display.drawString("Auto-cycling demos every 5 seconds", M5.Display.width()/2, M5.Display.height() - 10);
}

void drawSDStatusDemo() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.drawString("SD Card Information:", 20, 80);
    
    // Total space
    float totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
    M5.Display.drawString("Total Space: " + String(totalGB, 2) + " GB", 20, 100);
    
    // Used space
    float usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
    M5.Display.drawString("Used Space: " + String(usedGB, 2) + " GB", 20, 120);
    
    // Free space
    float freeGB = (totalBytes - usedBytes) / (1024.0 * 1024.0 * 1024.0);
    M5.Display.drawString("Free Space: " + String(freeGB, 2) + " GB", 20, 140);
    
    // Usage percentage
    float usagePercent = (float)usedBytes / totalBytes * 100.0;
    M5.Display.drawString("Usage: " + String(usagePercent, 1) + "%", 20, 160);
    
    // Draw usage bar
    int barWidth = 200;
    int barHeight = 20;
    int barX = (M5.Display.width() - barWidth) / 2;
    int barY = 190;
    
    M5.Display.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
    int usedWidth = (usagePercent / 100.0) * barWidth;
    uint16_t barColor = TFT_GREEN;
    if (usagePercent > 80) barColor = TFT_RED;
    else if (usagePercent > 60) barColor = TFT_YELLOW;
    
    M5.Display.fillRect(barX, barY, usedWidth, barHeight, barColor);
    
    // File count
    M5.Display.drawString("Files in root: " + String(fileList.size()), 20, 230);
    
    // SD card type info (if available)
    M5.Display.drawString("Card Type: SD", 20, 250);
}

void drawFileBrowserDemo() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Current Path: " + currentPath, 10, 80);
    
    // Draw file list
    int startY = 100;
    int itemHeight = 15;
    
    for (int i = 0; i < maxDisplayFiles && (i + displayOffset) < fileList.size(); i++) {
        int fileIndex = i + displayOffset;
        int y = startY + i * itemHeight;
        
        // Highlight selected file
        if (fileIndex == selectedFile) {
            M5.Display.fillRect(10, y - 2, M5.Display.width() - 20, itemHeight, TFT_BLUE);
        }
        
        // Set color based on file type
        uint16_t textColor = TFT_WHITE;
        if (isDirectory[fileIndex]) {
            textColor = TFT_YELLOW;
        }
        
        M5.Display.setTextColor(textColor);
        String displayName = fileList[fileIndex];
        if (displayName.length() > 35) {
            displayName = displayName.substring(0, 32) + "...";
        }
        
        String prefix = isDirectory[fileIndex] ? "[DIR] " : "      ";
        M5.Display.drawString(prefix + displayName, 15, y);
    }
    
    // Draw scrollbar if needed
    if (fileList.size() > maxDisplayFiles) {
        int scrollbarHeight = (maxDisplayFiles * itemHeight * maxDisplayFiles) / fileList.size();
        int scrollbarY = startY + (displayOffset * itemHeight * maxDisplayFiles) / fileList.size();
        M5.Display.fillRect(M5.Display.width() - 5, scrollbarY, 3, scrollbarHeight, TFT_WHITE);
    }
    
    // Instructions
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Touch: Select  ACTION: Open/Enter", 10, 250);
}

void drawFileOperationsDemo() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.drawString("File Operations:", 20, 80);
    M5.Display.drawString("• Touch top: Create test file", 20, 100);
    M5.Display.drawString("• Touch middle: Read test file", 20, 120);
    M5.Display.drawString("• Touch bottom: Delete test file", 20, 140);
    
    // Check if test file exists
    bool testFileExists = SD.exists("/test.txt");
    M5.Display.setTextColor(testFileExists ? TFT_GREEN : TFT_RED);
    M5.Display.drawString("Test file: " + String(testFileExists ? "EXISTS" : "NOT FOUND"), 20, 170);
    
    // Show last operation result
    static String lastResult = "Ready for operations";
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Last result:", 20, 200);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString(lastResult, 20, 220);
}

void drawDataLoggerDemo() {
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextColor(TFT_WHITE);
    
    M5.Display.drawString("Data Logger Status:", 20, 80);
    
    // Logging status
    M5.Display.setTextColor(loggingActive ? TFT_GREEN : TFT_RED);
    M5.Display.drawString("Logging: " + String(loggingActive ? "ACTIVE" : "STOPPED"), 20, 100);
    
    if (loggingActive) {
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("File: " + logFileName, 20, 120);
        M5.Display.drawString("Interval: " + String(logInterval) + "ms", 20, 140);
        M5.Display.drawString("Entries: " + String(logCounter), 20, 160);
        
        // Show next log time
        unsigned long nextLog = lastLogTime + logInterval;
        unsigned long timeToNext = (nextLog > millis()) ? (nextLog - millis()) : 0;
        M5.Display.drawString("Next in: " + String(timeToNext) + "ms", 20, 180);
    }
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.drawString("Instructions:", 20, 210);
    M5.Display.drawString("Touch: Toggle logging", 20, 225);
    M5.Display.drawString("ACTION: Change interval", 20, 240);
}

void handleFileBrowser() {
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        int touchY = touch.y;
        if (touchY >= 100 && touchY <= 220) {
            int itemIndex = (touchY - 100) / 15 + displayOffset;
            if (itemIndex < fileList.size()) {
                selectedFile = itemIndex;
                displayCurrentDemo();
            }
        }
    }
}

void handleFileOperations() {
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        int touchY = touch.y;
        String result = "";
        
        if (touchY >= 95 && touchY <= 115) {
            // Create test file
            if (simulationMode) {
                result = "File created (simulated)";
            } else {
                File file = SD.open("/test.txt", FILE_WRITE);
                if (file) {
                    file.println("M5Stack SD Test File");
                    file.println("Created: " + String(millis()));
                    file.println("Demo: File Operations");
                    file.close();
                    result = "File created successfully";
                } else {
                    result = "Failed to create file";
                }
            }
        } else if (touchY >= 115 && touchY <= 135) {
            // Read test file
            if (simulationMode) {
                result = "Content: Simulated test file data";
            } else {
                File file = SD.open("/test.txt");
                if (file) {
                    result = "File content: ";
                    while (file.available()) {
                        result += (char)file.read();
                        if (result.length() > 100) {
                            result += "...";
                            break;
                        }
                    }
                    file.close();
                } else {
                    result = "Failed to read file";
                }
            }
        } else if (touchY >= 135 && touchY <= 155) {
            // Delete test file
            if (simulationMode) {
                result = "File deleted (simulated)";
            } else {
                if (SD.remove("/test.txt")) {
                    result = "File deleted successfully";
                } else {
                    result = "Failed to delete file";
                }
            }
        }
        
        if (result != "") {
            M5.Display.fillRect(15, 215, 300, 30, TFT_BLACK);
            M5.Display.setTextColor(TFT_CYAN);
            M5.Display.drawString("Last result:", 20, 200);
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.drawString(result.substring(0, 40), 20, 220);
        }
    }
}

void handleDataLogger() {
    auto touch = M5.Touch.getDetail();
    if (touch.wasPressed()) {
        loggingActive = !loggingActive;
        
        if (loggingActive) {
            // Start logging
            logFileName = "/log_" + String(millis()) + ".csv";
            if (simulationMode) {
                // Just simulate file creation
                logCounter = 0;
                lastLogTime = millis();
            } else {
                File logFile = SD.open(logFileName, FILE_WRITE);
                if (logFile) {
                    logFile.println("Timestamp,Uptime,FreeHeap,TouchX,TouchY");
                    logFile.close();
                    logCounter = 0;
                    lastLogTime = millis();
                } else {
                    loggingActive = false;
                }
            }
        }
        
        displayCurrentDemo();
    }
    
    // Log interval change is now handled in the main loop via ACTION button
    
    // Log data if active
    if (loggingActive && millis() - lastLogTime >= logInterval) {
        if (simulationMode) {
            // Just increment counter for simulation
            logCounter++;
            lastLogTime = millis();
        } else {
            File logFile = SD.open(logFileName, FILE_APPEND);
            if (logFile) {
                String logEntry = String(millis()) + ",";
                logEntry += String(millis()/1000) + ",";
                logEntry += String(ESP.getFreeHeap()) + ",";
                
                auto touchLog = M5.Touch.getDetail();
                if (touchLog.isPressed()) {
                    logEntry += String(touchLog.x) + "," + String(touchLog.y);
                } else {
                    logEntry += "0,0";
                }
                
                logFile.println(logEntry);
                logFile.close();
                logCounter++;
                lastLogTime = millis();
            }
        }
    }
}

// Touch button helper functions
void drawTouchButton(TouchButton& btn) {
    uint16_t fillColor = btn.pressed ? TFT_YELLOW : btn.color;
    uint16_t textColor = btn.pressed ? TFT_BLACK : TFT_WHITE;
    uint16_t borderColor = btn.pressed ? TFT_YELLOW : TFT_WHITE;
    
    // Draw button with thicker border for visibility
    M5.Display.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, fillColor);
    M5.Display.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 8, borderColor);
    M5.Display.drawRoundRect(btn.x+1, btn.y+1, btn.w-2, btn.h-2, 7, borderColor);
    
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.setTextColor(textColor);
    M5.Display.setTextSize(2);  // Larger text
    M5.Display.drawString(btn.label, btn.x + btn.w/2, btn.y + btn.h/2);
}

bool checkTouchButton(TouchButton& btn, int touchX, int touchY) {
    bool result = (touchX >= btn.x && touchX <= btn.x + btn.w &&
                   touchY >= btn.y && touchY <= btn.y + btn.h);
    
    // Debug output
    if (result) {
        M5.Display.fillRect(0, 20, 320, 20, TFT_BLACK);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.setTextSize(1);
        M5.Display.setTextDatum(TL_DATUM);
        M5.Display.drawString("Button pressed: " + btn.label, 5, 25);
    }
    
    return result;
}

void drawAllButtons() {
    drawTouchButton(btnPrev);
    drawTouchButton(btnAction);
    drawTouchButton(btnNext);
}

void loop() {
    M5.update();
    
    // Auto-cycle through demos every 5 seconds since touch may not work
    static uint32_t lastDemoSwitch = millis();
    static bool autoCycleMode = true;
    
    if (autoCycleMode && millis() - lastDemoSwitch > 5000) {
        currentDemo = (SDDemo)((currentDemo + 1) % SD_DEMO_COUNT);
        displayCurrentDemo();
        lastDemoSwitch = millis();
    }
    
    // Still try to handle touch if available (simplified, no debug output)
    if (M5.Touch.isEnabled()) {
        auto touch = M5.Touch.getDetail();
        
        if (touch.wasPressed()) {
            // Any touch just advances to next demo
            currentDemo = (SDDemo)((currentDemo + 1) % SD_DEMO_COUNT);
            displayCurrentDemo();
            lastDemoSwitch = millis();
            
            // Re-enable auto-cycle after touch
            autoCycleMode = true;
        }
    }
    
    // Auto-retry SD initialization if failed
    static uint32_t lastSDRetry = 0;
    if (!sdCardPresent && !simulationMode && millis() - lastSDRetry > 10000) {
        initializeSD();
        if (sdCardPresent) {
            loadFileList(currentPath);
            displayCurrentDemo();
        }
        lastSDRetry = millis();
    }
    
    // Handle current demo
    if (sdCardPresent) {
        switch(currentDemo) {
            case DEMO_SD_STATUS:
                // Status demo is static, no interaction needed
                break;
            case DEMO_FILE_BROWSER:
                handleFileBrowser();
                break;
            case DEMO_FILE_OPERATIONS:
                handleFileOperations();
                break;
            case DEMO_DATA_LOGGER:
                handleDataLogger();
                break;
        }
    }
    
    delay(10);
}