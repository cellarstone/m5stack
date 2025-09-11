#include <M5Unified.h>
#include <Wire.h>
#include <SensirionI2CScd4x.h>

// SCD40 sensor instance
SensirionI2CScd4x scd4x;

// Working I2C pins (found by scanner)
const int GROVE_SDA = 53;  // Yellow wire
const int GROVE_SCL = 54;  // White wire

// Sensor data
float temperature = 0.0;
float humidity = 0.0;
uint16_t co2 = 400;
float tempMin = 100, tempMax = -100;
float humMin = 100, humMax = 0;
uint16_t co2Min = 9999, co2Max = 0;

// History for graphs (last 60 readings = 5 minutes at 5 sec intervals)
const int HISTORY_SIZE = 60;
float tempHistory[HISTORY_SIZE] = {0};
float humHistory[HISTORY_SIZE] = {0};
uint16_t co2History[HISTORY_SIZE] = {0};
int historyIndex = 0;
bool historyFull = false;

// Display dimensions
const int SCREEN_WIDTH = 1280;  // M5Tab5 actual resolution
const int SCREEN_HEIGHT = 720;

// Color scheme
#define BG_COLOR TFT_BLACK
#define GRID_COLOR 0x2104  // Dark gray
#define TEXT_PRIMARY TFT_WHITE
#define TEXT_SECONDARY 0xB5B6  // Light gray

// Temperature colors
#define TEMP_COLD 0x04FF  // Cyan
#define TEMP_NORMAL 0x07E0  // Green  
#define TEMP_WARM 0xFBE0  // Orange
#define TEMP_HOT 0xF800  // Red

// Humidity colors
#define HUM_LOW 0xFBE0  // Orange
#define HUM_NORMAL 0x07FF  // Cyan
#define HUM_HIGH 0x001F  // Blue

// CO2 colors
#define CO2_GOOD 0x07E0  // Green
#define CO2_MODERATE 0xFFE0  // Yellow
#define CO2_POOR 0xFBE0  // Orange
#define CO2_BAD 0xF800  // Red

void setup() {
    // Initialize M5Stack Tab5
    auto cfg = M5.config();
    cfg.external_spk = false;
    M5.begin(cfg);
    M5.Display.setRotation(1);  // Landscape
    M5.Display.fillScreen(BG_COLOR);
    
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== M5Tab5 SCD40 Environmental Monitor ===");
    
    // Show startup screen
    M5.Display.setTextSize(4);
    M5.Display.setTextColor(TEXT_PRIMARY);
    M5.Display.drawString("Environmental Monitor", SCREEN_WIDTH/2, 100);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.drawString("Initializing SCD40 sensor...", SCREEN_WIDTH/2, 200);
    
    // Initialize I2C
    Wire.begin(GROVE_SDA, GROVE_SCL);
    Serial.printf("I2C initialized (SDA=%d, SCL=%d)\n", GROVE_SDA, GROVE_SCL);
    
    // Initialize SCD40
    scd4x.begin(Wire);
    
    uint16_t error;
    char errorMessage[256];
    
    // Stop any measurement
    error = scd4x.stopPeriodicMeasurement();
    delay(500);
    
    // Get serial number
    uint16_t serial0, serial1, serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (!error) {
        Serial.printf("SCD40 Serial: 0x%04x%04x%04x\n", serial0, serial1, serial2);
        M5.Display.drawString("SCD40 Connected", SCREEN_WIDTH/2, 250);
        M5.Display.setTextSize(1);
        M5.Display.drawString(String("Serial: ") + String(serial0, HEX) + String(serial1, HEX), SCREEN_WIDTH/2, 280);
    }
    
    // Start measurements
    error = scd4x.startPeriodicMeasurement();
    if (!error) {
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.drawString("Sensor Ready!", SCREEN_WIDTH/2, 320);
        Serial.println("SCD40 started successfully!");
    }
    
    delay(2000);
    M5.Display.fillScreen(BG_COLOR);
}

uint16_t getTemperatureColor(float temp) {
    if (temp < 18) return TEMP_COLD;
    if (temp < 24) return TEMP_NORMAL;
    if (temp < 28) return TEMP_WARM;
    return TEMP_HOT;
}

