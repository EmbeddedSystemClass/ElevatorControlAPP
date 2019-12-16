/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��7��9��
  ����޸�   :
  ��������   : ������ģ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��7��9��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "def.h"
#include "mqtt_app.h"

#define LOG_TAG    "main"
#include "elog.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
//�������ȼ�   



#define LED_TASK_PRIO	    ( tskIDLE_PRIORITY)
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define READER_TASK_PRIO	( tskIDLE_PRIORITY + 1)
#define QR_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 2)
#define DISPLAY_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define MQTT_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)
#define CMD_TASK_PRIO		( tskIDLE_PRIORITY + 4)
#define START_TASK_PRIO		( tskIDLE_PRIORITY + 5)




#define LED_STK_SIZE 		(1024)
#define CMD_STK_SIZE 		(1024*1)
#define START_STK_SIZE 	    (512)
#define QR_STK_SIZE 		(1024)
#define READER_STK_SIZE     (1024)
#define HANDSHAKE_STK_SIZE  (1024)
#define KEY_STK_SIZE        (1024)
#define MQTT_STK_SIZE        (1024)
#define DISPLAY_STK_SIZE        (1024)





//�¼���־
#define TASK_BIT_0	 (1 << 0)
#define TASK_BIT_1	 (1 << 1)
#define TASK_BIT_2	 (1 << 2)
#define TASK_BIT_3	 (1 << 3)
#define TASK_BIT_4	 (1 << 4)
#define TASK_BIT_5	 (1 << 5)
#define TASK_BIT_6	 (1 << 6)
#define TASK_BIT_7	 (1 << 7)
#define TASK_BIT_8	 (1 << 8)

//��ȡ���״̬������
#define READ_MOTOR_STATUS_TIMES 20


#define TASK_BIT_ALL ( TASK_BIT_0 | TASK_BIT_1 | TASK_BIT_2 |TASK_BIT_3|TASK_BIT_4|TASK_BIT_5|TASK_BIT_6|TASK_BIT_7|TASK_BIT_8)

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
//������
static TaskHandle_t xHandleTaskLed = NULL;      //LED��
static TaskHandle_t xHandleTaskCmd = NULL;      //androidͨѶ
static TaskHandle_t xHandleTaskReader = NULL;   //Τ��������
static TaskHandle_t xHandleTaskQr = NULL;       //��ά���ͷ
static TaskHandle_t xHandleTaskStart = NULL;    //���Ź�
static TaskHandle_t xHandleTaskHandShake = NULL;    // ����
static TaskHandle_t xHandleTaskKey = NULL;      //B�Ű���
static TaskHandle_t xHandleTaskMqtt = NULL;      //MQTT ����
static TaskHandle_t xHandleTaskDisplay = NULL;      //�����



//�¼����
static EventGroupHandle_t xCreatedEventGroup = NULL;


//������Ϊ�ѻ�ģʽ����£����⣬����������ά��������
//�ѻ�ģʽ���ж��������ı��뷶Χ���Լ���ά��ļ������
#ifdef USEQUEUE
#define MONITOR_TASK_PRIO	( tskIDLE_PRIORITY + 4)
#define MONITOR_STK_SIZE   (1024)
static TaskHandle_t xHandleTaskMonitor = NULL;    //�������
static void vTaskMonitor(void *pvParameters);
static QueueHandle_t xTransQueue = NULL;
#endif





/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

//������
static void vTaskLed(void *pvParameters);
static void vTaskKey(void *pvParameters);
static void vTaskReader(void *pvParameters);
static void vTaskQR(void *pvParameters);
static void vTaskStart(void *pvParameters);
//���Ϳ�������
static void vTaskHandShake(void *pvParameters);
static void vTaskMqttTest(void *pvParameters);
static void vTaskDisplay(void *pvParameters);




