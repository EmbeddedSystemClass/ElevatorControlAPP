/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : comm.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��6��18��
  ����޸�   :
  ��������   : ��������ָ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��18��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "comm.h"
#include "tool.h"
#include "bsp_led.h"
#include "malloc.h"
#include "ini.h"
#include "bsp_uart_fifo.h"
#include "version.h"
#include "errorcode.h"

#define LOG_TAG    "comm"
#include "elog.h"



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define DIM(x)  (sizeof(x)/sizeof(x[0]))


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf ); //����
static SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf ); //Զ�̱���
static SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf ); //��ӿ���
static SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf ); //ɾ������
static SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf ); //���豸��������
static SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf ); //����Ӧ��
static SYSERRORCODE_E EnableDev ( uint8_t* msgBuf ); //�����豸
static SYSERRORCODE_E DisableDev ( uint8_t* msgBuf ); //�ر��豸
static SYSERRORCODE_E SetDevParam ( uint8_t* msgBuf ); //���ò���
static SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf ); //����ʶ��ģʽ
static SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf ); //��ȡ�豸��Ϣ

static char *GetCmdID ( uint8_t* msgBuf ); //��ȡ��ǰָ��



typedef SYSERRORCODE_E ( *cmd_fun ) ( uint8_t *msgBuf ); 

typedef struct
{
	const char* cmd_id;            /* ����id */
	cmd_fun  fun_ptr;     /* ����ָ�� */
} CMD_HANDLE_T;

CMD_HANDLE_T CmdList[] =
{
	{"201",  OpenDoor},
	{"1006", AbnormalAlarm},
	{"1012", AddCardNo},
	{"1013", DelCardNo},
	{"1016", UpgradeDev},
	{"1017", UpgradeAck},
	{"1021", EnableDev},
	{"1023", SetDevParam},
	{"1024", SetJudgeMode},
	{"1026", GetDevInfo}
};


SYSERRORCODE_E exec_proc ( char* cmd_id, uint8_t *msg_buf )
{

	SYSERRORCODE_E result = NO_ERR;
	int i = 0;

    if(cmd_id == NULL)
    {
        log_d("empty cmd \r\n");
        return CMD_EMPTY_ERR; 
    }

	for ( i = 0; i < DIM ( CmdList ); i++ )
	{
		if ( 0 == strcmp ( CmdList[i].cmd_id, cmd_id ) )
		{
			CmdList[i].fun_ptr ( msg_buf );
			return result;
		}
	}
	log_d ( "invalid id %s\n", cmd_id );

	return result;
}



SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;






    
	return result;
}

SYSERRORCODE_E AbnormalAlarm ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E AddCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E DelCardNo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E UpgradeDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E EnableDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E DisableDev ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E SetDevParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E SetJudgeMode ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}

SYSERRORCODE_E GetDevInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
	return result;
}






