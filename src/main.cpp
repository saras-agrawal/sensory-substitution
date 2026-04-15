#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const uint8_t PCA_ADDRESS = 0x40;
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(PCA_ADDRESS);

const uint8_t SDA_PIN = 21;
const uint8_t SCL_PIN = 22;
const uint16_t PWM_FREQ = 500;

const uint8_t NUM_MOTORS = 8;
const uint8_t motorChannels[NUM_MOTORS] = {0, 1, 2, 3, 4, 5, 6, 7};

const uint16_t MOTOR_OFF = 0;
const uint16_t MOTOR_LOW = 1200;
const uint16_t MOTOR_MED = 2500;
const uint16_t MOTOR_HIGH = 4095;

bool pcaReady = false;

bool i2cDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool initPca9685() {
  if (!i2cDevicePresent(PCA_ADDRESS)) {
    Serial.print("PCA9685 not found at 0x");
    Serial.print(PCA_ADDRESS, HEX);
    Serial.println(". Check SDA=21, SCL=22, VCC, and shared GND.");
    return false;
  }

  pca.begin();
  pca.setPWMFreq(PWM_FREQ);
  delay(100);
  Serial.println("PCA9685 detected and initialized.");
  return true;
}

void setMotorRaw(uint8_t motorIndex, uint16_t pwmValue) {
  if (!pcaReady) return;
  if (motorIndex >= NUM_MOTORS) return;
  if (pwmValue > 4095) pwmValue = 4095;

  const uint8_t channel = motorChannels[motorIndex];

  if (pwmValue == MOTOR_OFF) {
    pca.setPWM(channel, 0, 0);
  } else if (pwmValue >= MOTOR_HIGH) {
    pca.setPWM(channel, 4096, 0);
  } else {
    pca.setPWM(channel, 0, pwmValue);
  }
}

void allMotorsOff() {
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    setMotorRaw(i, MOTOR_OFF);
  }
}

void buzzMotor(uint8_t motorIndex, uint16_t intensity, uint32_t durationMs) {
  setMotorRaw(motorIndex, intensity);
  delay(durationMs);
  setMotorRaw(motorIndex, MOTOR_OFF);
}

void pulseMotor(uint8_t motorIndex, uint16_t intensity, uint16_t onMs, uint16_t offMs, uint8_t repeats) {
  for (uint8_t i = 0; i < repeats; i++) {
    setMotorRaw(motorIndex, intensity);
    delay(onMs);
    setMotorRaw(motorIndex, MOTOR_OFF);
    delay(offMs);
  }
}

void sweepMotors(uint16_t intensity, uint16_t onMs) {
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    setMotorRaw(i, intensity);
    delay(onMs);
    setMotorRaw(i, MOTOR_OFF);
  }
}

void startupPattern() {
  sweepMotors(MOTOR_MED, 100);
  delay(150);

  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    setMotorRaw(i, MOTOR_LOW);
  }
  delay(250);
  allMotorsOff();

  delay(150);
  pulseMotor(0, MOTOR_HIGH, 80, 80, 3);
}

void alternatingPattern() {
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t m = 0; m < NUM_MOTORS; m++) {
      setMotorRaw(m, (m % 2 == 0) ? MOTOR_MED : MOTOR_OFF);
    }
    delay(150);

    for (uint8_t m = 0; m < NUM_MOTORS; m++) {
      setMotorRaw(m, (m % 2 == 1) ? MOTOR_MED : MOTOR_OFF);
    }
    delay(150);
  }

  allMotorsOff();
}

void testEachMotor() {
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    Serial.print("Testing motor ");
    Serial.println(i);
    buzzMotor(i, MOTOR_MED, 300);
    delay(150);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Starting PCA9685 motor controller...");

  Wire.begin(SDA_PIN, SCL_PIN);
  pcaReady = initPca9685();
  if (!pcaReady) return;

  allMotorsOff();

  Serial.println("Running startup pattern...");
  startupPattern();

  Serial.println("Running motor test...");
  testEachMotor();
  Serial.println("Setup complete.");
}

void loop() {
  if (!pcaReady) {
    static uint32_t lastRetryMs = 0;
    const uint32_t now = millis();
    if (now - lastRetryMs >= 2000) {
      lastRetryMs = now;
      pcaReady = initPca9685();
      if (pcaReady) {
        allMotorsOff();
        Serial.println("Running startup pattern...");
        startupPattern();
        Serial.println("Running motor test...");
        testEachMotor();
        Serial.println("Setup complete.");
      }
    }
    return;
  }

  buzzMotor(0, MOTOR_HIGH, 200);
  delay(400);

  pulseMotor(3, MOTOR_MED, 100, 100, 3);
  delay(500);

  sweepMotors(MOTOR_LOW, 80);
  delay(500);

  alternatingPattern();
  delay(1000);
}
