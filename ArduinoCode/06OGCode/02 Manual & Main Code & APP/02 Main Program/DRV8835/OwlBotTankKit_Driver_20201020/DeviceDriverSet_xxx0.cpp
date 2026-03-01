/*
 * @Author: ELEGOO
 * @Date: 2019-07-10 16:46:17
 * @LastEditTime: 2021-01-04 11:16:53
 * @LastEditors: Changhua
 * @Description: OwlBot Tank Kit
 * @FilePath: 
 */

#include "DeviceDriverSet_xxx0.h"
#include "PinChangeInt.h"

extern unsigned long _millis()
{
  return millis() * TimeCompensation;
}
extern void _delay(unsigned long ms)
{
  delay(ms / TimeCompensation);
}

/*RBG LED*/
static uint32_t
Color(uint8_t r, uint8_t g, uint8_t b)
{
  return (((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}
void DeviceDriverSet_RBGLED::DeviceDriverSet_RBGLED_xxx(uint16_t Duration, uint8_t Traversal_Number, CRGB colour)
{
  if (NUM_LEDS < Traversal_Number)
  {
    Traversal_Number = NUM_LEDS;
  }
  for (int Number = 0; Number < Traversal_Number; Number++)
  {
    leds[Number] = colour;
    FastLED.show();
    _delay(Duration);
  }
}
void DeviceDriverSet_RBGLED::DeviceDriverSet_RBGLED_Init(uint8_t set_Brightness)
{
  FastLED.addLeds<NEOPIXEL, PIN_RBGLED>(leds, NUM_LEDS);
  FastLED.setBrightness(set_Brightness);
}
#if _Test_DeviceDriverSet
void DeviceDriverSet_RBGLED::DeviceDriverSet_RBGLED_Test(void)
{
  leds[0] = CRGB::White;
  FastLED.show();
  _delay(50);
  leds[1] = CRGB::Red;
  FastLED.show();
  _delay(50);
  leds[2] = CRGB::Green;
  FastLED.show();
  _delay(50);
  leds[3] = CRGB::Yellow;
  FastLED.show();
  _delay(50);
  leds[4] = CRGB::Orange;
  FastLED.show();
  _delay(50);
  DeviceDriverSet_RBGLED_xxx(50 /*Duration*/, 5 /*Traversal_Number*/, CRGB::Black);
}
#endif

void DeviceDriverSet_RBGLED::DeviceDriverSet_RBGLED_Color(uint8_t LED_s, uint8_t r, uint8_t g, uint8_t b)
{
  if (LED_s > NUM_LEDS)
    return;
  if (LED_s == NUM_LEDS)
  {
    FastLED.showColor(Color(r, g, b));
  }
  else
  {
    leds[LED_s] = Color(r, g, b);
  }
  FastLED.show();
}

/*passive Buzzer*/
void DeviceDriverSet_passiveBuzzer::DeviceDriverSet_passiveBuzzer_Init(void)
{
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_passiveBuzzer::DeviceDriverSet_passiveBuzzer_Test(void)
{
  for (int thisNote = 0; thisNote < 8; thisNote++)
  {
    tone(5, melody[thisNote], duration); // pinxx output the voice, every scale is 0.5 sencond
    _delay(1000);                        // Output the voice after several minutes
  }
}
#endif

void DeviceDriverSet_passiveBuzzer::DeviceDriverSet_passiveBuzzer_controlMonosyllabic(uint8_t controlMonosyllabic, uint32_t Duration)
{
  if (controlMonosyllabic > 7) //0----7
    controlMonosyllabic = 7;
  //Serial.println(Duration);
  tone(5, melody[controlMonosyllabic], Duration); // pinxx output the voice, every scale is 0.5 sencond
  _delay(Duration);
}

void DeviceDriverSet_passiveBuzzer::DeviceDriverSet_passiveBuzzer_controlAudio(uint16_t controlAudio, uint32_t Duration)
{
  if (controlAudio > 2500)
  {
    controlAudio = 2500;
  }
  tone(5, controlAudio, Duration); //
  _delay(Duration);
}

void DeviceDriverSet_passiveBuzzer::DeviceDriverSet_passiveBuzzer_Scale_c8(uint32_t Duration)
{
  for (int thisNote = 0; thisNote < 9; thisNote++)
  {
    tone(5, melody[thisNote], duration); // pinxx output the voice, every scale is 0.5 sencond
    _delay(Duration);
  }
}

/*Key*/
uint8_t DeviceDriverSet_Key::keyValue = 1;

static void attachPinChangeInterrupt_GetKeyValue(void)
{
  DeviceDriverSet_Key Key;
  static uint32_t keyValue_time = 0;
  static uint8_t keyValue_temp = 1;
  if ((_millis() - keyValue_time) > 500)
  {
    keyValue_temp += 1;
    keyValue_time = _millis();
    if (keyValue_temp > keyValue_Max)
    {
      keyValue_temp = 1;
    }
    Key.keyValue = keyValue_temp;
  }
}
void DeviceDriverSet_Key::DeviceDriverSet_Key_Init(void)
{
  pinMode(PIN_Key, INPUT_PULLUP);
  attachPinChangeInterrupt(PIN_Key, attachPinChangeInterrupt_GetKeyValue, FALLING);
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_Key::DeviceDriverSet_Key_Test(void)
{
  Serial.println(DeviceDriverSet_Key::keyValue);
}
#endif

void DeviceDriverSet_Key::DeviceDriverSet_key_Get(uint8_t *get_keyValue)
{
  *get_keyValue = keyValue;
}

/*ITR20001 Detection*/
void DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_Init(void)
{
  pinMode(PIN_ITR20001xxxL, INPUT);
  pinMode(PIN_ITR20001xxxR, INPUT);
}
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_L(void)
{
  return analogRead(PIN_ITR20001xxxL);
}
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_R(void)
{
  return analogRead(PIN_ITR20001xxxR);
}
#if _Test_DeviceDriverSet
void DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_Test(void)
{
  Serial.print("ITR20001_getAnaloguexxx_L=");
  Serial.println(analogRead(PIN_ITR20001xxxL));
  Serial.print("ITR20001_getAnaloguexxx_R=");
  Serial.println(analogRead(PIN_ITR20001xxxR));
}
#endif

/*Voltage Detection*/
void DeviceDriverSet_Voltage::DeviceDriverSet_Voltage_Init(void)
{
  pinMode(PIN_Voltage, INPUT);
}
float DeviceDriverSet_Voltage::DeviceDriverSet_Voltage_getAnalogue(void)
{
  return (analogRead(PIN_Voltage) * 4.97 / 1024) * 8.0;
  // analogRead(PIN_Voltage) * 4.97 / 1024;
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_Voltage::DeviceDriverSet_Voltage_Test(void)
{
  Serial.print("Voltage=");
  Serial.println(analogRead(PIN_Voltage) * 4.97 / 1024);
}
#endif

/*MIC Detection*/
void DeviceDriverSet_MIC::DeviceDriverSet_MIC_Init(void)
{
  pinMode(PIN_MIC, INPUT);
}
float DeviceDriverSet_MIC::DeviceDriverSet_MIC_getAnalogue(void)
{
  return analogRead(PIN_MIC);
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_MIC::DeviceDriverSet_MIC_Test(void)
{
  Serial.print("MIC=");
  Serial.println(analogRead(PIN_MIC));
}
#endif

/*Motor control*/
void DeviceDriverSet_Motor::DeviceDriverSet_Motor_Init(void)
{

  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);
  pinMode(PIN_Motor_AIN, OUTPUT);
  pinMode(PIN_Motor_BIN, OUTPUT);

  //cli(); // 禁止中断
  // // set timer1 fast pwm, f = 62.5kHz
  // // D9 = OCR1A; D10 = OCR1B
  // pinMode(9, OUTPUT);
  // //pinMode(10, OUTPUT);
  // TCCR1A = 0;
  // TCCR1B = 0;
  // // 比较匹配时清零 OC1A/OC1B
  // TCCR1A |= _BV(COM1A1) | _BV(COM1B1);
  // // 配置8位快速PWM, TOP = 0xFF
  // TCCR1A |= _BV(WGM10);
  // TCCR1B |= _BV(WGM12);
  // // 时钟无分频
  // //TCCR1B |= _BV(CS10);
  TCCR1B = TCCR1B & 0b11111000 | 0x04; //D9
  //OCR1A = 200;                         // 占空比50%
  //OCR1B = 200;                         // 占空比20%

  TCCR0B = TCCR0B & 0b11111000 | 0x04; //D6

  //sei(); // 允许中断
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_Motor::DeviceDriverSet_Motor_Test(void)
{
  //digitalWrite(PIN_Motor_STBY, HIGH);
  digitalWrite(PIN_Motor_AIN, LOW);
  digitalWrite(PIN_Motor_BIN, HIGH);
  // OCR1A = 150; // 占空比50%
  // OCR1B = 150; // 占空比20%
  //OCR1A = 150;
  //OCR1B = 150;
  analogWrite(PIN_Motor_PWMA, 150);
  analogWrite(PIN_Motor_PWMB, 150); //Z
}
#endif

/*
 Motor_control：AB / 方向、速度
*/
void DeviceDriverSet_Motor::DeviceDriverSet_Motor_control(boolean direction_A, uint8_t speed_A, //A组电机参数
                                                          boolean direction_B, uint8_t speed_B, //B组电机参数
                                                          boolean controlED                     //AB使能允许 true
                                                          )                                     //电机控制
{
  //if (speed_A > speed_Max || speed_B > speed_Max) //大于最大输出速度量视为不合法值
  //  return;

  if (controlED == control_enable) //使能允许？
  {
    // digitalWrite(PIN_Motor_STBY, HIGH); //开启
    {                      //A...
      switch (direction_A) //方向控制
      {
      case direction_just:
        digitalWrite(PIN_Motor_AIN, LOW);
        analogWrite(PIN_Motor_PWMA, speed_A);
        break;
      case direction_back:
        digitalWrite(PIN_Motor_AIN, HIGH);
        analogWrite(PIN_Motor_PWMA, speed_A);
        break;
      case direction_void:
        analogWrite(PIN_Motor_PWMA, 0);
        break;
      default:
        analogWrite(PIN_Motor_PWMA, 0);
        break;
      }
    }
    { //B...
      switch (direction_B)
      {
      case direction_just:
        digitalWrite(PIN_Motor_BIN, HIGH);
        analogWrite(PIN_Motor_PWMB, speed_B);
        break;
      case direction_back:
        digitalWrite(PIN_Motor_BIN, LOW);
        analogWrite(PIN_Motor_PWMB, speed_B);

        break;
      case direction_void:
        analogWrite(PIN_Motor_PWMB, 0);
        break;
      default:
        analogWrite(PIN_Motor_PWMB, 0);
        break;
      }
    }
  }
  else
  {
    //digitalWrite(PIN_Motor_STBY, LOW); //关闭
    analogWrite(PIN_Motor_PWMA, 0);
    analogWrite(PIN_Motor_PWMB, 0);
    return;
  }
}

/*ULTRASONIC*/
void DeviceDriverSet_ULTRASONIC::DeviceDriverSet_ULTRASONIC_Init(void)
{
  Wire.begin();
}

void DeviceDriverSet_ULTRASONIC::DeviceDriverSet_ULTRASONIC_Get(uint16_t *ULTRASONIC_Get /*out*/)
{
  unsigned dat[2] = {0};
  Wire.requestFrom(0x07, 1); //从器件读取一位数
  if (Wire.available() > 0)
  {
    dat[0] = Wire.read();
  }
  Wire.requestFrom(0x07, 1); //从器件读取一位数
  if (Wire.available() > 0)
  {
    dat[1] = Wire.read();
  }
  *ULTRASONIC_Get = ((dat[0] << 8) | dat[1]);
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_ULTRASONIC::DeviceDriverSet_ULTRASONIC_Test(void)
{
  unsigned dat[2] = {0};
  Wire.requestFrom(0x07, 1); //从器件读取一位数
  if (Wire.available() > 0)
  {
    dat[0] = Wire.read();
  }

  Wire.requestFrom(0x07, 1); //从器件读取一位数
  if (Wire.available() > 0)
  {
    dat[1] = Wire.read();
  }
  _delay(100);
  Serial.print("ULTRASONIC=");
  Serial.println((dat[0] << 8) | dat[1]);
}
#endif

/*STM8S003F3_IR*/
void DeviceDriverSet_STM8S003F3_IR::DeviceDriverSet_STM8S003F3_IR_Init(void)
{
  Wire.begin();
  Wire.begin();
  Wire.beginTransmission(STM8S003F3_IR_devAddr);
  Wire.write(110);
  Wire.endTransmission();
}

void DeviceDriverSet_STM8S003F3_IR::DeviceDriverSet_STM8S003F3_IR_Get(uint16_t *STM8S003F3_IRL /*out*/, uint16_t *STM8S003F3_IRM /*out*/, uint16_t *STM8S003F3_IRR /*out*/)
{
  uint8_t STM8S003F3_IR_IIC_buff[8];
  uint8_t a = 0;
  Wire.requestFrom(STM8S003F3_IR_devAddr, 8); // request 6 bytes from slave device #2
  while (Wire.available())                    // slave may send less than requested
  {
    STM8S003F3_IR_IIC_buff[a++] = Wire.read(); // receive a byte as character
  }

  if ((STM8S003F3_IR_IIC_buff[0] == 0XA0) && (STM8S003F3_IR_IIC_buff[7] == 0XB0))
  {

    *STM8S003F3_IRR = (STM8S003F3_IR_IIC_buff[1] << 8) | (STM8S003F3_IR_IIC_buff[2]);
    *STM8S003F3_IRL = (STM8S003F3_IR_IIC_buff[3] << 8) | (STM8S003F3_IR_IIC_buff[4]);
    *STM8S003F3_IRM = (STM8S003F3_IR_IIC_buff[5] << 8) | (STM8S003F3_IR_IIC_buff[6]);
  }
  else
  {
    /* code */
    //Serial.println("Contact Changhua :STM8S003F3_IR data error"); // print the character
    return;
  }
}

#if _Test_DeviceDriverSet
void DeviceDriverSet_STM8S003F3_IR::DeviceDriverSet_STM8S003F3_IR_Test(void)
{
  uint8_t STM8S003F3_IR_IIC_buff[8];
  uint8_t a = 0;
  Wire.requestFrom(STM8S003F3_IR_devAddr, 8); // request 6 bytes from slave device #2
  while (Wire.available())                    // slave may send less than requested
  {
    STM8S003F3_IR_IIC_buff[a++] = Wire.read(); // receive a byte as character
  }

  if ((STM8S003F3_IR_IIC_buff[0] == 0XA0) && (STM8S003F3_IR_IIC_buff[7] == 0XB0))
  {

    Serial.print("\t   L:");
    Serial.print((STM8S003F3_IR_IIC_buff[3] << 8) | (STM8S003F3_IR_IIC_buff[4]));

    Serial.print("\t   M:");
    Serial.print((STM8S003F3_IR_IIC_buff[5] << 8) | (STM8S003F3_IR_IIC_buff[6]));

    Serial.print("\t   R:");
    Serial.println((STM8S003F3_IR_IIC_buff[1] << 8) | (STM8S003F3_IR_IIC_buff[2]));

    //Serial.println("STM8S003F3_IR"); // print the character
  }
  else
  {
    /* code */

    Serial.println("STM8S003F3_IR data error"); // print the character
  }

  // delay(600);
}
#endif

/*STM8S003F3_MPU6050*/
void DeviceDriverSet_MPU6050::DeviceDriverSet_MPU6050_Init(void)
{
  Wire.begin();
  Wire.beginTransmission(STM8S003F3_MPU6050_devAddr);
  Wire.write(110);
  Wire.endTransmission();
  _delay(50);
}

void DeviceDriverSet_MPU6050::DeviceDriverSet_MPU6050_dveGetEulerAngles(float *is_yaw)
{
  // uint8_t STM8S003F3_MPU6050_IIC_buff[6];
  // uint8_t a = 0;
  // Wire.requestFrom(STM8S003F3_MPU6050_devAddr, 6); // request 6 bytes from slave device #2
  // while (Wire.available())                         // slave may send less than requested
  // {
  //   STM8S003F3_MPU6050_IIC_buff[a++] = Wire.read(); // receive a byte as character
  // }

  // if ((STM8S003F3_MPU6050_IIC_buff[0] == 0XA1) && (STM8S003F3_MPU6050_IIC_buff[5] == 0XB1))
  // {
  //   //*gyro = ((STM8S003F3_MPU6050_IIC_buff[1] << 8) | (STM8S003F3_MPU6050_IIC_buff[2])) / 1000.00;
  //   *is_yaw = ((STM8S003F3_MPU6050_IIC_buff[3] << 8) | (STM8S003F3_MPU6050_IIC_buff[4])) / 100.00;
  // }
  // else
  // {
  //   /* code */
  //   //Serial.println("Contact Changhua :STM8S003F3_MPU6050 data error"); // print the character
  //   return;
  // }
  float gyro, yaw;
  DeviceDriverSet_MPU6050::DeviceDriverSet_MPU6050_dveGetEulerAngles(&gyro, &yaw);
  *is_yaw = yaw;
}

void DeviceDriverSet_MPU6050::DeviceDriverSet_MPU6050_dveGetEulerAngles(float *gyro, float *is_yaw)
{
  uint8_t STM8S003F3_MPU6050_IIC_buff[6];
  uint8_t a = 0;
  Wire.requestFrom(STM8S003F3_MPU6050_devAddr, 6); // request 6 bytes from slave device #2
  while (Wire.available())                         // slave may send less than requested
  {
    STM8S003F3_MPU6050_IIC_buff[a++] = Wire.read(); // receive a byte as character
  }

  if ((STM8S003F3_MPU6050_IIC_buff[0] == 0XA1) && (STM8S003F3_MPU6050_IIC_buff[5] == 0XB1))
  {
    *gyro = ((STM8S003F3_MPU6050_IIC_buff[1] << 8) | (STM8S003F3_MPU6050_IIC_buff[2])) / 1000.00;
    *is_yaw = ((STM8S003F3_MPU6050_IIC_buff[3] << 8) | (STM8S003F3_MPU6050_IIC_buff[4])) / 100.00;
  }
  else
  {
    /* code */
    //Serial.println("Contact Changhua :STM8S003F3_MPU6050 data error"); // print the character
    return;
  }
}
#if _Test_DeviceDriverSet
void DeviceDriverSet_MPU6050::DeviceDriverSet_MPU6050_Test(void)
{

  uint8_t STM8S003F3_MPU6050_IIC_buff[4];
  uint8_t a = 0;
  Wire.requestFrom(STM8S003F3_MPU6050_devAddr, 4); // request 6 bytes from slave device #2
  while (Wire.available())                         // slave may send less than requested
  {
    STM8S003F3_MPU6050_IIC_buff[a++] = Wire.read(); // receive a byte as character
  }

  if ((STM8S003F3_MPU6050_IIC_buff[0] == 0XA1) && (STM8S003F3_MPU6050_IIC_buff[3] == 0XB1))
  {
    Serial.print("STM8S003F3_MPU6050->is_yaw:      ");
    Serial.println((STM8S003F3_MPU6050_IIC_buff[1] << 8) | (STM8S003F3_MPU6050_IIC_buff[2]));
    Serial.println("...................................."); // print the character
  }
  else
  {
    /* code */
    Serial.println("STM8S003F3_MPU6050 data error"); // print the character
  }
}
#endif