static void AppTaskCreate(void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void EasyLogInit(void);




int main(void)
{   
    //Ӳ����ʼ��
    bsp_Init();  

    EasyLogInit();  
    
	mymem_init(SRAMIN);								//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);								//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM);	  					    //��ʼ��CCM�ڴ��

	while(lwip_comm_init() != 0) //lwip��ʼ��
	{
        log_d("lwip init error!\r\n");
		delay_ms(1200);
	}

    log_d("lwip init success!\r\n");

	/* ��������ͨ�Ż��� */
	AppObjCreate();

	/* �������� */
	AppTaskCreate();
    
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();
    
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{

#if LWIP_DHCP
        lwip_comm_dhcp_creat();                             //����DHCP����
#endif


    //��android����
    xTaskCreate((TaskFunction_t )vTaskHandShake,
                (const char*    )"vHandShake",       
                (uint16_t       )HANDSHAKE_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskHandShake);  

    //����LED����
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )"vTaskLed",       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);                   

    //�����
    xTaskCreate((TaskFunction_t )vTaskDisplay,
                (const char*    )"vTaskDisplay",       
                (uint16_t       )DISPLAY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DISPLAY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDisplay);  
    
    //���������Ϣ��������
//    xTaskCreate((TaskFunction_t )vTaskMortorToHost,     
//                (const char*    )"vTMTHost",   
//                (uint16_t       )MOTOR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )MOTOR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskMotor);     

    //��androidͨѶ�������ݽ���
//    xTaskCreate((TaskFunction_t )vTaskMsgPro,     
//                (const char*    )"cmd",   
//                (uint16_t       )CMD_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )CMD_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskCmd);      

    //���⴫����״̬����
    xTaskCreate((TaskFunction_t )vTaskMqttTest,     
                (const char*    )"vMqttTest",   
                (uint16_t       )MQTT_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MQTT_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMqtt);    


    //ȫ���ŵ��״̬����
//    xTaskCreate((TaskFunction_t )vTaskRs485,     
//                (const char*    )"vRs485",   
//                (uint16_t       )RS485_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )RS485_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskRs485);  

    //Τ��������
//    xTaskCreate((TaskFunction_t )vTaskReader,     
//                (const char*    )"vReader",   
//                (uint16_t       )READER_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )READER_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskReader);    

    //��ά��ɨ��ģ��
//    xTaskCreate((TaskFunction_t )vTaskQR,     
//                (const char*    )"vTaskQR",   
//                (uint16_t       )QR_STK_SIZE, 
//                (void*          )NULL,
//                (UBaseType_t    )QR_TASK_PRIO,
//                (TaskHandle_t*  )&xHandleTaskQr);      

    //B�Ű���
    xTaskCreate((TaskFunction_t )vTaskKey,         
                (const char*    )"vTaskKey",       
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )KEY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskKey);   

    #ifdef USEQUEUE
    //����߳�
    xTaskCreate((TaskFunction_t )vTaskMonitor,     
                (const char*    )"vTaskMonitor",   
                (uint16_t       )MONITOR_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )MONITOR_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskMonitor);
    #endif

    //���Ź�
//	xTaskCreate((TaskFunction_t )vTaskStart,     		/* ������  */
//                (const char*    )"vTaskStart",   		/* ������    */
//                (uint16_t       )START_STK_SIZE,        /* ����ջ��С����λword��Ҳ����4�ֽ� */
//                (void*          )NULL,           		/* �������  */
//                (UBaseType_t    )START_TASK_PRIO,       /* �������ȼ�*/
//                (TaskHandle_t*  )&xHandleTaskStart );   /* ������  */                

}


/*
*********************************************************************************************************
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	/* �����¼���־�� */
	xCreatedEventGroup = xEventGroupCreate();
	
	if(xCreatedEventGroup == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("�����¼���־��ʧ��\r\n");
    }

	/* ���������ź��� */
    gxMutex = xSemaphoreCreateMutex();
	
	if(gxMutex == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
        App_Printf("���������ź���ʧ��\r\n");
    }    

    //������ֵ�ź���
//    gBinarySem_Handle = xSemaphoreCreateBinary();

//    if(gBinarySem_Handle == NULL)
//    {
//        App_Printf("������ֵ�ź���ʧ��\r\n");
//    }

    #ifdef USEQUEUE
    /* ������Ϣ���� */
    xTransQueue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* ��Ϣ���еĳ��� */
                              (UBaseType_t ) sizeof(QUEUE_TO_HOST_T *));/* ��Ϣ�Ĵ�С */
    if(xTransQueue == NULL)
    {
        App_Printf("����xTransQueue��Ϣ����ʧ��!\r\n");
    }	
    #endif

}



