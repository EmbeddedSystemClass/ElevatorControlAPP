/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485_process.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��23��
  ����޸�   :
  ��������   : ���ݿ�������ָ����ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "rs485_process.h"





#define LOG_TAG    "RS485"
#include "elog.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
READER_BUFF_T gReaderMsg;

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/



void packetDefaultSendBuf(uint8_t *buf)
{
    uint8_t sendBuf[64] = {0};

    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
}


void packetSendBuf(READER_BUFF_T *pQueue,uint8_t *buf)
{
    uint8_t jsonBuf[JSON_ITEM_MAX_LEN] = {0};
    uint8_t sendBuf[64] = {0};
    uint16_t len = 0;
    LOCAL_USER_T *localUserData;//= &gLoalUserData;
    sendBuf[0] = CMD_STX;
    sendBuf[1] = bsp_dipswitch_read();
    sendBuf[MAX_SEND_LEN-1] = xorCRC(sendBuf,MAX_SEND_LEN-2);

    memcpy(buf,sendBuf,MAX_SEND_LEN);
    
    authReader(pQueue,localUserData);

    
//    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
//    log_d("localUserData->userId = %s\r\n",localUserData->userId);
//    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
//    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
//    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
//    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
//    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);



     packetPayload(localUserData,jsonBuf); 

     len = strlen(jsonBuf);
     log_d("jsonBuf = %s,len = %d\r\n",jsonBuf,len);

     len = PublishData(jsonBuf,len);

     log_d("send = %d\r\n",len);

}

uint8_t authReader(READER_BUFF_T *pQueue,LOCAL_USER_T *localUserData)
{
    char *value;
    char *buf[6] = {0}; //��ŷָ������ַ��� 
    int num = 0;
    uint8_t key[8] = {0};
    
//    uint8_t userid[16] = {0};
//    uint8_t cardid[16] = {0};
//    uint8_t authFloor[16] = {0};
//    uint8_t defalutFloor[16] = {0};
//    uint8_t startTime[20] = {0};
//    uint8_t endTime[20] = {0};

    //-2 ��0d 0a
    if(AUTH_MODE_QR == pQueue->authMode)
    {
        memcpy(key,pQueue->data+pQueue->dataLen-2-CARD_NO_LEN,CARD_NO_LEN);        
    }
    else
    {
        memcpy(key,pQueue->data+pQueue->dataLen-2-CARD_NO_LEN,CARD_NO_LEN);
    }
    
    log_d("key = %s\r\n",key);
    
    //��ȡ��¼
    value = ef_get_env((const char*)key);

    log_d("get env = %s\r\n",value);

    if(!value)
    {
        //δ�ҵ���¼����Ȩ��
        log_d("not find record\r\n");
        return 0;
    }

    split(value,";",buf,&num); //���ú������зָ� 
    log_d("num = %d\r\n",num);

    if(num != 5)
    {
        log_d("read record error\r\n");
        return 0;       
    }

    localUserData->authMode = pQueue->authMode;
    if(AUTH_MODE_QR == pQueue->authMode)
    {
        strcpy(localUserData->userId,key);
        
        strcpy(localUserData->cardNo,buf[0]);        
    }
    else
    {
        memcpy(localUserData->cardNo,key,CARD_NO_LEN);

        log_d("buf[0] = %s\r\n",buf[0]);
        strcpy(localUserData->userId,buf[0]);        
    }   

    //3867;0;0;2019-12-29;2029-12-31
    
    
    strcpy(localUserData->accessLayer,buf[1]);
    localUserData->defaultLayer = atoi(buf[2]);
    strcpy(localUserData->startTime,buf[3]);
    strcpy(localUserData->endTime,buf[4]);    



    log_d("localUserData->cardNo = %s\r\n",localUserData->cardNo);
    log_d("localUserData->userId = %s\r\n",localUserData->userId);
    log_d("localUserData->accessLayer = %s\r\n",localUserData->accessLayer);
    log_d("localUserData->defaultLayer = %d\r\n",localUserData->defaultLayer);    
    log_d("localUserData->startTime = %s\r\n",localUserData->startTime);        
    log_d("localUserData->endTime = %s\r\n",localUserData->endTime);        
    log_d("localUserData->authMode = %d\r\n",localUserData->authMode);



        return 1;
}










