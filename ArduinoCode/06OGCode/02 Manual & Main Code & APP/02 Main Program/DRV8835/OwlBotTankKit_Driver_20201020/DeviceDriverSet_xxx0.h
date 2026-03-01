/*
 * @Author: ELEGOO
 * @Date: 2019-07-10 16:46:17
 * @LastEditTime: 2020-10-19 18:06:24
 * @LastEditors: Changhua
 * @Description: OwlBot Tank Kit
 * @FilePath: 
 */
#ifndef _DeviceDriverSet_xxx0_H_
#define _DeviceDriverSet_xxx0_H_

#include "pitches.h"

#define _Test_DeviceDriverSet 1
#define TimeCompensation 4 //时间补偿系数  （由于修改了T0定时器，导致标准库的 delay() 、millis()函数出现偏差，特此设置误差调节）
extern unsigned long _millis();
extern void _delay(unsigned long ms);
/*RBG LED*/
#include "FastLED.h"
    class DeviceDriverSet_RBGLED
{
public:
  void DeviceDriverSet_RBGLED_Init(uint8_t set_Brightness);
  void DeviceDriverSet_RBGLED_xxx(uint16_t Duration, uint8_t Traversal_Number, CRGB colour);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_RBGLED_Test(void);
#endif
  void DeviceDriverSet_RBGLED_Color(uint8_t LED_s, uint8_t r, uint8_t g, uint8_t b);

public:
private:
#define PIN_RBGLED 10
//#define PIN_RBGLED 3
#define NUM_LEDS 5
public:
  CRGB leds[NUM_LEDS];
};
/*passive Buzzer*/
class DeviceDriverSet_passiveBuzzer
{
public:
  void DeviceDriverSet_passiveBuzzer_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_passiveBuzzer_Test(void);
#endif
  void DeviceDriverSet_passiveBuzzer_control(void);

public:
  void DeviceDriverSet_passiveBuzzer_controlMonosyllabic(uint8_t controlMonosyllabic, uint32_t Duration);
  void DeviceDriverSet_passiveBuzzer_controlAudio(uint16_t controlAudio, uint32_t Duration);
  void DeviceDriverSet_passiveBuzzer_Scale_c8(uint32_t Duration);

private:
#define PIN_passiveBuzzer 5
  int duration = 200;                                                                       // 500 miliseconds
  int melody[8] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6}; // notes in the melody
  //int melody[8] = {523, 587, 659, 698, 784, 880, 988, 1047}; // notes in the melody
};
/*Key Detection*/
class DeviceDriverSet_Key
{
public:
  void DeviceDriverSet_Key_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_Key_Test(void);
#endif
  void DeviceDriverSet_key_Get(uint8_t *get_keyValue);

public:
#define PIN_Key 2
#define keyValue_Max 3
public:
  static uint8_t keyValue;
};

/*ITR20001 Detection*/
class DeviceDriverSet_ITR20001
{
public:
  void DeviceDriverSet_ITR20001_Init(void);
  float DeviceDriverSet_ITR20001_getAnaloguexxx_L(void);
  float DeviceDriverSet_ITR20001_getAnaloguexxx_R(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_ITR20001_Test(void);
#endif

private:
#define PIN_ITR20001xxxL A0
#define PIN_ITR20001xxxR A1
};

/*Voltage Detection*/
class DeviceDriverSet_Voltage
{
public:
  void DeviceDriverSet_Voltage_Init(void);
  float DeviceDriverSet_Voltage_getAnalogue(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_Voltage_Test(void);
#endif
private:
#define PIN_Voltage A3
};
/*MIC Detection*/
class DeviceDriverSet_MIC
{
public:
  void DeviceDriverSet_MIC_Init(void);
  float DeviceDriverSet_MIC_getAnalogue(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_MIC_Test(void);
#endif

private:
#define PIN_MIC A2
};
/*Motor*/
class DeviceDriverSet_Motor
{
public:
  void DeviceDriverSet_Motor_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_Motor_Test(void);
#endif
  void DeviceDriverSet_Motor_control(boolean direction_A, uint8_t speed_A, //A组电机参数
                                     boolean direction_B, uint8_t speed_B, //B组电机参数
                                     boolean controlED                     //AB使能允许 true
  );                                                                       //电机控制
private:
//#define PIN_Motor_STBY 4
#define PIN_Motor_PWMA 9
//#define PIN_Motor_PWMB 3 //测试版用
//#define PIN_Motor_PWMB 11
#define PIN_Motor_PWMB 6
#define PIN_Motor_AIN 8
#define PIN_Motor_BIN 7

public:
#define speed_Max 250
#define direction_just true
#define direction_back false
#define direction_void 3

#define Duration_enable true
#define Duration_disable false
#define control_enable true
#define control_disable false
};
/*ULTRASONIC*/
#include "I2Cdev.h"
class DeviceDriverSet_ULTRASONIC
{
public:
  void DeviceDriverSet_ULTRASONIC_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_ULTRASONIC_Test(void);
#endif
  void DeviceDriverSet_ULTRASONIC_Get(uint16_t *ULTRASONIC_Get /*out*/);

private:
};

/*STM8S003F3 IR IIC*/
#include <Wire.h>
class DeviceDriverSet_STM8S003F3_IR
{
public:
  void DeviceDriverSet_STM8S003F3_IR_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_STM8S003F3_IR_Test(void);
#endif
  void DeviceDriverSet_STM8S003F3_IR_Get(uint16_t *STM8S003F3_IRL /*out*/, uint16_t *STM8S003F3_IRM /*out*/, uint16_t *STM8S003F3_IRR /*out*/);

private:
#define STM8S003F3_IR_devAddr 0XA0
};
/*STM8S003F3 MPU6050 IIC */
class DeviceDriverSet_MPU6050
{
public:
  void DeviceDriverSet_MPU6050_Init(void);
#if _Test_DeviceDriverSet
  void DeviceDriverSet_MPU6050_Test(void);
#endif
  void DeviceDriverSet_MPU6050_dveGetEulerAngles(float *is_yaw);
  void DeviceDriverSet_MPU6050_dveGetEulerAngles(float *gyro, float *is_yaw);

private:
#define STM8S003F3_MPU6050_devAddr 0XA1
};

#endif