/*
*********************************************************************************************************
*	�� �� ��: vTaskStart
*	����˵��: �������񣬵ȴ����������¼���־������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* ����ӳ�1000ms */   
    
	/* 
	  ��ʼִ����������������ǰʹ�ܶ������Ź���
	  ����LSI��32��Ƶ�����溯��������Χ0-0xFFF���ֱ������Сֵ1ms�����ֵ4095ms
	  �������õ���4s�����4s��û��ι����ϵͳ��λ��
	*/
	bsp_InitIwdg(4000);
	
	/* ��ӡϵͳ����״̬������鿴ϵͳ�Ƿ�λ */
	App_Printf("=====================================================\r\n");
	App_Printf("ϵͳ����ִ��\r\n");
	App_Printf("=====================================================\r\n");
	
    while(1)
    {   
        
		/* �ȴ������������¼���־ */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* �¼���־���� */
							         TASK_BIT_ALL,       /* �ȴ�TASK_BIT_ALL������ */
							         pdTRUE,             /* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
							         pdTRUE,             /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
							         xTicksToWait); 	 /* �ȴ��ӳ�ʱ�� */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{  
		    IWDG_Feed(); //ι��			
		}
	    else
		{
			/* ������ÿxTicksToWait����һ�� */
			/* ͨ������uxBits�򵥵Ŀ����ڴ˴�����Ǹ�������û�з������б�־ */

            //ʱ��ԭ��ֵ��̫׼ȷ����Ҫ����׼�ķ���

//            if((uxBits & TASK_BIT_0) != 0x01)
//            {
//                DBG("BIT_0 vTaskLed error = %02x,%02x   %02x \r\n",(uxBits & TASK_BIT_0),uxBits,TASK_BIT_0);
//            }

//            if((uxBits & TASK_BIT_1) != 0x02)
//            {
//                DBG("BIT_1 vTaskMotorToHost error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_1),uxBits,TASK_BIT_1);
//            }

//            if((uxBits & TASK_BIT_2) != 0x04)
//            {
//                DBG("BIT_2 vTaskMsgPro error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_2),uxBits,TASK_BIT_2);
//            }
//            
//            if((uxBits & TASK_BIT_3) != 0x08)
//            {
//                DBG("BIT_3 vTaskInfrared error = %02x, %02x   %02x \r\n",(uxBits & TASK_BIT_3),uxBits,TASK_BIT_3);
//            }

//            if((uxBits & TASK_BIT_4) != 0x10)
//            {
//                DBG("BIT_4 vTaskReader error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_4),uxBits,TASK_BIT_4);
//            }

//            if((uxBits & TASK_BIT_5) != 0x20)
//            {
//                DBG("BIT_5 vTaskQR error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_5),uxBits,TASK_BIT_5);
//            }       

//            if((uxBits & TASK_BIT_6) != 0x40)
//            {
//                DBG("BIT_6 vTaskRs485 error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_6),uxBits,TASK_BIT_6);
//            }  

//            if((uxBits & TASK_BIT_7) != 0x80)
//            {
//                DBG("BIT_7 vTaskKey error = %02x,%02x   ,%02x \r\n",(uxBits & TASK_BIT_7),uxBits,TASK_BIT_7);
//            } 

//            if((uxBits & (TASK_BIT_8>>8)) != 0x01)
//            {
//                DBG("BIT_8 vTaskQueryMotor error = %04x,%02x   ,%02x \r\n",(uxBits & (TASK_BIT_8>>8)),uxBits,TASK_BIT_8);
//            } 

            
		}
    }
}




//LED������ 
void vTaskLed(void *pvParameters)
{       
    uint8_t i = 0;
    BEEP = 1;
    vTaskDelay(300);
    BEEP = 0;
    
    while(1)
    {  
        if(i == 250)
        {
            i = 100;
        }
        i+=20;
        bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 100, ((i) * 10000) /255);

        LED1=!LED1;  
        LED2=!LED2; 
        LED3=!LED3; 
        LED4=!LED4; 
        

        
		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
        vTaskDelay(100);     
    }
}   

