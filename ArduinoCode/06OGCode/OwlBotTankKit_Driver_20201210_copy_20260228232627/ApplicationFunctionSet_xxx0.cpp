/*
 * @Author: ELEGOO
 * @Date: 2019-07-10 16:46:17
 * @LastEditTime: 2021-01-06 15:57:27
 * @LastEditors: Changhua
 * @Description: OwlCar Tank Kit
 * @FilePath: 
 */
#include <hardwareSerial.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
//#include "MsTimer2.h"
#include "ApplicationFunctionSet_xxx0.h"
#include "DeviceDriverSet_xxx0.h"
#include "TM1640.h"
#include "ArduinoJson-v6.11.1.h" //ArduinoJson

// #include <Servo.h>

#define _is_print 1
#define _Test_print 0 //When testing, remember to set 0 after using the test to save controller resources and load.
ApplicationFunctionSet Application_FunctionSet;
/*硬件设备成员对象序列*/
//MPU6050_getdata AppMPU6050getdata;
TM1640 Apptm1640;
DeviceDriverSet_RBGLED AppRBG_LED;
DeviceDriverSet_passiveBuzzer ApppassiveBuzzer;
DeviceDriverSet_Key AppKey;
DeviceDriverSet_ITR20001 AppITR20001;
DeviceDriverSet_Voltage AppVoltage;
DeviceDriverSet_Motor AppMotor;
DeviceDriverSet_ULTRASONIC AppULTRASONIC;
DeviceDriverSet_STM8S003F3_IR AppSTM8S003F3_IR;
DeviceDriverSet_MPU6050 AppMPU6050getdata;

// Servo scanServo;

// static const uint8_t SERVO_PIN = 11;   // IMPORTANT: do NOT use 9 or 6 (motor PWM pins)
// static const int servoCenter = 90;     // neutral position
// static int servoOffset = 0;            // current oscillation offset
// static int servoDirection = 1;         // +1 or -1
// static unsigned long lastServoMove = 0;
// static bool servoAttached = false;




/*f(x) int */
static boolean
function_xxx(long x, long s, long e) //f(x)
{
  if (s <= x && x <= e)
    return true;
  else
    return false;
}

/*运动方向控制序列*/
enum OwlCarMotionControl
{
  Forward,       //(1)
  Backward,      //(2)
  Left,          //(3)
  Right,         //(4)
  LeftForward,   //(5)
  LeftBackward,  //(6)
  RightForward,  //(7)
  RightBackward, //(8)
  stop_it        //(9)
};               //direction:前行（1）、后退（2）、 左前（3）、右前（4）、后左（5）、后右（6）

/*模式控制序列*/
enum OwlCarFunctionalModel
{
  Standby_mode,           /*空闲模式*/
  TraceBased_mode,        /*循迹模式*/
  ObstacleAvoidance_mode, /*避障模式*/
  Rocker_mode,            /*摇杆模式*/
  Exploration_mode,

  CMD_Programming_mode,                   /*编程模式*/
  CMD_ClearAllFunctions_Standby_mode,     /*清除所有功能：进入空闲模式*/
  CMD_ClearAllFunctions_Programming_mode, /*清除所有功能：进入编程模式*/
  CMD_MotorControl,                       /*电机控制模式*/
  CMD_CarControl_TimeLimit,               /*小车方向控制：有时间限定模式*/
  CMD_CarControl_NoTimeLimit,             /*小车方向控制：无时间限定模式*/
  CMD_MotorControl_Speed,                 /*电机控制:控制转速模式*/
  CMD_ServoControl,                       /*舵机控制:模式*/
  CMD_VoiceControl,                       /*声音控制:模式*/
  CMD_ledExpressionControl,               /*矩阵表情控制:模式*/
  CMD_ledNumberControl,                   /*矩阵数字控制:模式*/
  CMD_LightingControl_TimeLimit,          /*灯光控制:模式*/
  CMD_LightingControl_NoTimeLimit,        /*灯光控制:模式*/
  CMD_TrajectoryControl,                  /*轨迹控制:模式*/
};

/*控制管理成员*/
struct Application_xxx
{
  OwlCarMotionControl Motion_Control;
  OwlCarFunctionalModel Functional_Mode;
  unsigned long CMD_CarControl_Millis;
  unsigned long CMD_LightingControl_Millis;
  float AppMPU6050getdata_yaw;
};
Application_xxx Application_OwlCarxxx0;

static void ApplicationFunctionSet_MetalCarLinearMotionControl(OwlCarMotionControl direction, uint8_t directionRecord, uint8_t speed, uint8_t Kp, uint8_t UpperLimit);
static void ApplicationFunctionSet_OwlCarMotionControl(OwlCarMotionControl direction, uint8_t speed);
static void MsTimer2_MPU6050getdata(void);
void ApplicationFunctionSet::ApplicationFunctionSet_Init(void)
{
  Serial.begin(9600);



  AppSTM8S003F3_IR.DeviceDriverSet_STM8S003F3_IR_Init();
  AppMPU6050getdata.DeviceDriverSet_MPU6050_Init();
  Apptm1640.TM1640_InitConfig_led16x8(true);
  ApppassiveBuzzer.DeviceDriverSet_passiveBuzzer_Init();
  AppKey.DeviceDriverSet_Key_Init();
  AppMotor.DeviceDriverSet_Motor_Init();
  AppRBG_LED.DeviceDriverSet_RBGLED_Init(20);
  AppULTRASONIC.DeviceDriverSet_ULTRASONIC_Init();

  // scanServo.attach(SERVO_PIN);
  // scanServo.write(servoCenter);
  // servoAttached = true;

}

static bool ApplicationFunctionSet_OwlCarLeaveTheGround(void)
{
  AppSTM8S003F3_IR.DeviceDriverSet_STM8S003F3_IR_Get(&Application_FunctionSet.TrackingData_L /*uint16_t * STM8S003F3_IRL out*/,
                                                     &Application_FunctionSet.TrackingData_M /*uint16_t * STM8S003F3_IRM out*/,
                                                     &Application_FunctionSet.TrackingData_R /*uint16_t * STM8S003F3_IRR out*/);
  /*R循迹状态更新*/
  Application_FunctionSet.TrackingDetectionStatus_R = function_xxx(Application_FunctionSet.TrackingData_R, Application_FunctionSet.TrackingDetection_S, Application_FunctionSet.TrackingDetection_E);
  /*M循迹状态更新*/
  Application_FunctionSet.TrackingDetectionStatus_M = function_xxx(Application_FunctionSet.TrackingData_M, Application_FunctionSet.TrackingDetection_S, Application_FunctionSet.TrackingDetection_E);
  /*L循迹状态更新*/
  Application_FunctionSet.TrackingDetectionStatus_L = function_xxx(Application_FunctionSet.TrackingData_L, Application_FunctionSet.TrackingDetection_S, Application_FunctionSet.TrackingDetection_E);
  //检测小车是否离开地面
  if (Application_FunctionSet.TrackingData_L > Application_FunctionSet.TrackingDetection_CarLeaveTheGround &&
      Application_FunctionSet.TrackingData_M > Application_FunctionSet.TrackingDetection_CarLeaveTheGround &&
      Application_FunctionSet.TrackingData_R > Application_FunctionSet.TrackingDetection_CarLeaveTheGround)
  {
    Application_FunctionSet.Car_LeaveTheGround = false;
  }
  else
  {
    Application_FunctionSet.Car_LeaveTheGround = true;
  }
}
/*
  电机调速调整：驱动履带的电机需要更大的驱动量
*/
static void MotorSpeedAdjustment(uint8_t *is_Speed)
{
  if (100 == *is_Speed) //150
  {
    *is_Speed = 150;
  }
  else if (150 == *is_Speed) //200
  {
    *is_Speed = 200;
  }
  else if (250 == *is_Speed) //255
  {
    *is_Speed = 255;
  }
}


// static void UpdateServoOscillation()
// {
//   if (!servoAttached) return;

//   // Update every ~30ms for smooth motion
//   if (millis() - lastServoMove >= 30)
//   {
//     lastServoMove = millis();

//     servoOffset += servoDirection;

//     if (servoOffset >= 15) servoDirection = -1;
//     if (servoOffset <= -15) servoDirection = 1;

//     scanServo.write(servoCenter + servoOffset);
//   }
// }

// ============================================================
// UPDATED FUNCTION (copy/paste into your ApplicationFunctionSet_xxx0.cpp)
// Replaces: ApplicationFunctionSet::ApplicationFunctionSet_ForwardUntilDistanceLessThan(...)
//
// Behavior:
// - Normally drives forward while distance >= stopDistance_mm
// - If distance stays < stopDistance_mm continuously for > 1 second,
//   it performs an "escape": back up, then rotate ~180° (timed spin),
//   then resumes normal behavior.
//
// Tuning notes:
// - BACKUP_MS controls how far it backs up
// - TURN180_MS controls how close to 180° the rotation is (depends on battery/surface)
// ============================================================

