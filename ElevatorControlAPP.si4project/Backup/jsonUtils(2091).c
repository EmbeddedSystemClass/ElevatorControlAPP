/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : jsonUtils.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��19��
  ����޸�   :
  ��������   : JSON���ݴ���C�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��19��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "jsonUtils.h"
#include "version.h"
/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define JSON_ITEM_MAX_LEN   512


/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : modifyJsonItem
 ��������  : �޸Ļ�������JSON���ݰ���ָ��item��ֵ
 �������  : const char *srcJson   json���ݰ�
             const char *item    ��Ҫ����ֵ��key 
             const char *value   ��Ҫ���µ�value 
             uint8_t isSubitem   =1 �� data������ 
             char *descJson      ���º��json���ݰ�
 �������  : ��
 �� �� ֵ  : 
 
 �޸���ʷ      :
  1.��    ��   : 2019��12��20��
    ��    ��   : �Ŷ�
SYSERRORCODE_E modifyJsonItem(const char *srcJson,const char *item,const char *value,uint8_t isSubitem,char *descJson);
    �޸�����   : �����ɺ���

*****************************************************************************/
SYSERRORCODE_E modifyJsonItem(const uint8_t *srcJson,const uint8_t *item,const uint8_t *value,uint8_t isSubitem,uint8_t *descJson)
{
    cJSON *root ,*dataObj;
    char *tmpBuf;

    if(!srcJson)
    {
        log_d("error json data\r\n");
        return STR_EMPTY_ERR;
    }    
    
    root = cJSON_Parse((char *)srcJson);    //�������ݰ�
    if (!root)  
    {  
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

    if(isSubitem == 1)
    {
        //����Э�飬Ĭ�����е�������data
        dataObj = cJSON_GetObjectItem ( root, "data" );         
        cJSON_AddStringToObject(dataObj,item,value);
    }
    else
    {
        cJSON_AddStringToObject(root,item,value);
    }  

    
    tmpBuf = cJSON_PrintUnformatted(root); 

    if(!tmpBuf)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descJson,tmpBuf);


    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);
    
    return NO_ERR;

}



/*****************************************************************************
 �� �� ��  : GetJsonItem
 ��������  : ��ȡJSON�ַ�����ָ����Ŀ��ֵ
 �������  : const char *jsonBuff  json�ַ���
           const char *item      Ҫ������KEY
           uint8_t isSubitem     �Ƿ��DATA�ڵ���Ŀ��=1 ��data����Ŀ��=0 ��root����Ŀ
 �������  : ��
 �� �� ֵ  : char *

 �޸���ʷ      :
  1.��    ��   : 2019��12��20��
    ��    ��   : �Ŷ�
 char *GetJsonItem(const char *jsonBuff,const char *item,uint8_t isSubitem)
    �޸�����   : �����ɺ���

*****************************************************************************/
uint8_t* GetJsonItem ( const uint8_t* jsonBuff,const uint8_t* item,uint8_t isSubitem)
{
	static uint8_t value[JSON_ITEM_MAX_LEN] = {0};
	cJSON* root,*json_item,*dataObj;
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�

	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
		return NULL;
	}
	else
	{
        if(isSubitem == 1)
        {
            //����Э�飬Ĭ�����е�������data
            dataObj = cJSON_GetObjectItem ( root, "data" );  
            json_item = cJSON_GetObjectItem ( dataObj, item );
        }
        else
        {
            json_item = cJSON_GetObjectItem ( root, item );
        }  
		
		if ( json_item->type == cJSON_String )
		{
			//�������
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}

			strcpy ( value, json_item->valuestring );
			log_d ( "json_item =  %s\r\n",json_item->valuestring );
		}
		else if ( json_item->type == cJSON_Number )
		{
			log_d ( "json_item = :%d\r\n", json_item->valueint );
			sprintf ( value,"%d",json_item->valueint );
			log_d ( "json_item =  %s\r\n",value);
		}
		else
		{
			log_d ( "can't parse json buff\r\n" );
            cJSON_Delete(root);
			return NULL;
		}

	}

    cJSON_Delete(root);
	return value;
}

