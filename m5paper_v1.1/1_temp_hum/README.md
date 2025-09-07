# M5Paper v1.1 Temperature & Humidity Monitor

This project displays temperature, humidity, and CO2 readings from an SCD40 sensor on an M5Paper v1.1 e-ink display.

## Hardware Requirements

- M5Paper v1.1 development board
- SCD40 CO2/Temperature/Humidity sensor
- Grove cable for connecting sensor to Port A

## Features

- Real-time temperature, humidity, and CO2 monitoring
- Large, easy-to-read display optimized for the 960x540 e-ink screen
- Automatic sensor reading every 5 seconds
- CO2 sensor calibration mode (420 ppm outdoor calibration)
- Persistent calibration settings

## Display Layout

The 960x540 landscape screen is divided into three vertical sections:
- **Left Section**: Temperature display with large digits
- **Middle Section**: Humidity display with large digits  
- **Right Section**: CO2 concentration with large digits
- Vertical dividing lines separate the three sections

## Button Controls

M5Paper v1.1 has a wheel control on the back that can be:
- **Rotate UP/Left (G37/BtnL)**: Enter calibration mode
- **Push/Press (G38/BtnP)**: Cancel calibration
- **Rotate DOWN/Right (G39/BtnR)**: Confirm calibration (when ready)

## Calibration Process

1. Rotate the wheel UP to enter calibration mode
2. Take the device outside to fresh air
3. Wait 15 seconds for sensor stabilization
4. When "READY!" appears, rotate wheel DOWN to calibrate
5. Device will count down 15 seconds and calibrate to 420 ppm

## Wiring

Connect the SCD40 sensor to Port A:
- Yellow wire (SDA) → Pin 25
- White wire (SCL) → Pin 32
- Red wire → 5V
- Black wire → GND

## Building and Uploading

1. Install PlatformIO
2. Open the project folder
3. Build: `pio run`
4. Upload: `pio run -t upload`
5. Monitor: `pio device monitor`

## Notes

- The e-ink display updates every 5 seconds to preserve display life
- Calibration settings are saved to non-volatile memory
- The sensor's Automatic Self-Calibration (ASC) is disabled for better accuracy
- Display uses DU4 update mode for faster refresh