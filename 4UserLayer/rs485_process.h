/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485_process.h
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
#ifndef __RS485_PROCESS_H_
#define __RS485_PROCESS_H_


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "errorcode.h"
#include "bsp_uart_fifo.h"
#include "tool.h"
#include "bsp_dipSwitch.h"
#include "easyflash.h"
#include "comm.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_CMD_LEN 5
#define MAX_SEND_LEN 37
#define QUEUE_BUF_LEN   64
#define CMD_STX     0x5A

#define AUTH_MODE_CARD  2
#define AUTH_MODE_QR    7

#pragma pack(1)
typedef struct
{
    uint8_t data[QUEUE_BUF_LEN];         //��Ҫ���͸�������������
    uint8_t authMode;                     //��Ȩģʽ,ˢ��=2��QR=7
    uint8_t dataLen;                     //���ݳ���    
}READER_BUFF_T;
#pragma pack()

extern READER_BUFF_T gReaderMsg;


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

void packetDefaultSendBuf(uint8_t *buf);
void packetSendBuf(READER_BUFF_T *pQueue,uint8_t *buf);

uint8_t authReader(READER_BUFF_T *pQueue,LOCAL_USER_T *localUserData);



#endif


