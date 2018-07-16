#include "resister.h"
#include "MyMap.h"

extern unsigned char LiXian_flag[256];

u8 receCount_HOST;        //���յ����ֽڸ���
u16 recbyteCount;         //���յ����������ֽ���
u8 sensorNum_nowAccess=0x00; //��ǰ���ʵĴ�������ַ
u8 sensorAddress_nowAccess=0x00; //��ǰ���ʵĴ�������ַ
u8 sensorDataNum_nowAccess=0x00; //��ǰ���ʵĴ����������ؼ�����Ч���ݣ��¶ȣ��̸�ֵ���2��
u8 receBuf_HOST[50];      //���ջ�����
u8 sendBuf_HOST[50];		  //���ͻ�����
u16 TimeOutCount_HOST;	 	//���ճ�ʱʱ��

u16 error=0;
u16 a=0;
u16 c=0;
u16 d=0;
u16 e=0;
/**
 * ��������uart1sends
 * ����  �����ͺ���
 * ����  ��buff ����  len ����
 * ���  ����
 * ����  ���ڲ�����
 * ����  :YaoTong 2017
 */
void uart2sends(u8 buff[],u8 len)
{
    u8 i;
    for(i=0; i<len; i++)
    {
        USART_SendData(USART2,buff[i]);
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
    }
}




/**
 * ��������readHoldRegisters
 * ����  �������ּĴ���  fuction:03
 * ����  ��addr:�ӻ���ַ registerStartaddr���Ĵ�����ʼ��ַ  bitCount�Ĵ�������
 * ���  ������1  ʧ��    ����0  �ɹ�
 * ����  ���ⲿ����
 * ����  :YaoTong 2017
 */
u8 SendCMD03toSensor(u8 addr,u16 registerStartaddr,u16 registernum)
{
    u16 crcData,i;


    sensorAddress_nowAccess=addr;
    sensorDataNum_nowAccess=registernum;

    b485Send1  = 1;

    for(i=0; i<50; i++)
        sendBuf_HOST[i]=0x00;
    i=0;

    sendBuf_HOST[0] = addr;
    sendBuf_HOST[1] = 3;
    sendBuf_HOST[2] = registerStartaddr>>8;
    sendBuf_HOST[3] = registerStartaddr;
    sendBuf_HOST[4] = registernum>>8;
    sendBuf_HOST[5] = registernum;
    crcData    = crc16(sendBuf_HOST,6);
    sendBuf_HOST[6] = crcData & 0xff;
    sendBuf_HOST[7] = crcData >> 8 ;

    uart2sends(sendBuf_HOST,8);
    delay_ms(1);
    b485Send1  = 0;
    recbyteCount=registernum*2+5;

    receCount_HOST  = 0; 	 				//����յ�ַƫ�ƼĴ���, �ȴ�����
    TimeOutCount_HOST=recbyteCount*4; 		//���ݽ������ݳ���ȷ���սӳ�ʱʱ��

    return 1;
}


void GetDataFromSensor(unsigned char ucData)
{
    u16 crcData,temp,i;
    u16 tempData;

    d++;
    TimeOutCount_HOST=recbyteCount*4;


    receBuf_HOST[receCount_HOST++]=ucData;

    //UART1_Put_Char(ucData);
    //һֱ�ȵ����չ�8���ֽ�

    if(receBuf_HOST[0]!=sensorAddress_nowAccess)
    {
        //receCount_HOST=0;
        return;

    }
    if(receCount_HOST<recbyteCount)
    {
        e++;
        return;
    }
    else
    {
        a++;
        if((receBuf_HOST[0]==sensorAddress_nowAccess)&&(receBuf_HOST[1]==3))   		//�˶Խ������ݵ�ַ
        {
            c++;
            //�ж��Ƿ����ߣ�1�������ߣ�0��������
            LiXian_flag[sensorNum_nowAccess]=0;

            crcData = crc16(receBuf_HOST,receBuf_HOST[2]+3);      //�˶�У����
            temp=receBuf_HOST[receBuf_HOST[2]+4];
            temp=(temp<<8)+receBuf_HOST[receBuf_HOST[2]+3];		//��λ��ǰ
            if(crcData == temp)
            {

                for(i=0; i<sensorDataNum_nowAccess; i++)
                {
                    tempData=receBuf_HOST[3+2*i]<<8;
                    tempData+=receBuf_HOST[4+2*i];
                    ////��Ҫ�޸�
                    //setRegisterVal(sensorNum_nowAccess*Device_RWReg_Union.device.BytesPerSensor + i,tempData);	  //ÿ����������������BytesPerSensor��u16�������洢����

                    Device_RWReg_Union.device.sensor[sensorNum_nowAccess].data[i] = tempData;
                }
                receCount_HOST = 0;

            }
            else
            {
                error++;
            }
        }
        receCount_HOST = 0;
    }

    receCount_HOST=0;
}




