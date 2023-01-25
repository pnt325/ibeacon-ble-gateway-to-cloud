# Bluetooth iBeacon Gateway

The device scan ibeacon message of configurated devices load from Cloud.

# Structure

```mermaid
graph LR

I0(iBeacon device - 1) -.BLE adv.->G0{{BLE Gateway}}
I1(iBeacon device - 2) -.BLE adv.->G0
I2(iBeacon device - 3) -.BLE adv.->G0
I3(iBeacon device - 4) -.BLE adv.->G0

G0--MQTT-->C0[(Cloud)]
```

# How it work?

## 1 - WIFI provisioning

The smart phone access the WIFI AP and connect to HTTP server on ESP32. Then send post message with WIFI SSID and PASSWORD.

## 2 - Connect to MQTT broker

MQTT client on ESP32 connected success to broker then subcribe to special topic for get the ibeacon device to be collect data the device matched with MAC address and iBeacon UUID

## 3 - Scan iBeacon message and publish to cloud

The data storage on devices as buffer and publish to MQTT as sequency.

