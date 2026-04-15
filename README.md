# ESP32 PCA9685 Motor Controller

Firmware for an ESP32-WROOM driving vibration motors through a PCA9685 PWM controller and ULN2803A low-side driver.

The PCA9685 should drive the ULN2803A inputs, not motors directly. Tie ESP32, PCA9685, ULN2803A, and motor-supply grounds together.

## Wiring

- ESP32 `3.3V` to PCA9685 `VCC`
- ESP32 `GND` to PCA9685 `GND`
- ESP32 `GPIO21` to PCA9685 `SDA`
- ESP32 `GPIO22` to PCA9685 `SCL`
- PCA9685 `OUT0` through `OUT7` to ULN2803A `IN1` through `IN8`
- ULN2803A `OUT1` through `OUT8` to each motor negative terminal
- Motor positive terminals to motor supply positive
- ULN2803A `GND` to common ground
- ULN2803A `COM` to motor supply positive so the internal clamp diodes can catch motor kickback

The ULN2803A is a Darlington sink array, so each motor will see less than the full motor supply voltage. For high-current motors or maximum vibration strength, logic-level MOSFETs are usually better.

## Commands

```powershell
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor
```

The firmware retries PCA9685 detection every 2 seconds and only runs motor patterns after the board responds at I2C address `0x40`.
