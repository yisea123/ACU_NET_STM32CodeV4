#include "resister.h"
#include "MyMap.h"

extern unsigned char LiXian_flag[256];

u8 receCount_HOST;        //接收到的字节个数
u16 recbyteCount;         //接收到传感器的字节数
u8 sensorNum_nowAccess=0x00; //当前访问的传感器地址
u8 sensorAddress_nowAccess=0x00; //当前访问的传感器地址
u8 sensorDataNum_nowAccess=0x00; //当前访问的传感器共传回几个有效数据，温度，烟感值算个2个
u8 receBuf_HOST[50];      //接收缓冲区
u8 sendBuf_HOST[50];		  //发送缓冲区
u16 TimeOutCount_HOST;	 	//接收超时时间

u16 error=0;
u16 a=0;
u16 c=0;
u16 d=0;
u16 e=0;
/**
 * 函数名：uart1sends
 * 描述  ：发送函数
 * 输入  ：buff 数组  len 长度
 * 输出  ：无
 * 调用  ：内部调用
 * 作者  :YaoTong 2017
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
 * 函数名：readHoldRegisters
 * 描述  ：读保持寄存器  fuction:03
 * 输入  ：addr:从机地址 registerStartaddr：寄存器起始地址  bitCount寄存器个数
 * 输出  ：返回1  失败    返回0  成功
 * 调用  ：外部调用
 * 作者  :YaoTong 2017
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

    receCount_HOST  = 0; 	 				//清接收地址偏移寄存器, 等待接收
    TimeOutCount_HOST=recbyteCount*4; 		//根据接收数据长度确定收接超时时间

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
    //一直等到接收够8个字节

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
        if((receBuf_HOST[0]==sensorAddress_nowAccess)&&(receBuf_HOST[1]==3))   		//核对接收数据地址
        {
            c++;
            //判断是否离线，1代表离线，0代表在线
            LiXian_flag[sensorNum_nowAccess]=0;

            crcData = crc16(receBuf_HOST,receBuf_HOST[2]+3);      //核对校验码
            temp=receBuf_HOST[receBuf_HOST[2]+4];
            temp=(temp<<8)+receBuf_HOST[receBuf_HOST[2]+3];		//低位在前
            if(crcData == temp)
            {

                for(i=0; i<sensorDataNum_nowAccess; i++)
                {
                    tempData=receBuf_HOST[3+2*i]<<8;
                    tempData+=receBuf_HOST[4+2*i];
                    ////需要修改
                    //setRegisterVal(sensorNum_nowAccess*Device_RWReg_Union.device.BytesPerSensor + i,tempData);	  //每个传感器分配连续BytesPerSensor个u16，用来存储数据

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





