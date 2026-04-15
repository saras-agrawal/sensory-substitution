# ESP32 PCA9685 Motor Controller

Firmware for an ESP32-WROOM driving vibration motors through a PCA9685 PWM controller and ULN2803A low-side driver.

The PCA9685 should drive the ULN2803A inputs, not motors directly. Tie ESP32, PCA9685, ULN2803A, and motor-supply grounds together.

## Wiring

- ESP32 `3.3V` to PCA9685 `VCC`
- ESP32 `GND` to PCA9685 `GND`
- ESP32 `GPIO21` to PCA9685 `SDA`
- ESP32 `GPIO22` to PCA9685 `SCL`
- PCA9685 `OE` to `GND` or otherwise held low
- PCA9685 `OUT0` through `OUT7` to ULN2803A `IN1` through `IN8`
- ULN2803A `OUT1` through `OUT8` to each motor negative terminal
- Motor positive terminals to motor supply positive
- ULN2803A `GND` to common ground
- ULN2803A `COM` to motor supply positive so the internal clamp diodes can catch motor kickback

For a DIP ULN2803A, the pin mapping is:

- `IN1` pin 1 -> `OUT1` pin 18
- `IN2` pin 2 -> `OUT2` pin 17
- `IN3` pin 3 -> `OUT3` pin 16
- `IN4` pin 4 -> `OUT4` pin 15
- `IN5` pin 5 -> `OUT5` pin 14
- `IN6` pin 6 -> `OUT6` pin 13
- `IN7` pin 7 -> `OUT7` pin 12
- `IN8` pin 8 -> `OUT8` pin 11
- `GND` pin 9 -> common ground
- `COM` pin 10 -> motor supply positive

The ULN2803A is a Darlington sink array, so each motor will see less than the full motor supply voltage. For high-current motors or maximum vibration strength, logic-level MOSFETs are usually better.

If the serial monitor says the PCA9685 is detected but motors do not move, check the ULN2803A side with a multimeter:

- PCA9685 output pin should rise when a channel is on.
- Matching ULN2803A output pin should pull low when that input is high.
- Motor positive should stay at the motor supply voltage.
- Motor negative should be connected to the matching ULN2803A output, not ground directly.

## Commands

```powershell
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor
```

The firmware retries PCA9685 detection every 2 seconds and only runs motor patterns after the board responds at I2C address `0x40`.

Serial monitor commands:

```text
help
status
scan
test
pattern
off
all 4095
m 0 4095 2000
```
