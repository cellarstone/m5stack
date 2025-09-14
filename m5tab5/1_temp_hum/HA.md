# Home Assistant Integration Guide

This guide explains how to integrate the M5Tab5 Environmental Monitor with Home Assistant.

## Prerequisites

1. **Home Assistant** installed and running on your network
2. **MQTT Broker** installed (e.g., Mosquitto)
3. **MQTT Integration** enabled in Home Assistant
4. **WiFi Network** accessible to both M5Tab5 and Home Assistant

## Configuration Steps

### 1. Configure the M5Tab5 Device

Edit the following lines in `src/code.cpp` with your network details:

```cpp
// WiFi Configuration - UPDATE THESE VALUES
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Configuration - UPDATE THESE VALUES
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";  // e.g., "192.168.1.100"
const int mqtt_port = 1883;
const char* mqtt_user = "YOUR_MQTT_USER";  // Leave empty "" if no auth
const char* mqtt_password = "YOUR_MQTT_PASSWORD";  // Leave empty "" if no auth
```

### 2. Device Identification

The device uses these identifiers:
- **Device Name**: `M5Tab5_Environment`
- **Device ID**: `m5tab5_env_01`

You can customize these in the code if you have multiple devices:

```cpp
const char* device_name = "M5Tab5_Environment";
const char* device_id = "m5tab5_env_01";  // Must be unique for each device
```

### 3. MQTT Broker Setup in Home Assistant

If you haven't set up an MQTT broker yet:

1. Install **Mosquitto Broker** add-on:
   - Go to Settings → Add-ons → Add-on Store
   - Search for "Mosquitto broker"
   - Install and start the add-on

2. Configure MQTT Integration:
   - Go to Settings → Devices & Services
   - Click "Add Integration"
   - Search for "MQTT"
   - Enter your broker details (usually `localhost` if using Mosquitto add-on)

### 4. Automatic Discovery

The M5Tab5 device supports **MQTT Discovery**. Once the device connects to your MQTT broker, it will automatically:

1. Publish discovery messages to Home Assistant
2. Create three sensor entities:
   - `sensor.m5tab5_environment_temperature`
   - `sensor.m5tab5_environment_humidity`
   - `sensor.m5tab5_environment_co2`

The sensors will appear automatically in Home Assistant within a few seconds of the device connecting.

### 5. Manual Configuration (Optional)

If automatic discovery doesn't work, you can manually add the sensors to your `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "M5Tab5 Temperature"
      state_topic: "homeassistant/sensor/m5tab5_env_01/state"
      availability_topic: "homeassistant/sensor/m5tab5_env_01/availability"
      unit_of_measurement: "°C"
      device_class: temperature
      value_template: "{{ value_json.temperature }}"
      
    - name: "M5Tab5 Humidity"
      state_topic: "homeassistant/sensor/m5tab5_env_01/state"
      availability_topic: "homeassistant/sensor/m5tab5_env_01/availability"
      unit_of_measurement: "%"
      device_class: humidity
      value_template: "{{ value_json.humidity }}"
      
    - name: "M5Tab5 CO2"
      state_topic: "homeassistant/sensor/m5tab5_env_01/state"
      availability_topic: "homeassistant/sensor/m5tab5_env_01/availability"
      unit_of_measurement: "ppm"
      device_class: carbon_dioxide
      value_template: "{{ value_json.co2 }}"
```

## MQTT Topics

The device publishes to these MQTT topics:

- **State Topic**: `homeassistant/sensor/m5tab5_env_01/state`
  - JSON payload: `{"temperature": 22.5, "humidity": 45.2, "co2": 650}`
  
- **Availability Topic**: `homeassistant/sensor/m5tab5_env_01/availability`
  - Payload: `online` or `offline`

- **Discovery Topics** (auto-configuration):
  - `homeassistant/sensor/m5tab5_env_01_temperature/config`
  - `homeassistant/sensor/m5tab5_env_01_humidity/config`
  - `homeassistant/sensor/m5tab5_env_01_co2/config`

## Verifying the Connection

### On the M5Tab5 Display

The device shows connection status in the top-right corner:
- **WiFi**: Shows IP address when connected or "Disconnected"
- **MQTT**: Shows "Connected" or "Disconnected"
- A green dot flashes when data is published

### In Home Assistant

1. Go to **Settings → Devices & Services → MQTT**
2. Look for a device called "M5Tab5_Environment"
3. Click on it to see the three sensor entities

### Using MQTT Explorer

You can use MQTT Explorer (or similar tools) to verify data flow:
1. Connect to your MQTT broker
2. Look for topics under `homeassistant/sensor/m5tab5_env_01/`
3. You should see data updates every 5 seconds

## Home Assistant Dashboard

Add the sensors to your dashboard:

```yaml
type: entities
title: Environmental Monitor
entities:
  - entity: sensor.m5tab5_environment_temperature
    name: Temperature
  - entity: sensor.m5tab5_environment_humidity
    name: Humidity
  - entity: sensor.m5tab5_environment_co2
    name: CO2 Level
```

Or create a more advanced card:

```yaml
type: vertical-stack
cards:
  - type: sensor
    entity: sensor.m5tab5_environment_temperature
    graph: line
    name: Temperature
  - type: sensor
    entity: sensor.m5tab5_environment_humidity
    graph: line
    name: Humidity
  - type: gauge
    entity: sensor.m5tab5_environment_co2
    name: CO2 Level
    min: 400
    max: 2000
    severity:
      green: 400
      yellow: 800
      red: 1200
```

## Automations

Example automation for poor air quality:

```yaml
automation:
  - alias: "Alert on Poor Air Quality"
    trigger:
      - platform: numeric_state
        entity_id: sensor.m5tab5_environment_co2
        above: 1200
    action:
      - service: notify.mobile_app_your_phone
        data:
          title: "Poor Air Quality"
          message: "CO2 level is {{ states('sensor.m5tab5_environment_co2') }} ppm. Please ventilate!"
```

## Troubleshooting

### Device Not Appearing in Home Assistant

1. Check WiFi connection on M5Tab5 display
2. Verify MQTT broker is running
3. Check MQTT credentials are correct
4. Use MQTT Explorer to verify data is being published
5. Check Home Assistant logs for MQTT errors

### Connection Issues

- **WiFi keeps disconnecting**: Check signal strength and router settings
- **MQTT disconnects frequently**: Verify broker allows persistent connections
- **No data updates**: Check the device display for sensor readings

### Common Error Messages

- `MQTT connection failed, rc=-2`: Wrong broker address
- `MQTT connection failed, rc=-4`: Connection timeout (check network)
- `MQTT connection failed, rc=5`: Authentication failed (check credentials)

## Security Considerations

1. Always use MQTT authentication (username/password)
2. Consider using MQTT over TLS (requires additional configuration)
3. Use a dedicated MQTT user with limited permissions
4. Keep your MQTT broker behind your firewall

## Multiple Devices

To add multiple M5Tab5 devices:

1. Change the `device_id` in each device's code to be unique
2. Optionally change the `device_name` for easy identification
3. Each device will appear separately in Home Assistant

Example for a second device:
```cpp
const char* device_name = "M5Tab5_Bedroom";
const char* device_id = "m5tab5_env_02";
```