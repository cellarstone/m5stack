// #include <M5EPD.h>
// #include <Wire.h>
// #include <SensirionI2cScd4x.h>

// M5EPD_Canvas canvas(&M5.EPD);
// SensirionI2cScd4x scd4x;

// void setup() {
//     M5.begin();
//     M5.EPD.SetRotation(90);
//     M5.EPD.Clear(true);
    
//     canvas.createCanvas(540, 960);
//     canvas.fillCanvas(0);
//     canvas.setTextSize(4);
//     canvas.drawString("Initializing SCD40...", 50, 400);
//     canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
    
//     Wire.begin(21, 22);
//     uint16_t error;
//     error = scd4x.begin(Wire, SCD40_I2C_ADDR_62);
//     if (error) {
//         canvas.fillCanvas(0);
//         canvas.drawString("SCD40 init failed!", 50, 400);
//         canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
//     }
    
//     error = scd4x.startPeriodicMeasurement();
//     if (error) {
//         canvas.fillCanvas(0);
//         canvas.drawString("Failed to start measurement!", 50, 400);
//         canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
//     }
    
//     delay(5000); // Wait for first measurement
// }

// void loop() {
//     uint16_t co2 = 0;
//     float temperature = 0.0f;
//     float humidity = 0.0f;
    
//     bool isDataReady = false;
//     uint16_t error = scd4x.getDataReadyStatus(isDataReady);
    
//     if (!error && isDataReady) {
//         error = scd4x.readMeasurement(co2, temperature, humidity);
        
//         if (!error) {
//             canvas.fillCanvas(0);
//             canvas.setTextSize(4);
//             canvas.setCursor(50, 100);
//             canvas.printf("Temperature: %.1f C", temperature);
//             canvas.setCursor(50, 200);
//             canvas.printf("Humidity: %.1f %%", humidity);
//             canvas.setCursor(50, 300);
//             canvas.printf("CO2: %d ppm", co2);
            
//             canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
//         } else {
//             canvas.fillCanvas(0);
//             canvas.setTextSize(3);
//             canvas.drawString("Read error!", 50, 400);
//             canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
//         }
//     } else if (!isDataReady) {
//         canvas.fillCanvas(0);
//         canvas.setTextSize(3);
//         canvas.drawString("Waiting for data...", 50, 400);
//         canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
//     }
    
//     delay(1000);
// }