void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    uint16_t crc_value = 0;

    uint8_t cm4[] = { 0x02,0x7B,0x22,0x63,0x6D,0x64,0x22,0x3A,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x2C,0x22,0x76,0x61,0x6C,0x75,0x65,0x22,0x3A,0x7B,0x22,0x75,0x70,0x64,0x61,0x74,0x65,0x22,0x3A,0x22,0x41,0x37,0x22,0x7D,0x2C,0x22,0x64,0x61,0x74,0x61,0x22,0x3A,0x22,0x30,0x30,0x22,0x7D,0x03 };

    
    
    while(1)
    {
        ucKeyCode = bsp_Key_Scan(0);      
		
		if (ucKeyCode != KEY_NONE)
		{
            //dbg("ucKeyCode = %d\r\n",ucKeyCode);
              
			switch (ucKeyCode)
			{
				/* K1������ ��ӡ����ִ����� */
				case KEY_SET_PRES:	             
					App_Printf("=================================================\r\n");
					App_Printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
					vTaskList((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);
                    
					App_Printf("\r\n������       ���м���         ʹ����\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					App_Printf("%s\r\n", pcWriteBuffer);                    
					break;				
				/* K2�����£���ӡ���ڲ������� */
				case KEY_RR_PRES:
                
                    log_a("KEY_DOWN_K2\r\n");

			
					break;
				case KEY_LL_PRES:

                    log_i("KEY_DOWN_K3\r\n");  
					break;
				case KEY_OK_PRES:    
       
                    log_w("KEY_DOWN_K4\r\n");
                    crc_value = CRC16_Modbus(cm4, 54);
                    log_v("hi = %02x, lo = %02x\r\n", crc_value>>8, crc_value & 0xff);

					break;                
				
				/* �����ļ�ֵ������ */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* �����¼���־����ʾ������������ */
//		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);
	}   

}


void vTaskMqttTest(void *pvParameters)
{


    mqtt_thread();

    while(1)
    {
        vTaskDelay(500);        
    }

}


static void vTaskDisplay(void *pvParameters)
{
    while(1)
    {
        bsp_HC595Show(1,2,3);
        vTaskDelay(1000);
        bsp_HC595Show(4,5,6);
        vTaskDelay(1000);
        bsp_HC595Show(7,8,9);
        vTaskDelay(1000);
        bsp_HC595Show('a','b','c');
        vTaskDelay(1000);
        bsp_HC595Show('d','e','f');     
        vTaskDelay(1000);
        bsp_HC595Show('a',0,1);
        vTaskDelay(1000);
        bsp_HC595Show('d',3,4);   
        vTaskDelay(1000);
        bsp_HC595Show(1,0,1);  
        vTaskDelay(1000);
    }  

}



void vTaskReader(void *pvParameters)
{ 
    uint32_t CardID = 0;
    uint8_t dat[4] = {0};
    
//    uint32_t FunState = 0;
//    char *IcReaderState;
    #ifdef USEQUEUE
    QUEUE_TO_HOST_T *ptReaderToHost; 
    ptReaderToHost = &gQueueToHost;
    #endif
//    IcReaderState = ef_get_env("ICSTATE");
//    assert_param(IcReaderState);
//    FunState = atol(IcReaderState);
    
    while(1)
    {

//        if(FunState != 0x00)
        {
            CardID = bsp_WeiGenScanf();

            if(CardID != 0)
            {
                memset(dat,0x00,sizeof(dat));            
                
    			dat[0] = CardID>>24;
    			dat[1] = CardID>>16;
    			dat[2] = CardID>>8;
    			dat[3] = CardID&0XFF;    

                #ifdef USEQUEUE
                ptReaderToHost->cmd = WGREADER;
                memcpy(ptReaderToHost->data,dat,4);

    			/* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    			if(xQueueSend(xTransQueue,              /* ��Ϣ���о�� */
    						 (void *) &ptReaderToHost,   /* ���ͽṹ��ָ�����ptQueueToHost�ĵ�ַ */
    						 (TickType_t)10) != pdPASS )
    			{
                    DBG("��xTransQueue��������ʧ�ܣ���ʹ�ȴ���10��ʱ�ӽ���\r\n");                
                } 
                else
                {
                    dbh("WGREADER",(char *)dat,4);
                }
                #endif
                
                send_to_host(WGREADER,dat,4);
            }  
        }


		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);        
        
        vTaskDelay(100);
        
    }

}   


