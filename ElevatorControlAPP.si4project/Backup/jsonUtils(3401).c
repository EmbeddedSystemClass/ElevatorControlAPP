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
 �� �� ��  : GetJsonItem
 ��������  : ��ȡJSON�ַ�����ָ����Ŀ��ֵ
 �������  : const char *jsonBuff
             const char *item
 �������  : ��
 �� �� ֵ  : char

 �޸���ʷ      :
  1.��    ��   : 2019��12��20��
    ��    ��   : �Ŷ�
 char *GetJsonItem(const char *jsonBuff,const char *item)
    �޸�����   : �����ɺ���

*****************************************************************************/
char* GetJsonItem ( const char* jsonBuff,const char* item )
{
	static char value[JSON_ITEM_MAX_LEN] = {0};
	cJSON* root,*json_item;
	root = cJSON_Parse ( ( char* ) jsonBuff );    //�������ݰ�

	if ( !root )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
		return NULL;
	}
	else
	{
		json_item = cJSON_GetObjectItem ( root, item );
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
			return NULL;
		}

	}

	return value;
}

