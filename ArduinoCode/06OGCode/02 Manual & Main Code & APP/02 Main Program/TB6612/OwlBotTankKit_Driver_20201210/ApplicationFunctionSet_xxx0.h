#pragma once
#include <Arduino.h>

// Keep the same global instance name your original sketch used
class ApplicationFunctionSet
{
public:
  void ApplicationFunctionSet_Init();
  void ApplicationFunctionSet_Bootup();
  void ApplicationFunctionSet_SensorDataUpdate();
  void ApplicationFunctionSet_Obstacle();

  // The original framework calls many other methods; we keep no-op stubs
  // so older sketches still compile if they accidentally call them.
  inline void ApplicationFunctionSet_KeyCommand() {}
  inline void ApplicationFunctionSet_RGB() {}
  inline void ApplicationFunctionSet_Rocker() {}
  inline void ApplicationFunctionSet_Tracking() {}
  inline void ApplicationFunctionSet_Exploration() {}
  inline void ApplicationFunctionSet_Standby() {}
  inline void ApplicationFunctionSet_Expression() {}
  inline void ApplicationFunctionSet_SerialPortDataAnalysis() {}
  inline void CMD_MotorControl_xxx0() {}
  inline void CMD_CarControlTimeLimit_xxx0() {}
  inline void CMD_CarControlNoTimeLimit_xxx0() {}
  inline void CMD_MotorControlSpeed_xxx0() {}
  inline void CMD_VoiceControl_xxx0() {}
  inline void CMD_LightingControlTimeLimit_xxx0() {}
  inline void CMD_LightingControlNoTimeLimit_xxx0() {}
  inline void CMD_LEDCustomExpressionControl_xxx0() {}
  inline void CMD_LEDNumberDisplayControl_xxx0() {}
  inline void CMD_TrajectoryControl_xxx0() {}
  inline void CMD_ClearAllFunctions_xxx0() {}

  // Expose latest readings if you want to use them elsewhere
  uint16_t UltrasoundData_mm = 0;
  uint16_t UltrasoundData_cm = 0;
};

extern ApplicationFunctionSet Application_FunctionSet;