void ApplicationFunctionSet::ApplicationFunctionSet_ForwardUntilDistanceLessThan(uint16_t stopDistance_mm, uint8_t speed)
{
  static bool wasMoving = false;

  // --- keep last good distance ---
  static uint16_t lastGoodD = 5000;
  static unsigned long lastGoodMs = 0;

  ApplicationFunctionSet_SensorDataUpdate();
  uint16_t dRaw = UltrasoundData_mm;

  // Treat "no echo" / invalid special values as FAR (so we keep moving)
  bool isNoEcho = (dRaw >= 65531);          // catches 65531..65535 (and 65534, 65532, etc.)
  bool isPlausible = (dRaw > 0 && dRaw < 5000);

  uint16_t d;
  if (isNoEcho)
  {
    d = 5000; // FAR away -> keep driving
  }
  else if (isPlausible)
  {
    d = dRaw;
    lastGoodD = dRaw;
    lastGoodMs = millis();
  }
  else
  {
    // For other weird values, use last good value if it's recent; otherwise FAR
    if (millis() - lastGoodMs < 300)
      d = lastGoodD;
    else
      d = 5000;
  }

  // Debug print
  static unsigned long lastPrintMs = 0;
  if (millis() - lastPrintMs >= 100)
  {
    lastPrintMs = millis();
    Serial.print("Distance(mm): ");
    Serial.print(dRaw);
    if (isNoEcho) Serial.print("  (no echo -> treated as FAR)");
    Serial.println();
  }

  // Normal behavior: drive forward if far enough, else stop
  if (d >= stopDistance_mm)
  {
    ApplicationFunctionSet_OwlCarMotionControl(Forward, speed);
    wasMoving = true;
  }
  else
  {
    ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    wasMoving = false;
  }
}



