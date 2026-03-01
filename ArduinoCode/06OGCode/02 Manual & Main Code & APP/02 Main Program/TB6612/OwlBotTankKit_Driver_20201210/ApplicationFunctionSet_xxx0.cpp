#include "ApplicationFunctionSet_xxx0.h"
#include <Wire.h>
#include <Servo.h>

// ====== TB6612 motor pins (OwlBot board) ======
static const uint8_t PIN_Motor_STBY = 4;
static const uint8_t PIN_Motor_PWMA = 9;
static const uint8_t PIN_Motor_PWMB = 6;
static const uint8_t PIN_Motor_AIN  = 8;
static const uint8_t PIN_Motor_BIN  = 7;

// ====== Servo pin ======
static const uint8_t SERVO_PIN = 3;

// ====== Ultrasonic I2C ======
static const uint8_t ULTRASONIC_ADDR = 0x21; // from your I2C scan

// ====== Behavior ======
static const uint16_t STOP_DISTANCE_MM = 100;
static const uint8_t  DRIVE_SPEED      = 200;

// Global instance
ApplicationFunctionSet Application_FunctionSet;

// Servo object
static Servo gServo;
static bool gServoReady = false;

// -------- Motor helpers --------
static void motorsInit()
{
  pinMode(PIN_Motor_STBY, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);
  pinMode(PIN_Motor_AIN,  OUTPUT);
  pinMode(PIN_Motor_BIN,  OUTPUT);

  digitalWrite(PIN_Motor_STBY, LOW);
  analogWrite(PIN_Motor_PWMA, 0);
  analogWrite(PIN_Motor_PWMB, 0);
}

static void motorsForward(uint8_t speed)
{
  digitalWrite(PIN_Motor_STBY, HIGH);
  digitalWrite(PIN_Motor_AIN, HIGH);
  digitalWrite(PIN_Motor_BIN, HIGH);
  analogWrite(PIN_Motor_PWMA, speed);
  analogWrite(PIN_Motor_PWMB, speed);
}

static void motorsStop()
{
  analogWrite(PIN_Motor_PWMA, 0);
  analogWrite(PIN_Motor_PWMB, 0);
  digitalWrite(PIN_Motor_AIN, LOW);
  digitalWrite(PIN_Motor_BIN, LOW);
  digitalWrite(PIN_Motor_STBY, LOW);
}

// -------- Servo oscillation --------
static void servoEnsure()
{
  if (!gServoReady)
  {
    gServo.attach(SERVO_PIN);
    gServo.write(90);
    gServoReady = true;
  }
}

static void servoUpdate(bool enabled)
{
  servoEnsure();

  static const int center = 90;
  static const int range  = 15;
  static int angle = center;
  static int step  = 1;
  static unsigned long t0 = 0;

  if (!enabled)
  {
    gServo.write(center);
    return;
  }

  if (millis() - t0 < 25) return;
  t0 = millis();

  angle += step;
  if (angle >= center + range) { angle = center + range; step = -step; }
  if (angle <= center - range) { angle = center - range; step = -step; }
  gServo.write(angle);
}

// -------- Ultrasonic (robust, command-triggered) --------
//
// Many I2C ultrasonic modules do NOT stream distance continuously.
// Instead you must:
//   1) Write a "ranging command" to register 0x00
//   2) Wait for measurement (~65ms typical)
//   3) Read result registers (commonly 0x02,0x03)
//
// This is the classic SRFxx-style protocol; it matches your symptom of
// alternating 0 and a fixed garbage word (0xB1A1).
//
static bool isPlausible(uint16_t mm)
{
  if (mm == 0) return false;
  if (mm == 0xFFFF) return false;
  if (mm == 45473) return false;   // 0xB1A1 garbage
  if (mm > 5000) return false;     // indoors on this bot
  return true;
}

