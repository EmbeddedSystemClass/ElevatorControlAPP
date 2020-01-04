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
#include "easyflash.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "version.h"


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
QueueHandle_t xTransQueue = NULL; 
int gConnectStatus = 0;
int	gMySock = 0;
READER_BUFF_T gReaderMsg;

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode);

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
static SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf ); //��ȡģ�����
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf ); //��ȡ�û���Ϣ
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf ); //Զ�̺���
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf ); //ɾ���û���Ϣ
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf ); //��ӵ����û�
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf ); //�����



static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ); //����Ĭ����Ϣ


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
	{"1015", AddSingleUser},
	{"1016", UpgradeDev},
	{"1017", UpgradeAck},
	{"1021", EnableDev},
	{"1023", SetDevParam},
	{"1024", SetJudgeMode},
	{"1026", GetDevInfo},
    {"3002", GetServerIp},
    {"3003", GetTemplateParam},
    {"3004", GetUserInfo},   
    {"3005", RemoteOptDev},        
    {"3006", ClearUserInof},   
    {"3009", UnbindDev},          
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

    
    ReturnDefault(msg_buf);
	return result;
}


//�����Ϊ�˷������˵��ԣ���д��Ĭ�Ϸ��صĺ���
static SYSERRORCODE_E ReturnDefault ( uint8_t* msgBuf ) //����Ĭ����Ϣ
{
        SYSERRORCODE_E result = NO_ERR;
        uint8_t buf[MQTT_MAX_LEN] = {0};
        uint16_t len = 0;
    
        if(!msgBuf)
        {
            return STR_EMPTY_ERR;
        }
    
        result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);      
        result = modifyJsonItem(packetBaseJson(buf),"UnknownCommand","random return",1,buf);   
    
        if(result != NO_ERR)
        {
            return result;
        }
    
        len = strlen((const char*)buf);
    
        log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);
    
        PublishData(buf,len);
        
        return result;

}


SYSERRORCODE_E OpenDoor ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

//    result = modifyJsonItem(msgBuf,"openStatus","1",1,buf);
    result = modifyJsonItem(packetBaseJson(msgBuf),"openStatus","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
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
    uint8_t tmpUrl[256] = {0};
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.����URL
    strcpy((char *)tmpUrl,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"softwareUrl",1));
    log_d("tmpUrl = %s\r\n",tmpUrl);
    
    ef_set_env("url", (const char*)GetJsonItem((const uint8_t *)tmpUrl,(const uint8_t *)"picUrl",0)); 

    //2.��������״̬Ϊ������״̬
    ef_set_env("up_status", "101700");
    
    //3.��������JSON����
    ef_set_env("upData", (const char*)msgBuf);
    
    //4.���ñ�־λ������
    SystemUpdate();
    
	return result;

}

SYSERRORCODE_E UpgradeAck ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    //��ȡ�������ݲ�����JSON��   

    result = upgradeDataPacket(buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("OpenDoor len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
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
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t *identification;
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = PacketDeviceInfo(msgBuf,buf);


    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetDevInfo len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
    my_free(identification);
    
	return result;

}



int PublishData(uint8_t *payload_out,uint16_t payload_out_len)
{
    
    #define DEVICE_PUBLISH		"/smartCloud/server/msg/device"		
    
	MQTTString topicString = MQTTString_initializer;
    
	uint32_t len = 0;
	int32_t rc = 0;
	unsigned char buf[MQTT_MAX_LEN];
	int buflen = sizeof(buf);

	unsigned short msgid = 1;
	int req_qos = 0;
	unsigned char retained = 0;  

    if(!payload_out)
    {
        return STR_EMPTY_ERR;
    }

    log_d("payload_out = %s,payload_out_len = %d\r\n",payload_out,payload_out_len);

   if(gConnectStatus == 1)
   { 
       topicString.cstring = DEVICE_PUBLISH;       //�����ϱ� ����       

       len = MQTTSerialize_publish((unsigned char*)buf, buflen, 0, req_qos, retained, msgid, topicString, payload_out, payload_out_len);//������Ϣ
       rc = transport_sendPacketBuffer(gMySock, (unsigned char*)buf, len);
       if(rc == len)                                                           //
           log_d("send PUBLISH Successfully\r\n");
       else
           log_d("send PUBLISH failed\r\n");     
      
   }
   else
   {
        log_d("MQTT Lost the connect!!!\r\n");
   }

   return len;
}


SYSERRORCODE_E GetTemplateParam ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }
    
    result = modifyJsonItem(packetBaseJson(msgBuf),"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("GetParam len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
	return result;
}

