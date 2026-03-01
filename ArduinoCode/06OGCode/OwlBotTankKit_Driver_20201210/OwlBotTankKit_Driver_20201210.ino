/*
 * @Author: ELEGOO
 * @Date: 2019-07-10 16:46:17
 * @LastEditTime: 2021-01-06 16:00:26
 * @LastEditors: Changhua
 * @Description: OwlBot Tank Kit
 * @FilePath: 
 */

#include <avr/wdt.h>
#include "ApplicationFunctionSet_xxx0.h"
#include <hardwareSerial.h>
void setup()
{
  // put your setup code here, to run once:
  Application_FunctionSet.ApplicationFunctionSet_Init();
  Application_FunctionSet.ApplicationFunctionSet_Bootup();
  wdt_enable(WDTO_2S);
}

void loop()
{
  wdt_reset();

  // Autonomous demo:
  // Drive forward until the ultrasonic distance is < 100 mm, and print distance to Serial Monitor.
  Application_FunctionSet.ApplicationFunctionSet_ForwardUntilDistanceLessThan(100 /*mm*/, 200 /*speed*/);
}
