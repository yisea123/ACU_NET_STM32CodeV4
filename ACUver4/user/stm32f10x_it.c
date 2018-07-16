#include "stm32f10x_it.h"
#include "string.h"
#include "XC_Config.h"

#include "QueueByte.h"

#include <stdio.h>
#include <stdlib.h>

#include "wdg.h"

extern 	u8   receTimeOut_USART1;            	     		 //接收超时
extern 	u8   receCount_USART1;    		  			 				 //接收到的字节个数
extern	u8   receBuf_USART1[50];
extern  u16   timeMs;
extern  u16   timeMs_Uoload;
extern u16   timeMs_LED;

QUEUE_byte Queue_UART4_Rece;
uint8_t SendDataToExternRS485_busy=0;

u8 Timeout_IWG=0; //看门狗定时器

void HardFault_Handler(void)
{

    while (1);

}

void SendDataToExternRS485(void)
{
    uint8_t num_quene;
    uint8_t i;
    if(Device_RWReg_Union.device.WorkMod ==0 )//&& Device_RWReg_Union.device.Address_ExternRS485 ==)  //读挂载设备
    {
        //直接传输给扩展设备

        
        //SendDataToExternRS485();
        //uart4sends(receBuf_USART1,8);

        num_quene = Queue_byteCount(&Queue_UART4_Rece);
        if(num_quene !=0 )
        {
					b485Send2  = 1;
            for(i=0; i<num_quene+1; )
            {
              if(SendDataToExternRS485_busy == 0)   //串口1没有正在接收数据时
							{
								USART_SendData(UART4,GetByteFromQueue(&Queue_UART4_Rece));
                while(USART_GetFlagStatus(UART4,USART_FLAG_TXE)==RESET);
								i++;
							}
							else
							{
							 SendDataToExternRS485_busy = 0;
								break;
							}
                
            }
					b485Send2  = 0;
        }


        //receCount_USART1 = 0; //读挂载485设备时，不进行缓存
        Device_RWReg_Union.device.RJ45Busy = 2;
        
    }
	}

/**
* 函数名：TIM2_IRQHandler
* 描述  ：定时器中断
* 输入  ：无
* 输出  ：无
* 调用  ：系统调用
* 作者  :YaoTong 2017
*/
void TIM2_IRQHandler(void)
{
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);	/* 清除溢出中断标志 */
    Timeout_IWG++;
    if(Timeout_IWG>10)
    {
        IWDG_Feed();
    }

    timeMs++;
    timeMs_LED++;
    if(timeMs_LED>=1000)
    {
        timeMs_LED=0;
    }
    timeMs_Uoload++;

    /*
    	只要串口接收成功了，那么receTimeOut_USART1=10，没1MS自减1，
    	判断如果自减到了0通讯还没恢复，并且偏移地址大于0，
    	则设置485为接收模式。
    */
    if(receTimeOut_USART1 > 0)
    {
        receTimeOut_USART1--;

        //判断通讯接收是否超时
        if((receTimeOut_USART1 == 0) && (receCount_USART1 > 0))
        {
            //			b485Send = 0;      //将485置为接收状态
            receCount_USART1 = 0;     //将接收地址偏移寄存器清零
        }
    }
		
		//透传网线传来的数据给外扩485设备
		SendDataToExternRS485();
		
		
		
}

/**
 * 函数名：USART1_IRQHandler
 * 描述  ：串口中断
 * 输入  ：
 * 输出  ：
 * 调用  ：系统调用
 * 作者  :YaoTong 2017
 */
void USART1_IRQHandler(void)
{
    /* 串口接收中断 */

    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
    {
        receBuf_USART1[receCount_USART1] = USART_ReceiveData(USART1);
				SendDataToExternRS485_busy = 1;
        PutByteInQueue(&Queue_UART4_Rece, receBuf_USART1[receCount_USART1]);

        //通讯超时值这个地方很重要	10ms
        receTimeOut_USART1 = 10;
        //接收地址偏移寄存器加1
        receCount_USART1++;

        USART1->SR = 0;
			
				
    }
}


void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        GetDataFromSensor((unsigned char)USART2->DR);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    USART_ClearITPendingBit(USART2,USART_IT_ORE);
}

unsigned char uart4_temp;


void UART4_IRQHandler(void)
{



    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        uart4_temp = (unsigned char)UART4->DR;
        USART_SendData(USART1,uart4_temp);

    }

    USART_ClearITPendingBit(UART4,USART_IT_ORE);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