/*
  直线运动控制：(对于双驱动电机，因为电机系数偏差频繁以及外界干扰因素众多，致使小车很难实现相对直线运动，为此加入偏航控制环作用反馈)
  direction：方向选择 前/后
  directionRecord：方向记录（作用于首次进入该函数时更新方向位置数据，即:yaw偏航）
  speed：输入速度 （0--250）
  Kp：位置误差放大比例常数项（提高位置回复状态的反映，输入时根据不同的运动工作模式进行修改）
  UpperLimit：最大输出控制量上限
*/
static void ApplicationFunctionSet_MetalCarLinearMotionControl(OwlCarMotionControl direction, uint8_t directionRecord, uint8_t speed, uint8_t Kp, uint8_t UpperLimit)
{

  static float Yaw; //偏航
  static float yaw_So = 0;
  static uint8_t en = 0;
  //获取时间戳 timestamp
  static unsigned long is_time;
  if (en != directionRecord || millis() - is_time > 2)
  {
    // AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0, /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); //Motor control
    AppMPU6050getdata.DeviceDriverSet_MPU6050_dveGetEulerAngles(&Yaw);

    //Serial.println(Yaw);
    is_time = millis();
  }

  // if (en != directionRecord) //首次进入，记录基偏航角位置数据
  if (en != directionRecord || Application_FunctionSet.Car_LeaveTheGround == false)
  {
    en = directionRecord;
    yaw_So = Yaw;
  } //加入比例常数Kp  增大反弹作用
  //R...
  int R = (Yaw - yaw_So) * Kp + speed;
  if (R > UpperLimit)
  {
    R = UpperLimit;
  }
  else if (R < 10)
  {
    R = 10;
  }
  //L...
  int L = (yaw_So - Yaw) * Kp + speed;
  if (L > UpperLimit)
  {
    L = UpperLimit;
  }
  else if (L < 10)
  {
    L = 10;
  }
  // int R, L;
  // int te = (yaw_So - Yaw);
  // if (te > 0)
  // {
  //   L = 255;
  //   R = 0;
  // }
  // else if (te < 0)
  // {
  //   L = 0;
  //   R = 255;
  // }
  // else
  // {
  //   L = 255;
  //   R = 255;
  // }
  if (direction == Forward) //前进
  {
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ R,
                                           /*direction_B*/ direction_just, /*speed_B*/ L, /*controlED*/ control_enable);
  }
  else if (direction == Backward) //后退
  {
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ L,
                                           /*direction_B*/ direction_back, /*speed_B*/ R, /*controlED*/ control_enable);
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   static void ApplicationFunctionSet_OwlCarMotionControl(OwlCarMotionControl direction, uint8_t speed)
@ Functional function:  Owl Car 运动状态控制
@ Input parameters:     1# direction:前行（1）、后退（2）、 左前（3）、右前（4）、后左（5）、后右（6）、停止（6）
                        2# speed速度(0--255)
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层静态方法<调用硬件驱动层DeviceDriverSet_xxx0  Motor_control接口实现>
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
static void
ApplicationFunctionSet_OwlCarMotionControl(OwlCarMotionControl direction, uint8_t speed)
{
  ApplicationFunctionSet Application_FunctionSet;
  static uint8_t directionRecord = 0;
  uint8_t Kp, UpperLimit;
  //需要进行直线运动调整的控制模式（在以下工作运动模式小车前后方向运动时容易产生位置偏移，运动达不到相对直线方向的效果，因此需要加入控制调节）
  switch (Application_OwlCarxxx0.Functional_Mode)
  {
  case Rocker_mode:
    Kp = 10;
    UpperLimit = 255;
    break;
  case ObstacleAvoidance_mode:
    Kp = 2;
    UpperLimit = 255;
    //UpperLimit = 180;
    break;
  case CMD_CarControl_TimeLimit:
    Kp = 2;
    UpperLimit = 255;
    //UpperLimit = 180;
    break;
  case CMD_CarControl_NoTimeLimit:
    Kp = 2;
    UpperLimit = 255;
    //UpperLimit = 180;
    break;
  default:
    break;
  }
  switch (direction)
  {
  case /* constant-expression */
      Forward:
    /* code */
    //if (Application_OwlCarxxx0.Functional_Mode == TraceBased_mode)
    if (Application_OwlCarxxx0.Functional_Mode == TraceBased_mode || Application_OwlCarxxx0.Functional_Mode == CMD_TrajectoryControl)
    {
      AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                             /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    }
    else
    { //前进时进入方向位置逼近控制环处理
      // AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0,
      //                                        /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); //Motor control
      ApplicationFunctionSet_MetalCarLinearMotionControl(Forward, directionRecord, speed, Kp, UpperLimit);
      directionRecord = 1;
    }

    break;
  case /* constant-expression */ Backward:
    /* code */
    if (Application_OwlCarxxx0.Functional_Mode == TraceBased_mode || Application_OwlCarxxx0.Functional_Mode == CMD_TrajectoryControl)
    //if (Application_OwlCarxxx0.Functional_Mode == TraceBased_mode)
    {
      AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                             /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    }
    else
    { //后退时进入方向位置逼近控制环处理
      // AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0,
      //                                        /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); //Motor control
      ApplicationFunctionSet_MetalCarLinearMotionControl(Backward, directionRecord, speed, Kp, UpperLimit);
      directionRecord = 2;
    }

    break;
  case /* constant-expression */ Left:
    /* code */
    directionRecord = 3;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    break;
  case /* constant-expression */ Right:
    /* code */
    directionRecord = 4;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    break;
  case /* constant-expression */ LeftForward:
    /* code */
    directionRecord = 5;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed - 130, /*controlED*/ control_enable); //Motor control

    break;
  case /* constant-expression */ LeftBackward:
    /* code */
    directionRecord = 6;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed - 130, /*controlED*/ control_enable); //Motor control
    break;
  case /* constant-expression */ RightForward:
    /* code */
    directionRecord = 7;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ speed - 130,
                                           /*direction_B*/ direction_just, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    break;
  case /* constant-expression */ RightBackward:
    /* code */
    directionRecord = 8;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ speed - 130,
                                           /*direction_B*/ direction_back, /*speed_B*/ speed, /*controlED*/ control_enable); //Motor control
    break;
  case /* constant-expression */ stop_it:
    /* code */
    directionRecord = 9;
    AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ 0,
                                           /*direction_B*/ direction_void, /*speed_B*/ 0, /*controlED*/ control_enable); //Motor control
    break;
  default:
    directionRecord = 8;
    break;
  }
}
/*WaveFiltering:*/
static void WaveFiltering(uint8_t A_n, uint8_t *out)
{
  static uint8_t Record[5] = {0,0,0,0,0};
  static uint8_t Record_out = 0;

  uint8_t cout = 0;
  for (int i = 0; i < 4; i++) {
    if (abs((int)A_n - (int)Record[i]) < 5) cout++;
  }

  // shift left
  for (int i = 0; i < 4; i++) Record[i] = Record[i+1];
  Record[4] = A_n;

  if (cout >= 3) {            // 3/4 consistent
    *out = A_n;
    Record_out = A_n;
  } else {
    *out = Record_out;
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_SensorDataUpdate(void)
@ Functional function:  Owl Car 传感器数据更新
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用硬件驱动层DeviceDriverSet_xxx0>
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_SensorDataUpdate(void)
{
  static unsigned long us_time = 0;
  if (millis() - us_time >= 50)   // 20Hz update
  {
    us_time = millis();

    AppULTRASONIC.DeviceDriverSet_ULTRASONIC_Get(&UltrasoundData_mm);
    UltrasoundData_cm = UltrasoundData_mm / 10;

    UltrasoundDetectionStatus = function_xxx(UltrasoundData_cm, 0, ObstacleDetection);
  }

  // AppMotor.DeviceDriverSet_Motor_Test();
  { /*超声波数据更新：作用于避障*/
    AppULTRASONIC.DeviceDriverSet_ULTRASONIC_Get(&UltrasoundData_mm /*out*/);
    UltrasoundData_cm = UltrasoundData_mm / 10;
    UltrasoundDetectionStatus = function_xxx(UltrasoundData_cm, 0, ObstacleDetection); //避障状态
  }
  {
    /*STM8S003F3红外对管数据更新：作用于循迹*/
    ApplicationFunctionSet_OwlCarLeaveTheGround();
  }

  { /*电压状态更新*/
    static unsigned long VoltageData_time = 0;
    static int VoltageData_number = 1;
    if (millis() - VoltageData_time > 10) //10ms 采集并更新一次
    {
      VoltageData_time = millis();
      VoltageData_V = AppVoltage.DeviceDriverSet_Voltage_getAnalogue();
      //作用于大、小电池检测
      if ((VoltageData_V > 5.5 && VoltageData_V < 7.0) || (VoltageData_V > 2.0 && VoltageData_V < 3.7))
      // if (VoltageData_V < VoltageDetection)
      {
        VoltageData_number++;
        if (VoltageData_number == 500) //连续性多次判断最新的电压值...
        {
          VoltageDetectionStatus = true;
          VoltageData_number = 0;
        }
      }
      else
      {
        VoltageDetectionStatus = false;
      }
    }
  }
  //获取时间戳 timestamp
  static unsigned long logo_time;
  if (millis() - logo_time > 1000)
  {
    // float gyro, yaw;
    // AppMPU6050getdata.DeviceDriverSet_MPU6050_dveGetEulerAngles(&yaw);
    // Serial.print("\t");
    // Serial.print(yaw);
    // Serial.print("\t  ");
    // Serial.println(gyro);
    //AppSTM8S003F3_IR.DeviceDriverSet_STM8S003F3_IR_Test();
    logo_time = millis();
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Bootup(void)
@ Functional function:  Owl Car 开机动作设置
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用硬件驱动层<DeviceDriverSet_xxx0_H /TM1640.h>
  1# 表情面板显示
  2# 有源蜂鸣器播放  C调音阶（播发八个音名）
  3# Owl Car 进入空闲状态
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Bootup(void)
{
  Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[6] /*in*/);
  ApppassiveBuzzer.DeviceDriverSet_passiveBuzzer_Scale_c8(100 /*in:Duration*/);
  Application_OwlCarxxx0.Functional_Mode = Standby_mode;
  // MsTimer2::set(100, MsTimer2_updata);
  // MsTimer2::start();
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_RGB(void)
@ Functional function:  Owl Car RBG_LED 显示集合
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用硬件驱动层DeviceDriverSet_xxx0_H>
  1# Act on 低电压状态
  2# Act on 模式控制
  This is a set of RGB display logic control sets
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_RGB(void)
{
  static boolean ED = false;
  static unsigned long getAnalogue_time = 0;
  static uint8_t FastLED_clear;
  if (true == VoltageDetectionStatus) //1# Act on 低电压状态？
  {
    if (ED == false)
    {
      getAnalogue_time = millis();
      ED = true;
    }
    if ((millis() - getAnalogue_time) < 50)
    {
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Red);
    }
    else if ((millis() - getAnalogue_time) < 100)
    {
      AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
      getAnalogue_time = millis();
    }
  }
  else
  {
    ED = false;
    switch (Application_OwlCarxxx0.Functional_Mode) //2# Act on 模式控制
    {
    case /* constant-expression */ Standby_mode:
      /* code */
      {
        FastLED_clear = 0xff;
        if (VoltageDetectionStatus == true)
        {
          AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Red);
          delay(30);
          AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
          delay(30);
        }
        else
        { //作用于呼吸灯效果
          static uint8_t setBrightness = 0;
          static uint8_t colour = 0;
          static boolean et = false;
          static unsigned long time = 0;

          if ((millis() - time) > 10)
          {
            time = millis();
            if (et == false)
            {
              setBrightness += 1;
              if (setBrightness == 80)
                et = true;
            }
            else if (et == true)
            {
              setBrightness -= 1;
              if (setBrightness == 1)
              {
                et = false;
                colour = rand() % (8 - 0) + 0; //产生一个随机数<0--8>  z作用于颜色无规则循环颜色选择
              }
            }
          }
          if (colour == 0)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Brown;
          }
          else if (colour == 1)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Red;
          }
          else if (colour == 2)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Orange;
          }
          else if (colour == 3)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Yellow;
          }
          else if (colour == 4)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Green;
          }
          else if (colour == 5)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Blue;
          }
          else if (colour == 6)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Purple;
          }
          else if (colour == 7)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::Violet;
          }
          else if (colour == 8)
          {
            for (int i = 0; i < 5; i++)
              AppRBG_LED.leds[i] = CRGB::White;
          }

          FastLED.setBrightness(setBrightness);
          FastLED.show();
        }
      }
      break;
    case /* constant-expression */ CMD_Programming_mode:
      /* code */
      {
        FastLED_clear = 0xff;
      }
      break;
    case /* constant-expression */ TraceBased_mode:
      /* code */
      {
        FastLED_clear = 0xff;
        AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Green);
      }
      break;
    case /* constant-expression */ ObstacleAvoidance_mode:
      /* code */
      {
        FastLED_clear = 0xff;
        AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Yellow);
      }
      break;
    case /* constant-expression */ Rocker_mode:
      /* code */
      {
        if (FastLED_clear != Application_OwlCarxxx0.Motion_Control)
        {
          FastLED.clear(true);
          FastLED_clear = Application_OwlCarxxx0.Motion_Control;
        }
        switch (Application_OwlCarxxx0.Motion_Control)
        {
        case /* constant-expression */ Forward:
          /* code */
          {
            AppRBG_LED.leds[2] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ Backward:
          /* code */
          {
            AppRBG_LED.leds[0] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ Left:
          /* code */
          {
            AppRBG_LED.leds[3] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ Right:
          /* code */
          {
            AppRBG_LED.leds[1] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ LeftForward:
          /* code */
          {
            AppRBG_LED.leds[2] = CRGB::Blue;
            AppRBG_LED.leds[3] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ LeftBackward:
          /* code */
          {
            AppRBG_LED.leds[0] = CRGB::Blue;
            AppRBG_LED.leds[3] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ RightForward:
          /* code */
          {
            AppRBG_LED.leds[2] = CRGB::Blue;
            AppRBG_LED.leds[1] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ RightBackward:
          /* code */
          {
            AppRBG_LED.leds[1] = CRGB::Blue;
            AppRBG_LED.leds[0] = CRGB::Blue;
            FastLED.show();
          }
          break;
        case /* constant-expression */ stop_it:
          /* code */
          {
            FastLED_clear = 0xff;
            AppRBG_LED.leds[4] = CRGB::White;
            FastLED.show();
            delay(30);
            AppRBG_LED.leds[3] = CRGB::Yellow;
            FastLED.show();
            delay(30);
            AppRBG_LED.leds[2] = CRGB::Green;
            FastLED.show();
            delay(30);
            AppRBG_LED.leds[1] = CRGB::Red;
            FastLED.show();
            delay(30);
            AppRBG_LED.leds[0] = CRGB::Orange;
            FastLED.show();
            delay(30);
            AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
          }
          break;
        default:
          break;
        }
      }
      break;
    default:
      break;
    }
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Expression(void)
@ Functional function:  Owl Car 表情面板显示集合
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用硬件驱动层TM1640.h>
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Expression(void)
{
  switch (Application_OwlCarxxx0.Functional_Mode) //Act on 模式控制序列
  {
  case /* constant-expression */ Standby_mode: //空闲模式时面板将以 1000ms 的间隔循环显示表情logo集
    /* code */
    {
      static unsigned long Standby_mode_time = 0;
      static uint8_t Standby_mode_Panel = 0;
      if (millis() - Standby_mode_time > 1000)
      {
        Standby_mode_time = millis();
        Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[Standby_mode_Panel] /*in*/);
        Standby_mode_Panel += 1;
        if (Standby_mode_Panel > 9)
        {
          Standby_mode_Panel = 0;
        }
      }
    }
    break;
  case /* constant-expression */ CMD_ledExpressionControl:
    break;
  case /* constant-expression */ CMD_ledNumberControl:
    break;
  case /* constant-expression */ TraceBased_mode:
    /* code */
    {
      if (true == TrackingDetectionStatus_R && true == TrackingDetectionStatus_L)
      {
        Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[8] /*in*/);
      }
      else
      {
        Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[2] /*in*/);
      }
    }
    break;
  case /* constant-expression */ ObstacleAvoidance_mode:
    /* code */
    {
      if (true == UltrasoundDetectionStatus)
      {
        Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[0] /*in*/);
      }
      else
      {
        Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[4] /*in*/);
      }
    }
    break;
  case /* constant-expression */ Rocker_mode:
    /* code */
    {
      static unsigned long Rocker_mode_time = 0;
      static boolean Rocker_mode_Panel = false;
      if (millis() - Rocker_mode_time > 100)
      {
        Rocker_mode_time = millis();
        if (false == Rocker_mode_Panel)
        {
          Rocker_mode_Panel = true;
          Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[0] /*in*/);
        }
        else
        {
          Rocker_mode_Panel = false;
          Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[9] /*in*/);
        }
      }
      switch (Application_OwlCarxxx0.Motion_Control)
      {
      case /* constant-expression */ stop_it:
        /* code */
        {
          static unsigned long stop_it_time = 0;
          static uint8_t stop_it_Panel = 0;
          if (millis() - stop_it_time > 1000)
          {
            stop_it_time = millis();
            Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Model[stop_it_Panel] /*in*/);
            stop_it_Panel += 1;
            if (stop_it_Panel > 9)
              stop_it_Panel = 0;
          }
        }
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Rocker(void)
@ Functional function:  Owl Car 摇杆
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用ApplicationFunctionSet_OwlCarMotionControl()>
  命令N102:APP摇杆控制(控制命令接收及解析实现：ApplicationFunctionSet_SerialPortDataAnalysis())
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Rocker(void)
{

  if (Application_OwlCarxxx0.Functional_Mode == Rocker_mode)
  {
    ApplicationFunctionSet_OwlCarMotionControl(Application_OwlCarxxx0.Motion_Control /*direction*/, 255 /*speed*/);
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Exploration(void)
@ Functional function:  Owl Car 探索
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法<调用ApplicationFunctionSet_OwlCarMotionControl()>
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Exploration(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == Exploration_mode)
  {
    // --- turn-around timer ---
    static unsigned long lastTurnMs = 0;
    const unsigned long TURN_INTERVAL_MS = 999999999;   // every 5 seconds
    const unsigned long TURN_DURATION_MS = 900;    // how long to spin to "turn around" (tune this!)
    const uint8_t TURN_SPEED = 255;

    // Every 5 seconds, spin in place (turn-around)
    if (millis() - lastTurnMs >= TURN_INTERVAL_MS)
    {
      lastTurnMs = millis();

      // Spin in place (choose Left or Right)
      ApplicationFunctionSet_OwlCarMotionControl(Left, TURN_SPEED);
      delay(TURN_DURATION_MS);

      // Stop briefly (optional)
      ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      delay(100);
    }

    // --- existing exploration behavior ---
    float getAnaloguexxx_R = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R();
    float getAnaloguexxx_L = AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L();

    if ((getAnaloguexxx_R > TrackingDetection_CarLeaveTheGround) || (getAnaloguexxx_L > TrackingDetection_CarLeaveTheGround))
    {
      ApplicationFunctionSet_OwlCarMotionControl(Backward, 250);
      delay(500);

      ApplicationFunctionSet_OwlCarMotionControl(Left, 255);
      delay(800);
    }
    else
    {
      ApplicationFunctionSet_OwlCarMotionControl(Forward, 250);
    }
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Tracking(void)
@ Functional function:  Owl Car 循迹
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法
  模式进入：ApplicationFunctionSet_KeyCommand():1 / ApplicationFunctionSet_SerialPortDataAnalysis()：N 101
  1#可通过串口命令及模式按键切换进入当前工作模式，即：循迹
  2#获取两个模拟量的红外光电传感器数据（循迹模块:0--1024）
  3#设定阈值（线上：200 -- 860）
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 循迹功能编码表：2E3=8（三个传感器组合编程8个状态）
 --------------------------------|
  L/   0  1  0  1  0  1  0  1    |
 --------------------------------|
  M/   0  0  1  1  0  0  1  1    |
 --------------------------------|
  R/   0  0  0  0  1  1  1  1    |
 --------------------------------|
       0  1  2  3  4  5  6  7    |
 --------------------------------|
 前进：2、7     /    左转：1、3     /     右转：4、6      /      盲扫：0
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Tracking(void)
{
  static boolean timestamp = true;
  static boolean BlindDetection = true;
  static unsigned long MotorRL_time = 0;
  static uint8_t DirectionRecording = 0;

  if (Application_OwlCarxxx0.Functional_Mode == TraceBased_mode)
  {
    if (Car_LeaveTheGround == false)
    {
      ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      return;
    }

    AppSTM8S003F3_IR.DeviceDriverSet_STM8S003F3_IR_Get(&TrackingData_L /*uint16_t * STM8S003F3_IRL out*/,
                                                       &TrackingData_M /*uint16_t * STM8S003F3_IRM out*/,
                                                       &TrackingData_R /*uint16_t * STM8S003F3_IRR out*/);

#if _Test_print
    static unsigned long print_time = 0;
    if (millis() - print_time > 500)
    {
      print_time = millis();
      Serial.print("TrackingData_L=");
      Serial.println(TrackingData_L);
      Serial.print("TrackingData_M=");
      Serial.println(TrackingData_M);
      Serial.print("TrackingData_R=");
      Serial.println(TrackingData_R);
    }
#endif

    if (1 == function_xxx(TrackingData_M, TrackingDetection_S, TrackingDetection_E)) //前进 2==010
    {
      timestamp = true;
      BlindDetection = true;
      /*控制左右电机转动：实现匀速直行*/
      ApplicationFunctionSet_OwlCarMotionControl(Forward, 255);
      DirectionRecording = 0;
    }
    else if (1 == function_xxx(TrackingData_L, TrackingDetection_S, TrackingDetection_E) &&
             // 0 == function_xxx(TrackingData_M, TrackingDetection_S, TrackingDetection_E) &&
             0 == function_xxx(TrackingData_R, TrackingDetection_S, TrackingDetection_E)) //左转 1==001 || 3==011  可以不关心 M
    {
      timestamp = true;
      BlindDetection = true;
      /*控制左右电机转动：前左*/
      ApplicationFunctionSet_OwlCarMotionControl(Left, 255);
      DirectionRecording = 1;
    }
    else if (0 == function_xxx(TrackingData_L, TrackingDetection_S, TrackingDetection_E) &&
             // 0 == function_xxx(TrackingData_M, TrackingDetection_S, TrackingDetection_E) &&
             1 == function_xxx(TrackingData_R, TrackingDetection_S, TrackingDetection_E)) //右转 4==100 || 6==110  可以不关心 M
    {
      timestamp = true;
      BlindDetection = true;
      /*控制左右电机转动：前右*/
      ApplicationFunctionSet_OwlCarMotionControl(Right, 255);
      DirectionRecording = 2;
    }
    else //不在黑线上的时候。。。执行盲循迹   0==0000
    {

      ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      //获取时间戳 timestamp
      if (timestamp == true)
      {
        timestamp = false;
        MotorRL_time = millis();
        ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      }
      /*Blind Detection*/
      if ((function_xxx((millis() - MotorRL_time), 0, 200) || function_xxx((millis() - MotorRL_time), 600, 1000) || function_xxx((millis() - MotorRL_time), 1500, 3600)) && BlindDetection == true)
      {
        if (1 == DirectionRecording)
        {
          ApplicationFunctionSet_OwlCarMotionControl(Right, 255);
        }
        else
        {
          ApplicationFunctionSet_OwlCarMotionControl(Left, 255);
        }
      }
      else if (((function_xxx((millis() - MotorRL_time), 200, 600)) || function_xxx((millis() - MotorRL_time), 1000, 1400)) && BlindDetection == true)
      {
        if (1 == DirectionRecording)
        {
          ApplicationFunctionSet_OwlCarMotionControl(Left, 255);
        }
        else
        {
          ApplicationFunctionSet_OwlCarMotionControl(Right, 255);
        }
      }
      else if (((function_xxx((millis() - MotorRL_time), 1400, 1500))) && BlindDetection == true)
      {
        ApplicationFunctionSet_OwlCarMotionControl(Forward, 255);
      }
      else if ((function_xxx((millis() - MotorRL_time), 5000, 5500))) // Blind Detection ...s ?
      {
        BlindDetection == false;
        ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      }
    }
  }
  else
  {
    timestamp = true;
    BlindDetection = true;
    MotorRL_time = 0;
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_Obstacle(void)
@ Functional function:  Owl Car 避障
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法
  模式进入：ApplicationFunctionSet_KeyCommand():2 / ApplicationFunctionSet_SerialPortDataAnalysis()：N 101
  1#可通过串口命令及模式按键切换进入当前工作模式---避障
  2#获取超声波测距传感器数据
  3#设定阈值（检测范围：）
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_Obstacle(void)
{
  static boolean timestamp = true;
  if (Application_OwlCarxxx0.Functional_Mode == ObstacleAvoidance_mode)
  {
    if (Car_LeaveTheGround == false) //车子被拿起来了？
    {
      ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
      return;
    }
    if (function_xxx(UltrasoundData_cm, 0, ObstacleDetection)) //前方cm内有障碍物？(避障数据为超声波检测数据：后台周期性更新)
    {
      //获取时间戳 timestamp
      static unsigned long MotorRL_time;
      if (timestamp == true || millis() - MotorRL_time > 7000)
      {
        timestamp = false;
        MotorRL_time = millis();
      }

      if (function_xxx((millis() - MotorRL_time), 0, 1500) || function_xxx((millis() - MotorRL_time), 4500, 6000))
      {
        /*控制左右电机转动：左*/
        ApplicationFunctionSet_OwlCarMotionControl(Left, 255); //履带
      }
      else if (function_xxx((millis() - MotorRL_time), 1500, 4500))
      {
        /*控制左右电机转动：右*/
        ApplicationFunctionSet_OwlCarMotionControl(Right, 255); //履带
      }
      else if (function_xxx((millis() - MotorRL_time), 6000, 7000))
      {
        /*控制左右电机转动：后退*/
        ApplicationFunctionSet_OwlCarMotionControl(Backward, 250);
      }
    }
    else
    {
      timestamp = true;
      /*控制左右电机转动：实现匀速直行*/
      ApplicationFunctionSet_OwlCarMotionControl(Forward, 255);
    }
  }
  else
  {
    timestamp = true;
  }
}
/*待机*/
void ApplicationFunctionSet::ApplicationFunctionSet_Standby(void)
{
  static bool is_ED = true;
  static uint8_t cout = 0;
  if (Application_OwlCarxxx0.Functional_Mode == Standby_mode)
  {
    ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    if (true == is_ED) //作用于偏航原始数据调零(确认小车放置在静止平面！)
    {
      static unsigned long timestamp; //获取时间戳 timestamp
      if (millis() - timestamp > 20)
      {
        timestamp = millis();
        if (true == Car_LeaveTheGround /* condition */)
        {
          cout += 1;
        }
        else
        {
          cout = 0;
        }
        if (cout > 50)
        {
          is_ED = false;
          AppMPU6050getdata.DeviceDriverSet_MPU6050_Init();
        }
      }
    }
  }
}

/*Begin:CMD==============================================================================================================================================================================================================================

*/

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_MotorControl_xxx0(uint8_t is_MotorSelection, uint8_t is_MotorDirection, uint8_t is_MotorSpeed)
@ Functional function:  Owl Car 电机控制
@ Input parameters:     uint8_t CMD_is_MotorSelection,  电机选择  1左  2右  0全部
                        uint8_t CMD_is_MotorDirection,  电机转向  1正  2反  0停止
                        uint8_t CMD_is_MotorSpeed,      电机速度  0-250
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N1指令运动模式 <电机控制> 接收并根据 APP端控制命令,执行对电机的单方向驱动(无时间限定：电机在运行时不接收时间的限定)
    *面向<电机>控制为对象
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_MotorControl_xxx0(void)
{
  static boolean MotorControl = false;
  static uint8_t is_MotorSpeed_A = 0;
  static uint8_t is_MotorSpeed_B = 0;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_MotorControl)
  {
    MotorControl = true;
    // if (0 == CMD_is_MotorDirection)
    // {
    //   ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    //   return;
    // }
    // else
    {
      switch (CMD_is_MotorSelection) //电机选择
      {
      case 0:
      {
        is_MotorSpeed_A = CMD_is_MotorSpeed;
        is_MotorSpeed_B = CMD_is_MotorSpeed;
        if (1 == CMD_is_MotorDirection)
        { //正转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_just, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else if (2 == CMD_is_MotorDirection)
        { //反转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_back, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else
        {
          return;
        }
      }
      break;
      case 1:
      {
        is_MotorSpeed_A = CMD_is_MotorSpeed;
        if (1 == CMD_is_MotorDirection)
        { //正转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_just, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else if (2 == CMD_is_MotorDirection)
        { //反转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_void, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_back, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else
        {
          return;
        }
      }
      break;
      case 2:
      {
        is_MotorSpeed_B = CMD_is_MotorSpeed;
        if (1 == CMD_is_MotorDirection)
        { //正转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_void, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else if (2 == CMD_is_MotorDirection)
        { //反转
          AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_back, /*speed_A*/ is_MotorSpeed_A,
                                                 /*direction_B*/ direction_void, /*speed_B*/ is_MotorSpeed_B,
                                                 /*controlED*/ control_enable); //Motor control
        }
        else
        {
          return;
        }
      }
      break;
      default:
        break;
      }
    }
  }
  else
  {
    if (MotorControl == true)
    {
      MotorControl = false;
      is_MotorSpeed_A = 0;
      is_MotorSpeed_B = 0;
    }
  }
}

static void CMD_CarControl(uint8_t is_CarDirection, uint8_t is_CarSpeed)
{
  switch (is_CarDirection)
  {
  case 1: /*运动模式 左*/
    ApplicationFunctionSet_OwlCarMotionControl(Left, is_CarSpeed);
    break;
  case 2: /*运动模式 右*/
    ApplicationFunctionSet_OwlCarMotionControl(Right, is_CarSpeed);
    break;
  case 3: /*运动模式 前进*/
    ApplicationFunctionSet_OwlCarMotionControl(Forward, is_CarSpeed);
    break;
  case 4: /*运动模式 后退*/
    ApplicationFunctionSet_OwlCarMotionControl(Backward, is_CarSpeed);
    break;
  default:
    break;
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_CarControlTimeLimit_xxx0(void)
@ Functional function:  Owl Car 车子控制
@ Input parameters:     uint8_t CMD_is_CarDirection,  1（左转）、2（右转）、3（前进）、4（后退）
                        uint8_t CMD_is_CarSpeed,      电机速度  0-250
                        uint32_t CMD_is_Timer         执行时长
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N2指令运动模式 <车子控制> 接收并根据 APP端控制命令,执行对车子的运动控制(有时间限定：电机在运行时接收时间限定)
      *面向<车子控制>为对象
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_CarControlTimeLimit_xxx0(void)
{
  // Application_FunctionSet.CMD_is_CarDirection, Application_FunctionSet.CMD_is_CarSpeed, Application_FunctionSet.CMD_is_CarTimer
  static boolean CarControl = false;
  static boolean CarControl_TE = false; //还有时间标志
  static boolean CarControl_return = false;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_CarControl_TimeLimit) //进入车子有时间限定控制模式
  {
    CarControl = true;
    if (CMD_is_CarTimer != 0) //#1设定时间不为..时 (空)
    {
      if ((millis() - Application_OwlCarxxx0.CMD_CarControl_Millis) > (CMD_is_CarTimer)) //判断时间戳
      {
        CarControl_TE = true;
        ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);

        Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
        if (CarControl_return == false)
        {

#if _is_print
          Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
          CarControl_return = true;
        }
      }
      else
      {
        CarControl_TE = false; //还有时间
        CarControl_return = false;
      }
    }
    //if (CarControl_TE == false && Car_LeaveTheGround == true)
    if (CarControl_TE == false)
    {
      CMD_CarControl(CMD_is_CarDirection, CMD_is_CarSpeed);
    }
  }
  else
  {
    if (CarControl == true)
    {
      CarControl_return = false;
      CarControl = false;
      Application_OwlCarxxx0.CMD_CarControl_Millis = 0;
    }
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:  void ApplicationFunctionSet::CMD_CarControlNoTimeLimit_xxx0(void)
@ Functional function:  Owl Car 车子控制
@ Input parameters:     uint8_t CMD_is_CarDirection,  1（左转）、2（右转）、3（前进）、4（后退）
                        uint8_t CMD_is_CarSpeed,      电机速度  0-250
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N3指令运动模式 <车子控制> 接收并根据 APP端控制命令,执行对车子的运动控制(无时间限定：电机在运行时不接收时间限定)
      *面向<车子控制>为对象
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_CarControlNoTimeLimit_xxx0(void)
{

  static boolean CarControl = false;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_CarControl_NoTimeLimit) //进入小车无时间限定控制模式
  {
    CarControl = true;
    // if (Car_LeaveTheGround == true)
    {
      CMD_CarControl(CMD_is_CarDirection, CMD_is_CarSpeed);
    }
  }
  else
  {
    if (CarControl == true)
    {
      CarControl = false;
    }
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:  void ApplicationFunctionSet::CMD_MotorControlSpeed_xxx0(void)
@ Functional function:  Owl Car 电机控制
@ Input parameters:    uint8_t CMD_is_Speed_L(左电机的速度), uint8_t CMD_is_Speed_R(右电机的速度)
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N4指令运动模式 <电机控制> 接收并根据 APP端控制命令,执行对电机的运动控制(无时间限定：电机在运行时不接收时间限定)
      *面向<电机控制>为对象
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_MotorControlSpeed_xxx0(void)
{
  static boolean MotorControl = false;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_MotorControl_Speed)
  {
    MotorControl = true;

    // if (0 == CMD_is_MotorDirection)
    // {
    //   ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    //   return;
    // }
    // else
    {
      AppMotor.DeviceDriverSet_Motor_control(/*direction_A*/ direction_just, /*speed_A*/ CMD_is_MotorSpeed_R,
                                             /*direction_B*/ direction_just, /*speed_B*/ CMD_is_MotorSpeed_L,
                                             /*controlED*/ control_enable); //Motor control
    }
  }
  else
  {
    if (MotorControl == true)
    {
      MotorControl = false;
    }
  }
}

/*
  N5:指令
  CMD模式：<舵机控制>
  待升级：暂时不做处理
*/
void ApplicationFunctionSet::CMD_ServoControl_xxx0(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == CMD_ServoControl)
  {
    //Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:  void ApplicationFunctionSet::CMD_VoiceControl_xxx0(void)
@ Functional function:  Owl Car 声音控制
@ Input parameters:   is_controlAudio, 音频信号量
                      is_VoiceTimer    执行时长
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N6指令声音模式 <声音控制> 接收并根据 APP端控制命令,执行对声音模块控制(有时间限定：声音在运行时接收时间限定)
    0#：时间结束后进入编程模式
      *面向<声音控制>为对象
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_VoiceControl_xxx0(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == CMD_VoiceControl)
  {
    ApppassiveBuzzer.DeviceDriverSet_passiveBuzzer_controlAudio(CMD_is_controlAudio, CMD_is_VoiceTimer);
    Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
#if _is_print
    Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
  }
}

static void CMD_Lighting(uint8_t is_LightingSequence, int8_t is_LightingColorValue_R, uint8_t is_LightingColorValue_G, uint8_t is_LightingColorValue_B)
{
  switch (is_LightingSequence)
  {
  case 0:
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(NUM_LEDS, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  case 1: /*左*/
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(3, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  case 2: /*前*/
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(2, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  case 3: /*右*/
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(1, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  case 4: /*后*/
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(0, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  case 5: /*中*/
    AppRBG_LED.DeviceDriverSet_RBGLED_Color(4, is_LightingColorValue_R, is_LightingColorValue_G, is_LightingColorValue_B);
    break;
  default:
    break;
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_LightingControlTimeLimit_xxx0(void) 
@ Functional function:  Owl Car RGB 显示
@ Input parameters:     CMD_is_LightingSequence (Left, front, right, back and center)
                        CMD_is_LightingColorValue_R,
                        CMD_is_LightingColorValue_G,
                        CMD_is_LightingColorValue_B,
                        CMD_is_LightingTimer,
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N7:指令
    0# RGB Lighting Control with Time Limitation
    1# 进入编程模式
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_LightingControlTimeLimit_xxx0(void)
{
  static boolean LightingControl = false;
  static boolean LightingControl_TE = false; //还有时间标志
  static boolean LightingControl_return = false;

  if (Application_OwlCarxxx0.Functional_Mode == CMD_LightingControl_TimeLimit) //进入灯光有时间限定控制模式
  {
    LightingControl = true;
    if (CMD_is_LightingTimer != 0) //#1设定时间不为..时 (空)
    {
      if ((millis() - Application_OwlCarxxx0.CMD_LightingControl_Millis) > (CMD_is_LightingTimer)) //判断时间戳
      {
        LightingControl_TE = true;
        FastLED.clear(true);
        Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
        if (LightingControl_return == false)
        {

#if _is_print
          Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
          LightingControl_return = true;
        }
      }
      else
      {
        LightingControl_TE = false; //还有时间
        LightingControl_return = false;
      }
    }
    if (LightingControl_TE == false)
    {
      CMD_Lighting(CMD_is_LightingSequence, CMD_is_LightingColorValue_R, CMD_is_LightingColorValue_G, CMD_is_LightingColorValue_B);
    }
  }
  else
  {
    if (LightingControl == true)
    {
      LightingControl_return = false;
      LightingControl = false;
      Application_OwlCarxxx0.CMD_LightingControl_Millis = 0;
    }
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_LightingControlNoTimeLimit_xxx0(void)
@ Functional function:  Owl Car RGB 显示
@ Input parameters:     CMD_is_LightingSequence (Left, front, right, back and center)
                        CMD_is_LightingColorValue_R,
                        CMD_is_LightingColorValue_G,
                        CMD_is_LightingColorValue_B
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N8:指令
    0# RGB Lighting Control without Time Limit
    1# 进入编程模式
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_LightingControlNoTimeLimit_xxx0(void)
{
  static boolean LightingControl = false;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_LightingControl_NoTimeLimit) //进入灯光无时间限定控制模式
  {
    LightingControl = true;
    CMD_Lighting(CMD_is_LightingSequence, CMD_is_LightingColorValue_R, CMD_is_LightingColorValue_G, CMD_is_LightingColorValue_B);
  }
  else
  {
    if (LightingControl == true)
    {
      LightingControl = false;
    }
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_LEDCustomExpressionControl_xxx0(void)
@ Functional function:  Owl Car LED 8*16矩阵板 表情显示
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N9:指令
    0# LED matrix display expression
    1# 进入编程模式
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_LEDCustomExpressionControl_xxx0(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == CMD_ledExpressionControl)
  {
    Apptm1640.TM1640_FullScreenDisaple_led16x8(CMD_is_LEDCustomExpression_arry /*in*/);
    Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
  }
}
/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_LEDNumberDisplayControl_xxx0(void)
@ Functional function:  Owl Car LED 8*16矩阵板 数字显示
@ Input parameters:     CMD_is_LEDNumber （0--9）
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N10:指令
    0# LED matrix digital display
    1# 进入编程模式
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_LEDNumberDisplayControl_xxx0(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == CMD_ledNumberControl)
  {
    if (CMD_is_LEDNumber < 0)
    {
      CMD_is_LEDNumber = 0;
    }
    if (CMD_is_LEDNumber > 9)
    {
      CMD_is_LEDNumber = 9;
    }
    Apptm1640.TM1640_FullScreenDisaple_led16x8(Apptm1640.Display_Number[CMD_is_LEDNumber] /*in*/);
    Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_ClearAllFunctions_xxx0(void)
@ Functional function:  Owl Car 清除功能
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N100/N110:指令
    0# N100 清除所有功能进入空闲模式
    1# N110 清除所有功能进入编程模式
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_ClearAllFunctions_xxx0(void)
{
  if (Application_OwlCarxxx0.Functional_Mode == CMD_ClearAllFunctions_Standby_mode) // 0# N100 清除所有功能进入空闲模式
  {
    ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    FastLED.clear(true);
    AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
    Application_OwlCarxxx0.Motion_Control = stop_it;
    Application_OwlCarxxx0.Functional_Mode = Standby_mode;
  }
  if (Application_OwlCarxxx0.Functional_Mode == CMD_ClearAllFunctions_Programming_mode) //1# N110 清除所有功能进入编程模式
  {
    ApplicationFunctionSet_OwlCarMotionControl(stop_it, 0);
    FastLED.clear(true);
    AppRBG_LED.DeviceDriverSet_RBGLED_xxx(0 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
    Application_OwlCarxxx0.Motion_Control = stop_it;
    Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode;
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_UltrasoundModuleStatus_xxx0(uint8_t is_get)
@ Functional function:  Owl Car 向串口控制对象反馈循迹状态及数据
@ Input parameters:     is_get （0/1, 状态/数据）
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N21:指令
    0# 状态/数据 选择
    1# 数据反馈
    2# 切换到编程模式
    The ultrasonic ranging and obstacle avoidance module receives and feeds back the ultrasonic obstacle avoidance status and ranging data according to the control command of APP terminal.
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_UltrasoundModuleStatus_xxx0(uint8_t is_get)
{
  if (1 == is_get) //超声波  is_get Start     true：有障碍物 / false:无障碍物
  {
    if (true == UltrasoundDetectionStatus)
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_true}");
#endif
    }
    else
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_false}");
#endif
    }
  }
  else if (2 == is_get) //超声波 is_get data
  {

    char toString[10];
    sprintf(toString, "%d", UltrasoundData_cm);
    String str = "";
#if _is_print

    Serial.print('{' + CommandSerialNumber + '_' + toString + '}');
#endif
  }
  //2#切换到编程模式
  Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_TraceModuleStatus_xxx0(uint8_t is_get)
@ Functional function:  Owl Car 向串口控制对象反馈循迹状态及数据
@ Input parameters:     is_get （0/1, L/R）
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N22:指令
    0# L/R 选择
    1# 数据反馈
    2# 切换到编程模式
    Tracking module receives and feeds back tracing status and data according to the control command of APP terminal
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_TraceModuleStatus_xxx0(uint8_t is_get)
{
  //0# L  选择
  if (0 == is_get) /*循迹状态获取左边*/
  {
    // 1# 数据反馈
    if (true == TrackingDetectionStatus_L)
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_true}");
#endif
    }
    else
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_false}");
#endif
    }
  }
  //0# M  选择
  else if (1 == is_get) /*循迹状态获取中间*/
  {
    // 1# 数据反馈
    if (true == TrackingDetectionStatus_M)
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_true}");
#endif
    }
    else
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_false}");
#endif
    }
  }
  //0# R  选择
  else if (2 == is_get) /*循迹状态获取右边*/
  {
    // 1# 数据反馈
    if (true == TrackingDetectionStatus_R)
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_true}");
#endif
    }
    else
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_false}");
#endif
    }
  }
  //0# xxx  选择
  else if (3 == is_get) /*循迹状态获取右边*/
  {
    // 1# 数据反馈
    if (true == TrackingDetectionStatus_R || true == TrackingDetectionStatus_M || true == TrackingDetectionStatus_L)
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_true}");
#endif
    }
    else
    {
#if _is_print
      Serial.print('{' + CommandSerialNumber + "_false}");
#endif
    }
  }
  //2#切换到编程模式
  Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode; /*进入编程模式提示符<等待下一组控制命令的到来>*/
}

/*
  数学公式：获取任意坐标值的相对角度
*/
static void getRngle(float x /*in*/, float y /*in*/, float *getRngle /*out*/)
{
  float Rngle = atan2(y, x) / (2 * acos(-1)) * 360;
  if (Rngle < 0)
    Rngle += 360;
  *getRngle = Rngle;
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::CMD_TrajectoryControl_xxx0(void)
@ Functional function:  Owl Car 画线轨迹行径运动控制
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    CMD模式：N103:指令
    0# 获取运动坐标值的相对角度
    1# 计算平面位置角度偏移误差值
    2# 确定前后运动坐标的偏移方向
    3# 直行运动
    4# 切换到编程模式
    5# 等待下一次运动坐标值更新
    This part of the program is to deal with APP and PC command control
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::CMD_TrajectoryControl_xxx0(void)
{
  static boolean NextCtrlCycleUp_flag = false;
  static float Trajectory_Record_X = 0; //基坐标（虚拟圆点）
  static float Trajectory_Record_Y = 0;
  if (Application_OwlCarxxx0.Functional_Mode == CMD_TrajectoryControl)
  {
    if (NextCtrlCycleUp_flag == false) //首次坐标...记录
    {
      Trajectory_Record_X = CMD_is_TrajectoryControl_axisPlaneData_X; //基坐标（更新虚拟圆点）
      Trajectory_Record_Y = CMD_is_TrajectoryControl_axisPlaneData_Y;
    }
    //获取最新指定的坐标位置值
    float New_Record_X = (CMD_is_TrajectoryControl_axisPlaneData_X); //运动坐标
    float New_Record_Y = (CMD_is_TrajectoryControl_axisPlaneData_Y);
    float Position_angle_E0;        //本次角度
    static float Position_angle_E1; //历史前一次角度
    float Deviation_angle;          //偏差角
    int Motion_angle;               //运动角
    int DirectionControl = 0;       //方向控制选标志
    //0# 获取运动坐标值的相对角度
    getRngle(New_Record_X - Trajectory_Record_X /*in*/, New_Record_Y - Trajectory_Record_Y /*in*/, &Position_angle_E0 /*out*/);
    //1 #计算平面位置角度偏移误差值
    Deviation_angle = Position_angle_E0 - Position_angle_E1; //偏差角<（P-0） - （p-1）:本次 减 上次>
    //方向以 Position_angle_E1 为中心点平分左右运动控制范围 ±180°
    float LeftAngle = Position_angle_E1 + 180;
    float RightAngle = Position_angle_E1 - 180;

    if (NextCtrlCycleUp_flag == true)
    {
      //2 #确定前后运动坐标的偏移方向
      if (LeftAngle <= 360) //Position_angle_E1° --------（+360°）逆时针：左边角度在控制范围内？<<<----
      {
        if (Position_angle_E0 > Position_angle_E1 /*本次的角度值要比上次的大*/ && Position_angle_E0 < LeftAngle /*本次的角度值要比可控范围小*/)
        { //左前方运动
          DirectionControl = 1;
          Motion_angle = Deviation_angle;
        }
        else
        { //右前方运动
          if (Deviation_angle > 0)
          {
            DirectionControl = 2;
            int a = (360 - Position_angle_E0 + Position_angle_E1);
            Motion_angle = a;
          }
          else
          {
            DirectionControl = 2;
            Motion_angle = Deviation_angle;
          }
        }
      }
      else if (RightAngle >= 0) //Position_angle_E1° --------（+0°）顺时针：右边角度在控制范围内？---->>>
      {
        if (Position_angle_E0 < Position_angle_E1 && Position_angle_E0 > RightAngle)
        { //右前方运动
          DirectionControl = 2;
          Motion_angle = Deviation_angle;
        }
        else
        { //左前方运动
          if (Deviation_angle < 0)
          {
            DirectionControl = 1;
            int a = (360 - Position_angle_E1 + Position_angle_E0);
            Motion_angle = a;
          }
          else
          {
            DirectionControl = 1;
            Motion_angle = Deviation_angle;
          }
        }
      }
      Motion_angle = abs(Motion_angle);
      uint16_t Motion_angle_time = (750.0 / 90.0) * Motion_angle; //通过控制电机运动时间来改变偏移的位置角度
      // float yaw_So; float _yaw;
      // AppMPU6050getdata.DeviceDriverSet_MPU6050_dveGetEulerAngles(&yaw_So);
      if (2 == DirectionControl) //前左方向运动控制
      {
        // do
        // {
        //   wdt_reset();
        //   AppMPU6050getdata.DeviceDriverSet_MPU6050_dveGetEulerAngles(&_yaw);
        //   ApplicationFunctionSet_OwlCarMotionControl(Left /*in:direction*/, 255 /*in:0 < speed > 250*/);
        // } while ((yaw_So - Motion_angle) > _yaw);
        ApplicationFunctionSet_OwlCarMotionControl(Left /*in:direction*/, 255 /*in:0 < speed > 250*/);
        delay(Motion_angle_time);
      }
      else if (1 == DirectionControl) //前右方向运动控制
      {
        // do
        // {
        //   wdt_reset();
        //   AppMPU6050getdata.DeviceDriverSet_MPU6050_dveGetEulerAngles(&_yaw);
        //   ApplicationFunctionSet_OwlCarMotionControl(Right /*in:direction*/, 255 /*in:0 < speed > 250*/);
        // } while ((yaw_So + Motion_angle) > _yaw);
        ApplicationFunctionSet_OwlCarMotionControl(Right /*in:direction*/, 255 /*in:0 < speed > 250*/);
        delay(Motion_angle_time);
      }
      //3# 直行运动
      ApplicationFunctionSet_OwlCarMotionControl(Forward /*in:direction*/, 255 /*in:0 < speed > 250*/);
    }
    //4# 切换到编程模式
    Application_OwlCarxxx0.Functional_Mode = CMD_Programming_mode;

    Trajectory_Record_X = New_Record_X; //记录最新计算坐标值，作用于下个控制点
    Trajectory_Record_Y = New_Record_Y;
    Position_angle_E1 = Position_angle_E0; //记录本次角度值，作用于下个控制点
    NextCtrlCycleUp_flag = true;
  }
  else
  {
    if (NextCtrlCycleUp_flag == true)
    {
      if (Application_OwlCarxxx0.Functional_Mode == CMD_ClearAllFunctions_Standby_mode) //清除所有功能：进入空闲模式    N100:指令
      {
        NextCtrlCycleUp_flag = false;
        Trajectory_Record_X = 0; //基坐标（虚拟圆点）
        Trajectory_Record_Y = 0;
      }
    }
  }
}
/*End:CMD==============================================================================================================================================================================================================================
*/

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_KeyCommand(void)
@ Functional function:  Owl Car 模式按键值获取
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void ApplicationFunctionSet::ApplicationFunctionSet_KeyCommand(void)
{
  uint8_t get_keyValue;
  static uint8_t temp_keyValue = keyValue_Max;
  AppKey.DeviceDriverSet_key_Get(&get_keyValue);

  if (temp_keyValue != get_keyValue)
  {

    temp_keyValue = get_keyValue;
    switch (get_keyValue)
    {
    case /* constant-expression */ 1:
      /* code */
      Application_OwlCarxxx0.Functional_Mode = Standby_mode;
      break;
    case /* constant-expression */ 2:
      /* code */
      Application_OwlCarxxx0.Functional_Mode = TraceBased_mode;
      break;
    case /* constant-expression */ 3:
      /* code */
      Application_OwlCarxxx0.Functional_Mode = ObstacleAvoidance_mode;
      break;
    default:

      break;
    }
  }
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void ApplicationFunctionSet::ApplicationFunctionSet_SerialPortDataAnalysis(void)
@ Functional function:  Owl Car 串口数据解析
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          此为ApplicationFunctionSet层公共方法 <调用硬件驱动层: DeviceDriverSet_xxx0_H >
    1#接收串口数据流
    2#导入JsonDocument
    3#解析并更新控制命令的信号量值
$ Elegoo & OwlCar Tank Kit & 2019-09
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
StaticJsonDocument<128> doc; //声明一个JsonDocument对象
void ApplicationFunctionSet::ApplicationFunctionSet_SerialPortDataAnalysis(void)
{
  uint8_t Rocker_temp;
  static String SerialPortData = "";
  uint8_t c = "";
  //1 #接收串口数据流
  if (Serial.available() > 0)
  {
    while (c != '}' && Serial.available() > 0)
    {
      // while (Serial.available() == 0)//强行等待一帧数据完成接收
      //   ;
      c = Serial.read();
      SerialPortData += (char)c;
    }
  }
  if (c == '}')
  {
#if _Test_print
    Serial.println(SerialPortData);
#endif

    // if (true == SerialPortData.equals("{get_v}"))
    // {
    //   Serial.print("get_v:");
    //   Serial.println(VoltageData_V);
    //   return;
    // }
    // if (true == SerialPortData.equals("{version}"))
    // {
    //   Serial.println("Elegoo & OwlCar Tank Kit & 2019-09");
    //   SerialPortData = "";
    //   return;
    // }
    //2#导入JsonDocument
    StaticJsonDocument<200> doc;                                       //声明一个JsonDocument对象
    DeserializationError error = deserializeJson(doc, SerialPortData); //反序列化JSON数据
    SerialPortData = "";
    if (!error) //检查反序列化是否成功
    {
      uint8_t control_mode_N = doc["N"];
      char buf[3];
      uint8_t temp = doc["H"];
      sprintf(buf, "%d", temp);
      CommandSerialNumber = buf; //获取新命令的序号

      //3#解析并更新控制命令的信号量值
      switch (control_mode_N) /*以下代码块请结合小车通讯协议V.docx 文档查看*/
      {
      case 1: /*<命令：N 1> 电机控制模式 */
        Application_OwlCarxxx0.Functional_Mode = CMD_MotorControl;
        CMD_is_MotorSelection = doc["D1"];
        CMD_is_MotorDirection = 1; //默认电机正方向转动
        CMD_is_MotorSpeed = doc["D2"];
        MotorSpeedAdjustment(&CMD_is_MotorSpeed);

#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 2:                                                              /*<命令：N 2> */
        Application_OwlCarxxx0.Functional_Mode = CMD_CarControl_TimeLimit; /*小车方向控制：有时间限定模式*/
        CMD_is_CarDirection = doc["D1"];
        CMD_is_CarSpeed = doc["D2"];
        CMD_is_CarTimer = doc["T"];
        MotorSpeedAdjustment(&CMD_is_CarSpeed);
        Application_OwlCarxxx0.CMD_CarControl_Millis = millis();
#if _is_print
        //Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 3:                                                                /*<命令：N 3> */
        Application_OwlCarxxx0.Functional_Mode = CMD_CarControl_NoTimeLimit; /*小车方向控制：无时间限定模式*/
        CMD_is_CarDirection = doc["D1"];
        CMD_is_CarSpeed = doc["D2"];
        MotorSpeedAdjustment(&CMD_is_CarSpeed);
#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 4:                                                            /*<命令：N 4> */
        Application_OwlCarxxx0.Functional_Mode = CMD_MotorControl_Speed; /*电机控制:控制转速模式*/
        CMD_is_MotorSpeed_L = doc["D1"];
        CMD_is_MotorSpeed_R = doc["D2"];

        MotorSpeedAdjustment(&CMD_is_MotorSpeed_L);
        MotorSpeedAdjustment(&CMD_is_MotorSpeed_R);

#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 5:                                                      /*<命令：N 5> */
        Application_OwlCarxxx0.Functional_Mode = CMD_ServoControl; /*舵机控制*/
#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 6:                                                      /*<命令：N 6> */
        Application_OwlCarxxx0.Functional_Mode = CMD_VoiceControl; /*声音控制:有时间限定模式*/
                                                                   // CMD_is_VoiceName = doc["D1"];
        CMD_is_controlAudio = doc["D1"];
        CMD_is_VoiceTimer = doc["T"];
        // #if _is_print
        //         Serial.print('{' + CommandSerialNumber + "_ok}");
        // #endif
        break;

      case 7:                                                                   /*<命令：N 7> */
        Application_OwlCarxxx0.Functional_Mode = CMD_LightingControl_TimeLimit; /*灯光控制:有时间限定模式*/
        CMD_is_LightingSequence = doc["D1"];                                    //Lighting (Left, front, right, back and center)
        CMD_is_LightingColorValue_R = doc["D2"];
        CMD_is_LightingColorValue_G = doc["D3"];
        CMD_is_LightingColorValue_B = doc["D4"];
        CMD_is_LightingTimer = doc["T"];
        Application_OwlCarxxx0.CMD_LightingControl_Millis = millis();
#if _is_print
        //Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 8:                                                                     /*<命令：N 8> */
        Application_OwlCarxxx0.Functional_Mode = CMD_LightingControl_NoTimeLimit; /*灯光控制*/
        CMD_is_LightingSequence = doc["D1"];                                      //Lighting (Left, front, right, back and center)
        CMD_is_LightingColorValue_R = doc["D2"];
        CMD_is_LightingColorValue_G = doc["D3"];
        CMD_is_LightingColorValue_B = doc["D4"];
#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 9:                                                              /*<命令：N 9> */
        Application_OwlCarxxx0.Functional_Mode = CMD_ledExpressionControl; /*led矩阵显示表情控制*/

#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        uint16_t is_arry[8];
        is_arry[0] = doc["D1"];
        is_arry[1] = doc["D2"];
        is_arry[2] = doc["D3"];
        is_arry[3] = doc["D4"];
        is_arry[4] = doc["D5"];
        is_arry[5] = doc["D6"];
        is_arry[6] = doc["D7"];
        is_arry[7] = doc["D8"];
#if _Test_print
        for (int i = 0; i < 8; i++)
        {
          Serial.println(is_arry[i]);
        }
#endif
        {
          /*将双字节数据转换成单字节，且最高位移动到最低位*/
          uint8_t is_byte = 0x00;
          for (int is_true_j = 0; is_true_j < 16; is_true_j++)
          {
            for (int is_true_i = 0; is_true_i < 8; is_true_i++)
            {
              if ((is_arry[is_true_i] << is_true_j) & 0X8000) //取双字节最高位 如果为 1?
              {
                is_byte |= (0X01 << is_true_i); //置位1
              }
              else
              {
                is_byte &= ~(0X01 << is_true_i); //置位0
              }
            }
            CMD_is_LEDCustomExpression_arry[is_true_j] = is_byte; //更新单字节数据缓存
            is_byte = 0x00;
          }
        }

        // #if _is_print
        //         Serial.print('{' + CommandSerialNumber + "_ok}");
        // #endif
        break;

      case 10:                                                         /*<命令：N 10> */
        Application_OwlCarxxx0.Functional_Mode = CMD_ledNumberControl; /*led矩阵显示数字控制*/

        CMD_is_LEDNumber = doc["D1"];
#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 21: /*<命令：N 21>：超声波模块:测距 */
        CMD_UltrasoundModuleStatus_xxx0(doc["D1"]);
#if _is_print
        //Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 22: /*<命令：N 22>：红外模块：寻迹 */
        CMD_TraceModuleStatus_xxx0(doc["D1"]);
#if _is_print
        //Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 23: /*<命令：N 23>：是否离开地面 */
        if (true == Car_LeaveTheGround)
        {
#if _is_print
          Serial.print('{' + CommandSerialNumber + "_false}");
#endif
        }
        else if (false == Car_LeaveTheGround)
        {
#if _is_print
          Serial.print('{' + CommandSerialNumber + "_true}");
#endif
        }
        break;
      case 103:                                                         /*<命令：N 103> */
        Application_OwlCarxxx0.Functional_Mode = CMD_TrajectoryControl; /*轨迹控制*/

        CMD_is_TrajectoryControl_axisPlaneData_X = doc["D1"];
        CMD_is_TrajectoryControl_axisPlaneData_Y = doc["D2"];
#if _is_print
        //Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;
      case 110:                                                                          /*<命令：N 110> */
        Application_OwlCarxxx0.Functional_Mode = CMD_ClearAllFunctions_Programming_mode; /*清除功能:进入编程模式*/
        break;
      case 100:                                                                      /*<命令：N 100> */
        Application_OwlCarxxx0.Functional_Mode = CMD_ClearAllFunctions_Standby_mode; /*清除功能：进入空闲模式*/
        break;

      case 101: /*<命令：N 101> :遥控切换命令*/
        if (1 == doc["D1"])
        {
          Application_OwlCarxxx0.Functional_Mode = TraceBased_mode;
        }
        else if (2 == doc["D1"])
        {
          Application_OwlCarxxx0.Functional_Mode = ObstacleAvoidance_mode;
        }

#if _is_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 104: /*<命令：N 104> :循迹灵敏度调节控制命令*/

        //TrackingDetection_E = doc["D1"];
        TrackingDetection_S = doc["D1"];
        if (TrackingDetection_S > 500)
        {
          TrackingDetection_S = 500;
        }

#if _Test_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      case 105: /*<命令：N 105> :FastLED亮度调节控制命令*/
        if (1 == doc["D1"] && (CMD_is_FastLED_setBrightness < 250))
        {
          CMD_is_FastLED_setBrightness += 5;
        }
        else if (2 == doc["D1"] && (CMD_is_FastLED_setBrightness > 0))
        {
          CMD_is_FastLED_setBrightness -= 5;
        }
        FastLED.setBrightness(CMD_is_FastLED_setBrightness);

#if _Test_print
        Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;
      case 102: /*<命令：N 102> :摇杆控制命令*/
        Application_OwlCarxxx0.Functional_Mode = Rocker_mode;

        Rocker_temp = doc["D1"];
        //Rocker_CarSpeed = doc["D2"];
        switch (Rocker_temp)
        {
        case 1:
          Application_OwlCarxxx0.Motion_Control = Forward;
          break;
        case 2:
          Application_OwlCarxxx0.Motion_Control = Backward;
          break;
        case 3:
          Application_OwlCarxxx0.Motion_Control = Left;
          break;
        case 4:
          Application_OwlCarxxx0.Motion_Control = Right;
          break;
        case 5:
          Application_OwlCarxxx0.Motion_Control = LeftForward;
          break;
        case 6:
          Application_OwlCarxxx0.Motion_Control = LeftBackward;
          break;
        case 7:
          Application_OwlCarxxx0.Motion_Control = RightForward;
          break;
        case 8:
          Application_OwlCarxxx0.Motion_Control = RightBackward;
          break;
        case 9:
          Application_OwlCarxxx0.Motion_Control = stop_it;
          break;
        default:
          Application_OwlCarxxx0.Motion_Control = stop_it;
          break;
        }
        //ApplicationFunctionSet_OwlCarMotionControl(Application_OwlCarxxx0.Motion_Control /*direction*/, 255 /*speed*/);
#if _is_print
        // Serial.print('{' + CommandSerialNumber + "_ok}");
#endif
        break;

      default:
        break;
      }
    }
    else
    {
    }
  }
}
