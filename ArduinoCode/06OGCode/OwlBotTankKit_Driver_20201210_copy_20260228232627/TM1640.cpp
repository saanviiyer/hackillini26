/*
 * @Description: In User Settings Edit
 * @Author:  Changhua
 * @Date: 2019-07-11 11:43:35
 * @LastEditTime: 2019-09-06 17:22:16
 * @LastEditors: Please set LastEditors
 */
#include "TM1640.h"

/*Nop1640*/
static void Nop1640(uint16_t T_Dly)
{
        while (T_Dly--)
                ;
        return;
}
/*TM1640Start*/
static void TM1640Start(void)
{
        TM1640DAT_HING;
        Nop1640(VT_DLYNOP);
        TM1640SLK_HING;
        Nop1640(VT_DLY1640);
        TM1640DAT_LOW;
        Nop1640(VT_DLY1640);
        TM1640SLK_LOW;
        Nop1640(VT_DLY1640);
}
/*TM1640Stop*/
static void TM1640Stop(void)
{
        TM1640DAT_LOW;
        Nop1640(VT_DLYNOP);
        TM1640SLK_HING;
        Nop1640(VT_DLY1640);
        TM1640DAT_HING;
        Nop1640(VT_DLY1640);
        TM1640SLK_LOW;
        Nop1640(VT_DLY1640);
}
/*TM1640WriteByte*/
static void TM1640WriteByte(uint8_t date)
{
        uint8_t i;
        uint8_t Tmp;
        Tmp = date;
        TM1640DAT_LOW;
        Nop1640(VT_DLYNOP);
        TM1640SLK_LOW;

        for (i = 0; i < 8; i++)
        {
                TM1640SLK_LOW;
                Nop1640(VT_DLYNOP);
                if (Tmp & 0x01)
                {
                        TM1640DAT_HING;
                        Nop1640(VT_DLY1640);
                }
                else
                {
                        TM1640DAT_LOW;
                        Nop1640(VT_DLY1640);
                }
                TM1640SLK_HING;
                Tmp = Tmp >> 1;
        }

        TM1640SLK_LOW;
        Nop1640(VT_DLYNOP);
        TM1640DAT_LOW;
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:void TM1640_DisplayClear_led16x8(void)
@ Functional functions: Display Clear
@ Input parameters:     none
@ Output parameters:    none
@ Other Notes:          TM1640 Driver 16*8 Matrix LED Program
$ Shenzhen, China:Elegoo & Changhua & 2019-06
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void TM1640::TM1640_DisplayClear_led16x8(uint8_t InValue)
{

        uint8_t cout;
        TM1640Start();
        TM1640WriteByte(0xc0); //Setting Start Address
        for (cout = 0; cout < 16; cout++)
        {
                TM1640WriteByte(InValue);
        }
        TM1640Stop();
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:void TM1640_InitConfig_led16x8(void)
@ Functional functions: Initial configuration of TM1640 bus interface
@ Input parameters:     InValue  Display switch  
@ Output parameters:    none
@ Other Notes:          TM1640 Driver 16*8 Matrix LED Program
$ Shenzhen, China:Elegoo & Changhua & 2019-07
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void TM1640::TM1640_InitConfig_led16x8(uint8_t InValue)
{

        pinMode(TM1640SLK_PIN, OUTPUT);
        pinMode(TM1640DAT_PIN, OUTPUT);
        digitalWrite(TM1640SLK_PIN, HIGH);
        digitalWrite(TM1640DAT_PIN, HIGH);

        TM1640DAT_HING;
        Nop1640(VT_DLYNOP);
        TM1640SLK_HING;

        if (InValue == DSP1640_DIS) //Turn off the display
        {
                TM1640Start();
                TM1640WriteByte(SET_DSP1640_DIS); //显示关控制   数据写模式设置
                TM1640Stop();
        }
        else
        {
                TM1640Start();
                TM1640WriteByte(SET_DATA_ORDER); //数据命令设置 地址自动加1  数据写模式设置
                TM1640Stop();
                TM1640Start();
                TM1640WriteByte(SET_DISPLAY); //显示命令控制  脉冲宽度14/16  显示亮度设置
                TM1640Stop();
        }

        TM1640::TM1640_DisplayClear_led16x8(0);
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:   void TM1640_FullScreenDisaple_led16x8(uint8_t *cx)
@ Functional functions: Full screen disaple
@ Input parameters:     *cx Input byte data cache
@ Output parameters:    none
@ Other Notes:          TM1640 Driver 16*8 Matrix LED Program
$ Shenzhen, China:Elegoo & Changhua & 2019-07
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void TM1640::TM1640_FullScreenDisaple_led16x8(uint8_t *cx /*in*/)
{
        uint8_t cout;
        TM1640Start();
        TM1640WriteByte(0xc0); //设置起始地址
        for (cout = 0; cout < 16; cout++)
        {
                TM1640WriteByte(cx[cout]);
        }
        TM1640Stop();
}

/*
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
@ Function prototype:TM1640_SendDataxxx(uint8_t coordinatePoint_X,uint8_t coordinatePoint_Y)
@ Functional functions: Designated coordinate point display   1 is valid
@ Input parameters:     coordinatePoint_X <in: 0--15>  
                        coordinatePoint_Y <in: 0--7>
@ Output parameters:    none
@ Other Notes:          TM1640 Driver 16*8 Matrix LED Program
$ Shenzhen, China:Elegoo & Changhua & 2019-07
 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void TM1640::TM1640_DesignatedCoordinatePointDisplay_led16x8(uint8_t coordinatePoint_X, uint8_t coordinatePoint_Y)
{
        if (coordinatePoint_X > 16 || coordinatePoint_Y > 8)
                return;

        TM1640Start();
        TM1640WriteByte(0xC0 + coordinatePoint_X);
        TM1640WriteByte((0X01 << coordinatePoint_Y));
        TM1640Stop();
}
