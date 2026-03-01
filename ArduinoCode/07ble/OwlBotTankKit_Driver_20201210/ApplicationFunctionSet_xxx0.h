/*
 * @Author: ELEGOO
 * @Date: 2019-07-10 16:46:17
 * @LastEditTime: 2020-09-16 10:40:11
 * @LastEditors: Changhua
 * @Description: OwlBot Tank Kit
 * @FilePath: 
 */

#ifndef _ApplicationFunctionSet_xxx0_H_
#define _ApplicationFunctionSet_xxx0_H_

#include <arduino.h>

class ApplicationFunctionSet
{
public:
  void ApplicationFunctionSet_Init(void);
  void ApplicationFunctionSet_Bootup(void);
  void ApplicationFunctionSet_RGB(void);
  void ApplicationFunctionSet_Expression(void);             //表情面板显示
  void ApplicationFunctionSet_Rocker(void);                 //摇杆
  void ApplicationFunctionSet_Exploration(void);            //探索
  void ApplicationFunctionSet_Tracking(void);               //循迹
  void ApplicationFunctionSet_Obstacle(void);               //避障
  void ApplicationFunctionSet_Standby(void);                //待机
  void ApplicationFunctionSet_KeyCommand(void);             //按键命令
  void ApplicationFunctionSet_SensorDataUpdate(void);       //传感器数据更新
  void ApplicationFunctionSet_SerialPortDataAnalysis(void); //串口数据解析

public: /*CMD*/
  void CMD_UltrasoundModuleStatus_xxx0(uint8_t is_get);
  void CMD_TraceModuleStatus_xxx0(uint8_t is_get);
  void CMD_Car_LeaveTheGround_xxx0(uint8_t is_get);
  void CMD_MotorControl_xxx0(void);
  void CMD_CarControlTimeLimit_xxx0(void);
  void CMD_CarControlNoTimeLimit_xxx0(void);
  void CMD_MotorControlSpeed_xxx0(void);
  void CMD_ServoControl_xxx0(void);
  void CMD_VoiceControl_xxx0(void);
  void CMD_LightingControlTimeLimit_xxx0(void);
  void CMD_LightingControlNoTimeLimit_xxx0(void);
  void CMD_LEDCustomExpressionControl_xxx0(void);
  void CMD_ClearAllFunctions_xxx0(void);
  void CMD_LEDNumberDisplayControl_xxx0(void);
  void CMD_TrajectoryControl_xxx0(void);

public:
  /*传感器数据*/
  volatile float VoltageData_V;        //电压数据
  volatile uint16_t UltrasoundData_mm; //超声波数据
  volatile uint16_t UltrasoundData_cm; //超声波数据
  volatile uint16_t TrackingData_R;    //循迹数据
  volatile uint16_t TrackingData_L;
  volatile uint16_t TrackingData_M;
  //volatile float mpu6050_Yaw; //偏航
public:
  /*传感器状态*/
  boolean VoltageDetectionStatus = false;
  boolean UltrasoundDetectionStatus = false;
  boolean TrackingDetectionStatus_R = false;
  boolean TrackingDetectionStatus_L = false;
  boolean TrackingDetectionStatus_M = false;
  boolean Car_LeaveTheGround = true;

public:
  String CommandSerialNumber;
  uint8_t Rocker_CarSpeed = 255;

public: /*阈值设定*/
#define VoltageDetection 3.70
#define ObstacleDetection 30
  //#define ObstacleDetection 25

public: //Tracking
  // uint16_t TrackingDetection_S = 100;
  // uint16_t TrackingDetection_E = 900;
  uint16_t TrackingDetection_S = 400;
  uint16_t TrackingDetection_E = 850;
  uint16_t TrackingDetection_CarLeaveTheGround = 850;

public: //motor
  uint8_t CMD_is_MotorSelection;
  uint8_t CMD_is_MotorDirection;
  uint8_t CMD_is_MotorSpeed;
  uint32_t CMD_is_MotorTimer;
  uint8_t CMD_is_MotorSpeed_L;
  uint8_t CMD_is_MotorSpeed_R;
  uint8_t CMD_is_MotorSpeed_M;

public: //car
  uint8_t CMD_is_CarDirection;
  uint8_t CMD_is_CarSpeed;
  uint32_t CMD_is_CarTimer;

public: //voice
  uint8_t CMD_is_VoiceName;
  uint16_t CMD_is_controlAudio;
  uint32_t CMD_is_VoiceTimer;

public: //Lighting (Left, front, right, back and center)
  uint8_t CMD_is_LightingSequence;
  uint8_t CMD_is_LightingColorValue_R;
  uint8_t CMD_is_LightingColorValue_G;
  uint8_t CMD_is_LightingColorValue_B;
  uint32_t CMD_is_LightingTimer;

public: //LED Custom Expression Control
  uint8_t CMD_is_LEDCustomExpression_arry[16];
  uint8_t CMD_is_LEDNumber;
  //uint16_t is_arry[8];

public: //Trajectory
  uint16_t CMD_is_TrajectoryControl_axisPlaneData_X;
  uint16_t CMD_is_TrajectoryControl_axisPlaneData_Y;

public: //LED
  uint8_t CMD_is_FastLED_setBrightness = 20;
};
extern ApplicationFunctionSet Application_FunctionSet;

#endif