uint16_t getHumidityColor(float hum) {
    if (hum < 30) return HUM_LOW;
    if (hum < 60) return HUM_NORMAL;
    return HUM_HIGH;
}

uint16_t getCO2Color(uint16_t co2) {
    if (co2 < 800) return CO2_GOOD;
    if (co2 < 1200) return CO2_MODERATE;
    if (co2 < 2000) return CO2_POOR;
    return CO2_BAD;
}

String getCO2Status(uint16_t co2) {
    if (co2 < 800) return "Excellent";
    if (co2 < 1200) return "Good";
    if (co2 < 2000) return "Moderate";
    return "Poor - Ventilate!";
}

void drawGauge(int cx, int cy, int radius, float value, float minVal, float maxVal, uint16_t color, const char* label, const char* unit) {
    // Draw outer circle
    M5.Display.drawCircle(cx, cy, radius, TEXT_SECONDARY);
    M5.Display.drawCircle(cx, cy, radius-1, TEXT_SECONDARY);
    
    // Clear arc area first
    for (int i = -135; i <= 135; i += 270/20) {
        float rad = i * PI / 180;
        int x1 = cx + (radius - 10) * cos(rad);
        int y1 = cy + (radius - 10) * sin(rad);
        int x2 = cx + (radius - 5) * cos(rad);
        int y2 = cy + (radius - 5) * sin(rad);
        M5.Display.drawLine(x1, y1, x2, y2, BG_COLOR);
        M5.Display.drawLine(x1, y1+1, x2, y2+1, BG_COLOR);
    }
    
    // Draw arc based on value
    float angle = map(value * 100, minVal * 100, maxVal * 100, -135, 135);
    int segments = 20;
    for (int i = -135; i <= angle; i += 270/segments) {
        float rad = i * PI / 180;
        int x1 = cx + (radius - 10) * cos(rad);
        int y1 = cy + (radius - 10) * sin(rad);
        int x2 = cx + (radius - 5) * cos(rad);
        int y2 = cy + (radius - 5) * sin(rad);
        M5.Display.drawLine(x1, y1, x2, y2, color);
        M5.Display.drawLine(x1, y1+1, x2, y2+1, color);
    }
    
    // Clear value area and redraw
    M5.Display.fillRect(cx - 50, cy - 25, 100, 45, BG_COLOR);
    M5.Display.setTextColor(color);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(cx - 40, cy - 20);
    M5.Display.printf("%.1f", value);
    
    // Draw unit
    M5.Display.setTextSize(2);
    M5.Display.setCursor(cx - 15, cy + 20);
    M5.Display.print(unit);
    
    // Draw label
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(cx - 40, cy + 50);
    M5.Display.print(label);
}

