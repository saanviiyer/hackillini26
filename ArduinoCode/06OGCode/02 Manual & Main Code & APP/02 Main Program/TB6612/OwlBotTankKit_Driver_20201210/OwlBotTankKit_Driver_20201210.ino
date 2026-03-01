/*
 * Minimal OwlBot Tank (TB6612) build for Arduino Nano/Uno (ATmega328P)
 * - Reads I2C ultrasonic at 0x21 (register 0x00)
 * - Drives forward while distance >= 100mm
 * - Stops when distance < 100mm
 * - Servo oscillates gently while moving
 * - Prints distance to Serial
 *
 * This replaces the heavy Elegoo framework so the sketch fits in flash.
 */

#include <avr/wdt.h>
#include "ApplicationFunctionSet_xxx0.h"

void setup()
{
  Application_FunctionSet.ApplicationFunctionSet_Init();
  Application_FunctionSet.ApplicationFunctionSet_Bootup();
  wdt_enable(WDTO_2S);
}

void loop()
{
  wdt_reset();

  // Update sensor at a safe rate + print
  Application_FunctionSet.ApplicationFunctionSet_SensorDataUpdate();

  // Main behavior
  Application_FunctionSet.ApplicationFunctionSet_Obstacle();
}