//�������IP
static SYSERRORCODE_E GetServerIp ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t ip[32] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.����IP     
    strcpy((char *)ip,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"ip",1));
    log_d("server ip = %s\r\n",ip[0]);

    //Ӱ�������
    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status",(const uint8_t *)"1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);
    
	return result;

}

//��ȡ�û���Ϣ
static SYSERRORCODE_E GetUserInfo ( uint8_t* msgBuf )
{
	SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t userID[16] = {0};
    uint8_t cardID[16] = {0};
    uint8_t value[255] = {0};
    uint8_t cardIDvalue[255] = {0};
    uint8_t tmp[128] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.������userIDΪkey�ı�
    memset(userID,0x00,sizeof(userID));
    strcpy((char *)userID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    log_d("userId = %s\r\n",userID);
    
    //1.1 ������card id Ϊkey�ı�
    memset(cardIDvalue,0x00,sizeof(cardIDvalue));    
    strcpy((char *)cardIDvalue,(const char*)userID);
    

    //2.���濨��
    memset(value,0x00,sizeof(value));   
    strcpy((char *)cardID,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    strcpy((char *)value,(const char*)cardID);
    log_d("cardNo = %s\r\n",value);
    
    

    //3.����¥��Ȩ��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));
    
    strcat((char *)value,(const char *)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp);    
    log_d("accessLayer = %s\r\n",value);

    //4.����Ĭ��¥��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp);      
    log_d("defaultLayer = %s\r\n",value);

    //5.���濪ʼʱ��
    memset(tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp); 
    log_d("startTime = %s\r\n",value);

    //6.�������ʱ��
    memset((char *)tmp,0x00,sizeof(tmp));
    strcpy((char *)tmp,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));
    
    strcat((char *)value,(const char*)";");
    strcat((char *)value,(const char*)tmp);

    strcat((char *)cardIDvalue,(const char*)";");
    strcat((char *)cardIDvalue,(const char*)tmp); 
    log_d("endTime = %s\r\n",value);    

    //д��¼
    if((ef_set_env((const char*)userID, (const char*)value) == EF_NO_ERR) && (ef_set_env((const char*)cardID, (const char*)cardIDvalue) == EF_NO_ERR))
    {        
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","success",1,buf);
    }
    else
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add record error",1,buf);
    }
    

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    PublishData(buf,len);
    
	return result;

}

//Զ�̺���
static SYSERRORCODE_E RemoteOptDev ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t accessLayer[4] = {0};
    uint16_t len = 0;
    
    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    
    strcpy((char *)accessLayer,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }
    

    //������Ҫ����Ϣ����Ϣ���У����к���
    SendToQueue(accessLayer,strlen((const char*)accessLayer),AUTH_MODE_REMOTE);

    len = strlen((const char*)buf);

    log_d("RemoteOptDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len); 
    
    return result;

}