// Trigger a measurement in centimeters (0x51 is common) and then read range registers.
static uint16_t readUltrasonicMM_triggered()
{
  // 1) Trigger ranging (command 0x51 = range in cm on many modules)
  Wire.beginTransmission(ULTRASONIC_ADDR);
  Wire.write((uint8_t)0x00);
  Wire.write((uint8_t)0x51);
  if (Wire.endTransmission() != 0)
    return 0xFFFF;

  // 2) Wait for measurement to complete
  delay(70);

  // 3) Read result registers 0x02 (high byte), 0x03 (low byte)
  Wire.beginTransmission(ULTRASONIC_ADDR);
  Wire.write((uint8_t)0x02);
  if (Wire.endTransmission(false) != 0)
    return 0xFFFF;

  Wire.requestFrom(ULTRASONIC_ADDR, (uint8_t)2);
  if (Wire.available() >= 2)
  {
    uint16_t cm = ((uint16_t)Wire.read() << 8) | (uint16_t)Wire.read();
    // Some modules return 0 when too close / out of range; keep as-is for plausibility check.
    return (uint16_t)(cm * 10);
  }

  while (Wire.available()) Wire.read();
  return 0xFFFF;
}

// Fallback: pointer read (in case your module is different)
static uint16_t readUltrasonicMM_pointer(uint8_t reg)
{
  Wire.beginTransmission(ULTRASONIC_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 0xFFFF;

  delay(2);
  Wire.requestFrom(ULTRASONIC_ADDR, (uint8_t)2);
  if (Wire.available() >= 2)
  {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    return ((uint16_t)hi << 8) | lo;
  }
  while (Wire.available()) Wire.read();
  return 0xFFFF;
}

static uint16_t readUltrasonicMM()
{
  // Try triggered protocol first (most likely for your module)
  uint16_t mm = readUltrasonicMM_triggered();
  if (isPlausible(mm)) return mm;

  // Fallback: try a few pointer registers
  const uint8_t regs[] = {0x00, 0x01, 0x02};
  for (uint8_t i = 0; i < sizeof(regs); i++)
  {
    uint16_t mm2 = readUltrasonicMM_pointer(regs[i]);
    if (isPlausible(mm2)) return mm2;
  }

  return mm; // return last (even if invalid) so caller can see failure pattern
}

// ===== ApplicationFunctionSet methods =====

void ApplicationFunctionSet::ApplicationFunctionSet_Init()
{
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  motorsInit();
  servoEnsure();
  Serial.println(F("OwlBot minimal v4: trigger-read ultrasonic, stop <100mm"));
}

void ApplicationFunctionSet::ApplicationFunctionSet_Bootup()
{
  // kept for compatibility
}

void ApplicationFunctionSet::ApplicationFunctionSet_SensorDataUpdate()
{
  // Triggered reads already include ~70ms wait; don't call too fast.
  static unsigned long us_t0 = 0;
  static uint16_t lastGood = 0;
  static bool haveGood = false;

  if (millis() - us_t0 < 120) return; // ~8 Hz
  us_t0 = millis();

  uint16_t mm = readUltrasonicMM();

  if (isPlausible(mm))
  {
    lastGood = mm;
    haveGood = true;
  }
  else if (haveGood)
  {
    mm = lastGood;
  }

  UltrasoundData_mm = mm;
  UltrasoundData_cm = mm / 10;

  Serial.print(F("Ultrasonic mm: "));
  Serial.print(UltrasoundData_mm);
  Serial.print(F("  cm: "));
  Serial.println(UltrasoundData_cm);
}

void ApplicationFunctionSet::ApplicationFunctionSet_Obstacle()
{
  if (isPlausible(UltrasoundData_mm) && UltrasoundData_mm < STOP_DISTANCE_MM)
  {
    motorsStop();
    servoUpdate(false);
  }
  else
  {
    motorsForward(DRIVE_SPEED);
    servoUpdate(true);
  }
}
