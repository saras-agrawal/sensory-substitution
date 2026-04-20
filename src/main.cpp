#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const uint8_t PCA_ADDRESS = 0x40;
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(PCA_ADDRESS);

const uint8_t SDA_PIN = 21;
const uint8_t SCL_PIN = 22;
const uint16_t PWM_FREQ = 500;

const uint8_t NUM_MOTORS = 16;
const uint8_t motorChannels[NUM_MOTORS] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15,
};

const uint16_t MOTOR_OFF = 0;
const uint16_t MOTOR_LOW = 1200;
const uint16_t MOTOR_MED = 2500;
const uint16_t MOTOR_HIGH = 4095;
const uint16_t DEFAULT_TEST_MS = 1000;

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
    buzzMotor(i, MOTOR_HIGH, DEFAULT_TEST_MS);
    delay(300);
  }
}

void scanI2cBus() {
  Serial.println("Scanning I2C bus...");
  uint8_t found = 0;

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C devices found.");
  }
}

void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  help                  Show this list");
  Serial.println("  status                Print PCA9685 status");
  Serial.println("  scan                  Scan I2C bus");
  Serial.println("  test                  Run each motor at full power");
  Serial.println("  pattern               Run startup pattern");
  Serial.println("  off                   Turn all motors off");
  Serial.println("  all <pwm>             Set all motors, pwm 0-4095");
  Serial.println("  m <idx> <pwm> <ms>    Buzz one motor, example: m 0 4095 2000");
  Serial.println();
}

void printStatus() {
  Serial.print("PCA9685 address: 0x");
  Serial.println(PCA_ADDRESS, HEX);
  Serial.print("PCA9685 ready: ");
  Serial.println(pcaReady ? "yes" : "no");
  Serial.print("SDA pin: ");
  Serial.println(SDA_PIN);
  Serial.print("SCL pin: ");
  Serial.println(SCL_PIN);
  Serial.print("PWM frequency: ");
  Serial.println(PWM_FREQ);
}

void setAllMotors(uint16_t intensity) {
  for (uint8_t i = 0; i < NUM_MOTORS; i++) {
    setMotorRaw(i, intensity);
  }
}

void handleCommand(String command) {
  command.trim();
  command.toLowerCase();
  if (command.length() == 0) return;

  if (command == "help") {
    printHelp();
    return;
  }

  if (command == "status") {
    printStatus();
    return;
  }

  if (command == "scan") {
    scanI2cBus();
    return;
  }

  if (!pcaReady) {
    Serial.println("PCA9685 is not ready yet.");
    return;
  }

  if (command == "test") {
    testEachMotor();
    allMotorsOff();
    Serial.println("Motor test complete.");
    return;
  }

  if (command == "pattern") {
    startupPattern();
    allMotorsOff();
    Serial.println("Pattern complete.");
    return;
  }

  if (command == "off") {
    allMotorsOff();
    Serial.println("All motors off.");
    return;
  }

  int motorIndex = -1;
  int pwmValue = 0;
  int durationMs = 0;

  if (sscanf(command.c_str(), "m %d %d %d", &motorIndex, &pwmValue, &durationMs) == 3) {
    if (motorIndex < 0 || motorIndex >= NUM_MOTORS) {
      Serial.println("Motor index out of range.");
      return;
    }
    if (pwmValue < 0) pwmValue = 0;
    if (pwmValue > 4095) pwmValue = 4095;
    if (durationMs < 0) durationMs = 0;

    Serial.print("Buzzing motor ");
    Serial.print(motorIndex);
    Serial.print(" at ");
    Serial.print(pwmValue);
    Serial.print(" for ");
    Serial.print(durationMs);
    Serial.println(" ms.");
    buzzMotor(static_cast<uint8_t>(motorIndex), static_cast<uint16_t>(pwmValue), static_cast<uint32_t>(durationMs));
    Serial.println("Done.");
    return;
  }

  if (sscanf(command.c_str(), "all %d", &pwmValue) == 1) {
    if (pwmValue < 0) pwmValue = 0;
    if (pwmValue > 4095) pwmValue = 4095;

    setAllMotors(static_cast<uint16_t>(pwmValue));
    Serial.print("All motors set to ");
    Serial.println(pwmValue);
    return;
  }

  Serial.println("Unknown command. Type help.");
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  delay(500);

  Serial.println();
  Serial.println("Starting PCA9685 motor controller...");

  Wire.begin(SDA_PIN, SCL_PIN);
  pcaReady = initPca9685();
  if (!pcaReady) {
    printHelp();
    return;
  }

  allMotorsOff();

  Serial.println("Running startup pattern...");
  startupPattern();

  Serial.println("Running motor test at full power...");
  testEachMotor();
  allMotorsOff();
  Serial.println("Setup complete.");
  printHelp();
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
        Serial.println("Running motor test at full power...");
        testEachMotor();
        allMotorsOff();
        Serial.println("Setup complete.");
        printHelp();
      }
    }
    if (Serial.available() > 0) {
      handleCommand(Serial.readStringUntil('\n'));
    }
    return;
  }

  if (Serial.available() > 0) {
    handleCommand(Serial.readStringUntil('\n'));
  }
}
