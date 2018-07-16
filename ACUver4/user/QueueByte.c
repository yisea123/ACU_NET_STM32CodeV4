#include "QueueByte.h"




void Queue_byteInit(QUEUE_byte * This)  //?????
{
    This->Front     = 0;
    This->Rear     = 0;
    This->FullFlag = 0;
}
void UpHead(QUEUE_byte *This)  //??????
{
    This->Front = (This->Front + 1) % NUM_MES_DATA;
}
void UpTail(QUEUE_byte *This)  //??????
{
    This->Rear = (This->Rear + 1) % NUM_MES_DATA;
}

unsigned char GetByteFromQueue(QUEUE_byte *This)
{
    unsigned char GetData;
    if((This->Front==This->Rear)&&(!This->FullFlag))
    {
        return 0xff;   //??????,???? !*????
    }
    else
    {
        GetData = This->date[This->Rear];
        UpTail(This);
        This->FullFlag = 0;  //????????
        return GetData;
    }
}
void PutByteInQueue(QUEUE_byte *This,unsigned char PutData)
{
    if(!This->FullFlag)
    {
        This->date[This->Front] = PutData;
        UpHead(This);
        if(This->Front==This->Rear)  //????????
        {
            This->FullFlag = 1;
        }
    }
}
unsigned char Queue_byteCount(QUEUE_byte * This)
{
    if(This->Front<This->Rear)
    {
        return(This->Front + NUM_MES_DATA - This->Rear);
    }
    else
    {
        return(This->Front - This->Rear);
    }
}


