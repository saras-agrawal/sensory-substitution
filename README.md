# ESP32 PCA9685 Motor Controller

Firmware for an ESP32-WROOM driving vibration motors through a PCA9685 PWM controller.

The PCA9685 should drive MOSFET or transistor stages, not motors directly. Tie ESP32, PCA9685, and motor-supply grounds together.

## Wiring

- ESP32 `3.3V` to PCA9685 `VCC`
- ESP32 `GND` to PCA9685 `GND`
- ESP32 `GPIO21` to PCA9685 `SDA`
- ESP32 `GPIO22` to PCA9685 `SCL`
- PCA9685 PWM outputs to motor driver gates or bases

## Commands

```powershell
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor
```

The firmware retries PCA9685 detection every 2 seconds and only runs motor patterns after the board responds at I2C address `0x40`.
