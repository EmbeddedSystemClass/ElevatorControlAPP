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
    
    return 

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
	static char value[JSON_ITEM_MAX_LEN] = {0};
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
			log_d ( "json item = :%s\r\n", json_item->valuestring );

			//�������
			if ( strlen ( json_item->valuestring ) > JSON_ITEM_MAX_LEN )
			{
				memcpy ( value, json_item->valuestring,JSON_ITEM_MAX_LEN );
			}

			strcpy ( value, json_item->valuestring );
			log_d ( "the cmd id =  %d\r\n",json_item->valuestring );
		}
		else if ( json_item->type == cJSON_Number )
		{
			log_d ( "json item = :%s\r\n", json_item->valueint );
			sprintf ( value,"%d",json_item->valueint );
			log_d ( "the cmd id =  %d\r\n",json_item->valuestring );
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


