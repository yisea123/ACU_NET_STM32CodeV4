#include "stm32f10x_it.h"
#include "string.h"
#include "XC_Config.h"

#include "QueueByte.h"

#include <stdio.h>
#include <stdlib.h>

#include "wdg.h"

extern 	u8   receTimeOut_USART1;            	     		 //���ճ�ʱ
extern 	u8   receCount_USART1;    		  			 				 //���յ����ֽڸ���
extern	u8   receBuf_USART1[50];
extern  u16   timeMs;
extern  u16   timeMs_Uoload;
extern u16   timeMs_LED;

QUEUE_byte Queue_UART4_Rece;
uint8_t SendDataToExternRS485_busy=0;

u8 Timeout_IWG=0; //���Ź���ʱ��

void HardFault_Handler(void)
{

    while (1);

}

void SendDataToExternRS485(void)
{
    uint8_t num_quene;
    uint8_t i;
    if(Device_RWReg_Union.device.WorkMod ==0 )//&& Device_RWReg_Union.device.Address_ExternRS485 ==)  //�������豸
    {
        //ֱ�Ӵ������չ�豸

        
        //SendDataToExternRS485();
        //uart4sends(receBuf_USART1,8);

        num_quene = Queue_byteCount(&Queue_UART4_Rece);
        if(num_quene !=0 )
        {
					b485Send2  = 1;
            for(i=0; i<num_quene+1; )
            {
              if(SendDataToExternRS485_busy == 0)   //����1û�����ڽ�������ʱ
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


        //receCount_USART1 = 0; //������485�豸ʱ�������л���
        Device_RWReg_Union.device.RJ45Busy = 2;
        
    }
	}

/**
* ��������TIM2_IRQHandler
* ����  ����ʱ���ж�
* ����  ����
* ���  ����
* ����  ��ϵͳ����
* ����  :YaoTong 2017
*/
void TIM2_IRQHandler(void)
{
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);	/* �������жϱ�־ */
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
    	ֻҪ���ڽ��ճɹ��ˣ���ôreceTimeOut_USART1=10��û1MS�Լ�1��
    	�ж�����Լ�����0ͨѶ��û�ָ�������ƫ�Ƶ�ַ����0��
    	������485Ϊ����ģʽ��
    */
    if(receTimeOut_USART1 > 0)
    {
        receTimeOut_USART1--;

        //�ж�ͨѶ�����Ƿ�ʱ
        if((receTimeOut_USART1 == 0) && (receCount_USART1 > 0))
        {
            //			b485Send = 0;      //��485��Ϊ����״̬
            receCount_USART1 = 0;     //�����յ�ַƫ�ƼĴ�������
        }
    }
		
		//͸�����ߴ��������ݸ�����485�豸
		SendDataToExternRS485();
		
		
		
}

/**
 * ��������USART1_IRQHandler
 * ����  �������ж�
 * ����  ��
 * ���  ��
 * ����  ��ϵͳ����
 * ����  :YaoTong 2017
 */
void USART1_IRQHandler(void)
{
    /* ���ڽ����ж� */

    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
    {
        receBuf_USART1[receCount_USART1] = USART_ReceiveData(USART1);
				SendDataToExternRS485_busy = 1;
        PutByteInQueue(&Queue_UART4_Rece, receBuf_USART1[receCount_USART1]);

        //ͨѶ��ʱֵ����ط�����Ҫ	10ms
        receTimeOut_USART1 = 10;
        //���յ�ַƫ�ƼĴ�����1
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