void vTaskQR(void *pvParameters)
{ 
    uint8_t recv_buf[256] = {0};
    uint16_t len = 0;  
    

    
    while(1)
    {   
           memset(recv_buf,0x00,sizeof(recv_buf));
           len = comRecvBuff(COM3,recv_buf,sizeof(recv_buf));

           if(len > 0  && recv_buf[len-1] == 0x0A && recv_buf[len-2] == 0x0D)
           {
                DBG("QR = %s\r\n",recv_buf);
                SendAsciiCodeToHost(QRREADER,NO_ERR,recv_buf);
           }
    

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_5);  
        vTaskDelay(500);        
    }
}   


void vTaskHandShake(void *pvParameters)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[12] = {0};
    uint8_t bcdbuf[6] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    assert_param(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);
    
    /* boot count +1 */
    i_boot_times ++;

    /* interger to string */
    sprintf(c_new_boot_times,"%012ld", i_boot_times);
    
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);    

    asc2bcd(bcdbuf,(uint8_t *)c_new_boot_times , 12, 0);

    send_to_host(HANDSHAKE,bcdbuf,6);  
    
    vTaskDelete( NULL ); //ɾ���Լ�


//    uint32_t i_boot_times = NULL;
//    char *c_old_boot_times, c_new_boot_times[12] = {0};
//    

//    g500usCount = 1000*10;

//    c_old_boot_times = ef_get_env("boot_times");

//    DBG("1.c_old_boot_times = %s\r\n",c_old_boot_times);

//    while(1)
//    {
//        if(g500usCount == 0)
//        {
//            break;
//        }
//        /* get the boot count number from Env */
//        c_old_boot_times = ef_get_env("boot_times");
//        assert_param(c_old_boot_times);
//        i_boot_times = atol(c_old_boot_times);
//        
//        /* boot count +1 */
//        i_boot_times ++;

//        /* interger to string */
//        sprintf(c_new_boot_times,"%012ld", i_boot_times);

//        /* set and store the boot count number to Env */
//        ef_set_env("boot_times", c_new_boot_times);   


//    }

//    c_old_boot_times = ef_get_env("boot_times");

//    DBG("2.c_old_boot_times = %s\r\n",c_old_boot_times);    

//    vTaskDelete( NULL ); //ɾ���Լ�
}



/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
    char  buf_str[512 + 1];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* �����ź��� */
	xSemaphoreTake(gxMutex, portMAX_DELAY);

    printf("%s", buf_str);

   	xSemaphoreGive(gxMutex);
}



#ifdef USEQUEUE
static void vTaskMonitor(void *pvParameters)
{
  BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdTRUE */
  QUEUE_TO_HOST_T *ptMsg;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200); /* �������ȴ�ʱ��Ϊ200ms */  

  while (1)
  {
    xReturn = xQueueReceive( xTransQueue,    /* ��Ϣ���еľ�� */
                             (void *)&ptMsg,  /*�����ȡ���ǽṹ��ĵ�ַ */
                             xMaxBlockTime); /* ��������ʱ�� */
    if(pdPASS == xReturn)
    {
//        DBG("ptMsg->cmd = %02x\r\n", ptMsg->cmd);
//        dbh("ptMsg->data ", (char *)ptMsg->data,QUEUE_BUF_LEN);

        switch (ptMsg->cmd)
        {
            case GETSENSOR:
                 DBG("��������\r\n");
                break;
             case CONTROLMOTOR:
                 DBG("A�ŵ������\r\n");
                break;
            case DOOR_B:
                 DBG("B�ŵ������\r\n");
                break;
            case WGREADER:
                 DBG("����������\r\n");
                break;            
            
        }
    }    

  }    
}
#endif


static void EasyLogInit(void)
{
    /* initialize EasyLogger */
     elog_init();
     /* set EasyLogger log format */
     elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
     elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG );
     elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_TIME);
     elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_TIME);

     
     /* start EasyLogger */
     elog_start();  
}