//ɾ���û���Ϣ
static SYSERRORCODE_E ClearUserInof ( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    len = strlen((const char*)buf);

    log_d("ClearUserInof len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);


    //����û���Ϣ
    // ef_env_set_default();

    
    return result;

}

//��ӵ����û�
static SYSERRORCODE_E AddSingleUser( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t userID[16] = {0};
    uint8_t cardID[16] = {0};
    uint8_t value[255] = {0};
    uint8_t cardIDvalue[255] = {0};
    uint16_t len = 0;
    

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    //1.������userIDΪkey�ı�
    memset(userID,0x00,sizeof(userID));
    strcpy((char *)userID,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"userId",1));
    log_d("userId = %s\r\n",userID);

    //2.���濨��
    memset(value,0x00,sizeof(value));   
    strcpy((char *)cardID,(const char *)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"cardNo",1));
    if(strlen((char *)cardID) == 0)
    {
        strcat((char *)value,"00000000");
    }
    else
    {
        strcat((char *)value,(const char*)cardID);
    }
    log_d("cardNo = %s\r\n",cardID); 
    
    //3.����¥��Ȩ��
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"accessLayer",1));    
    log_d("accessLayer = %s\r\n",value);

    //4.����Ĭ��¥��
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"defaultLayer",1));    
    log_d("accessLayer = %s\r\n",value);

    //5.���濪ʼʱ��
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"startTime",1));    
    log_d("accessLayer = %s\r\n",value);    

    //6.�������ʱ��
    strcat((char *)value,(const char *)";");
    strcat((char *)value,  (const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"endTime",1));    
    log_d("accessLayer = %s\r\n",value);    

    //д��¼
    if(ef_set_env((const char*)userID, (const char*)value) == EF_NO_ERR)
    {        
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","1",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"commandCode","3004",1,buf);
    }
    else
    {
        //Ӱ�������
        result = modifyJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"status","0",1,buf);
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"commandCode","3004",1,buf);        
        result = modifyJsonItem((const uint8_t *)buf,(const uint8_t *)"reason","add record error",1,buf);
    }    

    if(result != NO_ERR)
    {
        return result;
    }
    
    len = strlen((const char*)buf);

    log_d("AddSingleUser len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);
    
    return result;

}

//�����
static SYSERRORCODE_E UnbindDev( uint8_t* msgBuf )
{
    SYSERRORCODE_E result = NO_ERR;
    uint8_t buf[MQTT_MAX_LEN] = {0};
    uint8_t type[4] = {0};
    uint16_t len = 0;

    if(!msgBuf)
    {
        return STR_EMPTY_ERR;
    }

    strcpy((char *)type,(const char*)GetJsonItem((const uint8_t *)msgBuf,(const uint8_t *)"type",1));

    result = modifyJsonItem(msgBuf,"status","1",1,buf);

    if(result != NO_ERR)
    {
        return result;
    }

    //������Ҫ����Ϣ����Ϣ���У������

    if(memcmp(type,"0",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_UNBIND);
    }
    else if(memcmp(type,"1",1) == 0)
    {
        SendToQueue(type,strlen((const char*)type),AUTH_MODE_BIND);
    } 
    
    len = strlen((const char*)buf);

    log_d("UnbindDev len = %d,buf = %s\r\n",len,buf);

    PublishData(buf,len);

    return result;

}

static SYSERRORCODE_E SendToQueue(uint8_t *buf,int len,uint8_t authMode)
{
    SYSERRORCODE_E result = NO_ERR;

    READER_BUFF_T *ptQR; 
    /* ��ʼ���ṹ��ָ�� */
    ptQR = &gReaderMsg;

	/* ���� */
    ptQR->authMode = authMode; 
    ptQR->dataLen = 0;
    memset(ptQR->data,0x00,sizeof(ptQR->data)); 

    ptQR->dataLen = len;                
    memcpy(ptQR->data,buf,len);
    
    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
                 (void *) &ptQR,   /* ����ָ�����recv_buf�ĵ�ַ */
                 (TickType_t)50) != pdPASS )
    {
        DBG("the queue is full!\r\n");                
        xQueueReset(xTransQueue);
    } 
    else
    {
        dbh("QR",(char *)buf,len);
        log_d("buf = %s,len = %d\r\n",buf,len);
    } 


    return result;
}