// New function to update only values without redrawing static elements
void updateValues(float oldTemp, float oldHum, uint16_t oldCO2) {
    int gaugeY = 200;
    int gaugeRadius = 130;
    
    // Only update if temperature changed significantly
    if (abs(temperature - oldTemp) > 0.1) {
        // Update temperature value with background fill to prevent artifacts
        M5.Display.setTextColor(getTemperatureColor(temperature), BG_COLOR);
        M5.Display.setTextSize(4);
        M5.Display.setCursor(250 - 40, gaugeY - 20);
        M5.Display.printf("%.1f", temperature);
        
        // Smooth arc update - only redraw the changed portion
        float oldAngle = map(oldTemp * 100, 0, 4000, -135, 135);
        float newAngle = map(temperature * 100, 0, 4000, -135, 135);
        
        if (newAngle < oldAngle) {
            // Temperature decreased - clear the extra arc
            for (int i = newAngle; i <= oldAngle; i += 270/20) {
                float rad = i * PI / 180;
                int x1 = 250 + (gaugeRadius - 10) * cos(rad);
                int y1 = gaugeY + (gaugeRadius - 10) * sin(rad);
                int x2 = 250 + (gaugeRadius - 5) * cos(rad);
                int y2 = gaugeY + (gaugeRadius - 5) * sin(rad);
                M5.Display.drawLine(x1, y1, x2, y2, BG_COLOR);
                M5.Display.drawLine(x1, y1+1, x2, y2+1, BG_COLOR);
            }
        } else if (newAngle > oldAngle) {
            // Temperature increased - draw the additional arc
            for (int i = oldAngle; i <= newAngle; i += 270/20) {
                float rad = i * PI / 180;
                int x1 = 250 + (gaugeRadius - 10) * cos(rad);
                int y1 = gaugeY + (gaugeRadius - 10) * sin(rad);
                int x2 = 250 + (gaugeRadius - 5) * cos(rad);
                int y2 = gaugeY + (gaugeRadius - 5) * sin(rad);
                M5.Display.drawLine(x1, y1, x2, y2, getTemperatureColor(temperature));
                M5.Display.drawLine(x1, y1+1, x2, y2+1, getTemperatureColor(temperature));
            }
        }
    }
    
    // Only update if humidity changed significantly
    if (abs(humidity - oldHum) > 0.1) {
        // Update humidity value with background color to avoid artifacts
        M5.Display.setTextColor(getHumidityColor(humidity), BG_COLOR);
        M5.Display.setTextSize(4);
        M5.Display.setCursor(640 - 40, gaugeY - 20);
        M5.Display.printf("%.1f", humidity);
        
        // Smooth arc update
        float oldAngle = map(oldHum * 100, 0, 10000, -135, 135);
        float newAngle = map(humidity * 100, 0, 10000, -135, 135);
        
        if (newAngle < oldAngle) {
            // Humidity decreased
            for (int i = newAngle; i <= oldAngle; i += 270/20) {
                float rad = i * PI / 180;
                int x1 = 640 + (gaugeRadius - 10) * cos(rad);
                int y1 = gaugeY + (gaugeRadius - 10) * sin(rad);
                int x2 = 640 + (gaugeRadius - 5) * cos(rad);
                int y2 = gaugeY + (gaugeRadius - 5) * sin(rad);
                M5.Display.drawLine(x1, y1, x2, y2, BG_COLOR);
                M5.Display.drawLine(x1, y1+1, x2, y2+1, BG_COLOR);
            }
        } else if (newAngle > oldAngle) {
            // Humidity increased
            for (int i = oldAngle; i <= newAngle; i += 270/20) {
                float rad = i * PI / 180;
                int x1 = 640 + (gaugeRadius - 10) * cos(rad);
                int y1 = gaugeY + (gaugeRadius - 10) * sin(rad);
                int x2 = 640 + (gaugeRadius - 5) * cos(rad);
                int y2 = gaugeY + (gaugeRadius - 5) * sin(rad);
                M5.Display.drawLine(x1, y1, x2, y2, getHumidityColor(humidity));
                M5.Display.drawLine(x1, y1+1, x2, y2+1, getHumidityColor(humidity));
            }
        }
    }
    
    // Only update if CO2 changed
    if (co2 != oldCO2) {
        int co2X = 1030;
        // Update CO2 value with background color
        M5.Display.setTextColor(getCO2Color(co2), BG_COLOR);
        M5.Display.setTextSize(8);
        M5.Display.setCursor(co2X - 90, gaugeY - 50);
        M5.Display.printf("%4d", co2);  // Fixed width to avoid flickering
        
        // Update status text only if category changed
        String oldStatus = getCO2Status(oldCO2);
        String newStatus = getCO2Status(co2);
        if (oldStatus != newStatus) {
            M5.Display.fillRect(co2X - 95, gaugeY + 80, 190, 35, BG_COLOR);
            M5.Display.setTextSize(3);
            M5.Display.setCursor(co2X - 80, gaugeY + 85);
            M5.Display.print(newStatus);
        }
    }
}