SYSERRORCODE_E PacketDeviceInfo ( const uint8_t* jsonBuff,const uint8_t* descJson)
{
	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*newroot,*json_item,*dataObj,*json_cmdid,*json_devcode,*identification;
    char *tmpBuf;
    
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�
	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
        cJSON_Delete(root);
		return CJSON_PARSE_ERR;
	}
	else
	{
        json_cmdid = cJSON_GetObjectItem ( root, "commandCode" );
        json_devcode = cJSON_GetObjectItem ( root, "deviceCode" );
        identification = cJSON_GetObjectItem ( root, "data" );

        newroot = cJSON_CreateObject();
        dataObj = cJSON_CreateObject();
        if(!newroot && !dataObj)
        {
            log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
            cJSON_Delete(root);
            cJSON_Delete(newroot);
    		return CJSON_CREATE_ERR;
        }

        cJSON_AddStringToObject(newroot, "commandCode", json_cmdid->valuestring);
        cJSON_AddStringToObject(newroot, "deviceCode", json_devcode->valuestring);
        cJSON_AddStringToObject(newroot, "identification", identification->valuestring);

        cJSON_AddItemToObject(newroot, "data", dataObj);

        cJSON_AddStringToObject(dataObj, "version", gDevinfo.SoftwareVersion);
        cJSON_AddStringToObject(dataObj, "appName", gDevinfo.Model);
        cJSON_AddStringToObject(dataObj, "regRersion", "8");
        cJSON_AddStringToObject(dataObj, "regface", "7");
        cJSON_AddStringToObject(dataObj, "ip", gDevinfo.GetIP());
                
        tmpBuf = cJSON_PrintUnformatted(newroot); 

        if(!tmpBuf)
        {
            log_d("cJSON_PrintUnformatted error \r\n");
            return CJSON_FORMAT_ERR;
        }    

        strcpy((char *)descJson,tmpBuf);

	}

    cJSON_Delete(root);

    cJSON_Delete(newroot);

    my_free(tmpBuf);

    return result;
}

SYSERRORCODE_E upgradeDataPacket(uint8_t *descBuf)
{
    SYSERRORCODE_E result = NO_ERR;
    unsigned char payload_out[1024] = {0};
    char *up_status;
    cJSON *root ,*dataObj;

    char *tmpBuf;
    
    root = cJSON_Parse((char *)ef_get_env("upData"));    //�������ݰ�
    if (!root)  
    {  
        log_d("Error before: [%s]\r\n",cJSON_GetErrorPtr());  
        return CJSON_PARSE_ERR;
    } 

     cJSON_AddStringToObject(root,"commandCode","1017");

    //��ȡdataobject
    dataObj = cJSON_GetObjectItem( root , "data" ); 

    up_status = ef_get_env("up_status");

    //����ʧ��
    if(memcmp(up_status,"101700",6) == 0)
    {
        cJSON_AddStringToObject(dataObj,"status","2");
    }
    else if(memcmp(up_status,"101711",6) == 0) //�����ɹ�
    {
        cJSON_AddStringToObject(dataObj,"status","1");
    }
    else if(memcmp(up_status,"101722",6) == 0) //����ʧ��
    {
        cJSON_AddStringToObject(dataObj,"status","2"); 
    }
    else if(memcmp(up_status,"101733",6) == 0) //��ֹ����
    {
        cJSON_AddStringToObject(dataObj,"status","3");
    }
    else
    {
        //����������
    }

    tmpBuf = cJSON_PrintUnformatted(root); 

    if(tmpBuf == NULL)
    {
        log_d("cJSON_PrintUnformatted error \r\n");
        return CJSON_FORMAT_ERR;
    }    

    strcpy((char *)descBuf,tmpBuf);


    log_d("send json data = %s\r\n",tmpBuf);

    cJSON_Delete(root);

    my_free(tmpBuf);

    return result;
}