void drawGraph(int x, int y, int w, int h, float* data, int dataSize, uint16_t color, float minVal, float maxVal) {
    // Draw border
    M5.Display.drawRect(x, y, w, h, GRID_COLOR);
    
    // Draw grid lines
    for (int i = 1; i < 4; i++) {
        int yPos = y + (h * i / 4);
        M5.Display.drawLine(x, yPos, x + w, yPos, GRID_COLOR);
    }
    
    // Draw data
    int dataPoints = historyFull ? HISTORY_SIZE : historyIndex;
    if (dataPoints > 1) {
        for (int i = 1; i < dataPoints; i++) {
            int x1 = x + (i-1) * w / HISTORY_SIZE;
            int x2 = x + i * w / HISTORY_SIZE;
            
            float val1 = data[i-1];
            float val2 = data[i];
            
            int y1 = y + h - (int)((val1 - minVal) * h / (maxVal - minVal));
            int y2 = y + h - (int)((val2 - minVal) * h / (maxVal - minVal));
            
            if (y1 < y) y1 = y;
            if (y1 > y + h) y1 = y + h;
            if (y2 < y) y2 = y;
            if (y2 > y + h) y2 = y + h;
            
            M5.Display.drawLine(x1, y1, x2, y2, color);
            M5.Display.drawLine(x1, y1+1, x2, y2+1, color);
        }
    }
}

void drawDisplay() {
    M5.Display.fillScreen(BG_COLOR);
    
    // Title bar - full width
    M5.Display.fillRect(0, 0, SCREEN_WIDTH, 45, 0x10A2);
    M5.Display.setTextColor(TEXT_PRIMARY);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(30, 10);
    M5.Display.print("SCD40 Environmental Monitor");
    
    // Time
    M5.Display.setCursor(SCREEN_WIDTH - 200, 10);
    M5.Display.printf("Up: %d min", millis() / 60000);
    
    // Main display area - HUGE gauges using full screen
    int gaugeY = 200;
    int gaugeRadius = 130;  // Much bigger gauges
    
    // Temperature gauge - left third
    drawGauge(250, gaugeY, gaugeRadius, temperature, 0, 40, getTemperatureColor(temperature), "Temperature", "C");
    
    // Humidity gauge - center third
    drawGauge(640, gaugeY, gaugeRadius, humidity, 0, 100, getHumidityColor(humidity), "Humidity", "%");
    
    // CO2 display - right third (massive)
    int co2X = 1030;
    M5.Display.fillRoundRect(co2X - 130, gaugeY - 120, 260, 240, 20, getCO2Color(co2));
    M5.Display.fillRoundRect(co2X - 125, gaugeY - 115, 250, 230, 20, BG_COLOR);
    
    M5.Display.setTextColor(getCO2Color(co2));
    M5.Display.setTextSize(8);  // Huge CO2 number
    M5.Display.setCursor(co2X - 90, gaugeY - 50);
    M5.Display.printf("%d", co2);
    
    M5.Display.setTextSize(4);
    M5.Display.setCursor(co2X - 35, gaugeY + 40);
    M5.Display.print("ppm");
    
    M5.Display.setTextSize(3);
    M5.Display.setCursor(co2X - 80, gaugeY + 85);
    M5.Display.print(getCO2Status(co2));
    
    // Min/Max values - positioned below gauges
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setTextSize(2);
    
    M5.Display.setCursor(160, 360);
    M5.Display.printf("Min: %.1fC | Max: %.1fC", tempMin, tempMax);
    
    M5.Display.setCursor(540, 360);
    M5.Display.printf("Min: %.0f%% | Max: %.0f%%", humMin, humMax);
    
    M5.Display.setCursor(920, 360);
    M5.Display.printf("Min: %d | Max: %d", co2Min, co2Max);
    
    // Graphs area - full width, taller
    int graphY = 420;
    int graphHeight = 160;  // Much taller graphs
    int graphWidth = 400;   // Much wider graphs
    int graphSpacing = 30;
    
    // Temperature graph
    M5.Display.setTextColor(getTemperatureColor(temperature));
    M5.Display.setTextSize(2);
    M5.Display.setCursor(graphSpacing, graphY - 20);
    M5.Display.print("Temperature History (5 min)");
    drawGraph(graphSpacing, graphY, graphWidth, graphHeight, tempHistory, HISTORY_SIZE, getTemperatureColor(temperature), 15, 35);
    
    // Humidity graph
    M5.Display.setTextColor(getHumidityColor(humidity));
    M5.Display.setCursor(graphSpacing + graphWidth + 20, graphY - 20);
    M5.Display.print("Humidity History (5 min)");
    drawGraph(graphSpacing + graphWidth + 20, graphY, graphWidth, graphHeight, humHistory, HISTORY_SIZE, getHumidityColor(humidity), 0, 100);
    
    // CO2 graph
    M5.Display.setTextColor(getCO2Color(co2));
    M5.Display.setCursor(graphSpacing + (graphWidth + 20) * 2, graphY - 20);
    M5.Display.print("CO2 History (5 min)");
    float co2Float[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; i++) co2Float[i] = co2History[i];
    drawGraph(graphSpacing + (graphWidth + 20) * 2, graphY, graphWidth, graphHeight, co2Float, HISTORY_SIZE, getCO2Color(co2), 400, 2000);
    
    // Status bar at bottom - full width
    int statusY = 600;
    M5.Display.fillRect(0, statusY, SCREEN_WIDTH, SCREEN_HEIGHT - statusY, 0x0841);
    
    // Air quality indicator - much bigger
    M5.Display.fillRoundRect(30, statusY + 15, 320, 80, 10, getCO2Color(co2));
    M5.Display.setTextColor(BG_COLOR);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(60, statusY + 35);
    M5.Display.print("Air: " + getCO2Status(co2));
    
    // Comfort level
    bool comfortable = (temperature >= 20 && temperature <= 26) && 
                      (humidity >= 30 && humidity <= 60) && 
                      (co2 < 1200);
    uint16_t comfortColor = comfortable ? CO2_GOOD : CO2_MODERATE;
    
    M5.Display.fillRoundRect(380, statusY + 15, 320, 80, 10, comfortColor);
    M5.Display.setCursor(420, statusY + 35);
    M5.Display.print(comfortable ? "Comfortable" : "Needs Adjust");
    
    // Recommendations - much larger text
    M5.Display.setTextColor(TEXT_PRIMARY);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(730, statusY + 25);
    
    if (co2 > 1200) {
        M5.Display.print("Open windows for fresh air!");
    } else if (humidity < 30) {
        M5.Display.print("Air is dry - add moisture");
    } else if (humidity > 60) {
        M5.Display.print("High humidity - ventilate");
    } else if (temperature < 18) {
        M5.Display.print("Too cold - increase heating");
    } else if (temperature > 28) {
        M5.Display.print("Too warm - cooling needed");
    } else {
        M5.Display.setTextColor(CO2_GOOD);
        M5.Display.print("Conditions are optimal!");
    }
    
    // Update indicator
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TEXT_SECONDARY);
    M5.Display.setCursor(730, statusY + 65);
    M5.Display.printf("Next update in %d seconds", 5 - ((millis() % 5000) / 1000));
}

void updateHistory() {
    tempHistory[historyIndex] = temperature;
    humHistory[historyIndex] = humidity;
    co2History[historyIndex] = co2;
    
    historyIndex++;
    if (historyIndex >= HISTORY_SIZE) {
        historyIndex = 0;
        historyFull = true;
    }
    
    // Update min/max
    if (temperature < tempMin) tempMin = temperature;
    if (temperature > tempMax) tempMax = temperature;
    if (humidity < humMin) humMin = humidity;
    if (humidity > humMax) humMax = humidity;
    if (co2 < co2Min) co2Min = co2;
    if (co2 > co2Max) co2Max = co2;
}

void loop() {
    M5.update();
    
    static unsigned long lastRead = 0;
    static unsigned long lastCounterUpdate = 0;
    static bool firstDraw = true;
    static float oldTemp = 0;
    static float oldHum = 0;
    static uint16_t oldCO2 = 0;
    
    // Read sensor every 5 seconds
    if (millis() - lastRead > 5000) {
        lastRead = millis();
        
        uint16_t error;
        char errorMessage[256];
        bool isDataReady = false;
        
        error = scd4x.getDataReadyFlag(isDataReady);
        if (!error && isDataReady) {
            float newTemp, newHum;
            uint16_t newCO2;
            
            error = scd4x.readMeasurement(newCO2, newTemp, newHum);
            if (!error && newCO2 > 0) {
                // Store old values
                oldTemp = temperature;
                oldHum = humidity;
                oldCO2 = co2;
                
                // Update to new values
                temperature = newTemp;
                humidity = newHum;
                co2 = newCO2;
                
                updateHistory();
                
                Serial.printf("T: %.1f°C, H: %.1f%%, CO2: %d ppm\n", 
                             temperature, humidity, co2);
                
                if (firstDraw) {
                    // First time - draw everything
                    drawDisplay();
                    firstDraw = false;
                } else {
                    // After first draw - only update changed values
                    updateValues(oldTemp, oldHum, oldCO2);
                    
                    // Update graphs only if significant change
                    if (abs(temperature - oldTemp) > 0.5 || abs(humidity - oldHum) > 1 || abs((int)co2 - (int)oldCO2) > 20) {
                        // Redraw graphs
                        int graphY = 420;
                        int graphHeight = 160;
                        int graphWidth = 400;
                        int graphSpacing = 30;
                        
                        // Clear graph area
                        M5.Display.fillRect(graphSpacing, graphY, graphWidth, graphHeight, BG_COLOR);
                        drawGraph(graphSpacing, graphY, graphWidth, graphHeight, tempHistory, HISTORY_SIZE, getTemperatureColor(temperature), 15, 35);
                        
                        M5.Display.fillRect(graphSpacing + graphWidth + 20, graphY, graphWidth, graphHeight, BG_COLOR);
                        drawGraph(graphSpacing + graphWidth + 20, graphY, graphWidth, graphHeight, humHistory, HISTORY_SIZE, getHumidityColor(humidity), 0, 100);
                        
                        M5.Display.fillRect(graphSpacing + (graphWidth + 20) * 2, graphY, graphWidth, graphHeight, BG_COLOR);
                        float co2Float[HISTORY_SIZE];
                        for (int i = 0; i < HISTORY_SIZE; i++) co2Float[i] = co2History[i];
                        drawGraph(graphSpacing + (graphWidth + 20) * 2, graphY, graphWidth, graphHeight, co2Float, HISTORY_SIZE, getCO2Color(co2), 400, 2000);
                    }
                    
                    // Update status bar if CO2 status changed
                    String oldStatus = getCO2Status(oldCO2);
                    String newStatus = getCO2Status(co2);
                    if (oldStatus != newStatus) {
                        int statusY = 600;
                        // Update air quality indicator
                        M5.Display.fillRoundRect(30, statusY + 15, 320, 80, 10, getCO2Color(co2));
                        M5.Display.setTextColor(BG_COLOR);
                        M5.Display.setTextSize(4);
                        M5.Display.setCursor(60, statusY + 35);
                        M5.Display.print("Air: " + getCO2Status(co2));
                        
                        // Update recommendation
                        M5.Display.fillRect(730, statusY + 25, 500, 35, 0x0841);
                        M5.Display.setTextColor(TEXT_PRIMARY);
                        M5.Display.setTextSize(3);
                        M5.Display.setCursor(730, statusY + 25);
                        if (co2 > 1200) {
                            M5.Display.print("Open windows for fresh air!");
                        } else if (humidity < 30) {
                            M5.Display.print("Air is dry - add moisture");
                        } else if (humidity > 60) {
                            M5.Display.print("High humidity - ventilate");
                        } else if (temperature < 18) {
                            M5.Display.print("Too cold - increase heating");
                        } else if (temperature > 28) {
                            M5.Display.print("Too warm - cooling needed");
                        } else {
                            M5.Display.setTextColor(CO2_GOOD);
                            M5.Display.print("Conditions are optimal!");
                        }
                    }
                }
            }
        }
    }
    
    // Initial draw after 5 seconds if no data yet
    if (firstDraw && millis() > 5000) {
        drawDisplay();
        firstDraw = false;
    }
    
    // Update only the countdown timer every second
    if (!firstDraw && millis() - lastCounterUpdate > 1000) {
        lastCounterUpdate = millis();
        
        int statusY = 600;
        M5.Display.fillRect(730, statusY + 65, 400, 30, 0x0841);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TEXT_SECONDARY);
        M5.Display.setCursor(730, statusY + 65);
        M5.Display.printf("Next update in %d seconds", 5 - ((millis() % 5000) / 1000));
    }
    
    delay(50);
}