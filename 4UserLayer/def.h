/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : def.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 头文件汇总
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __DEF_H
#define __DEF_H

/*
*********************************************************************************************************
*                                         标准库
*********************************************************************************************************
*/
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                         其它库
*********************************************************************************************************
*/
    //中间件头文件
#include "sys.h"
#include "delay.h"
#include "cmd_fun.h"
#include "ini.h"
#include "malloc.h"
#include "eth_cfg.h"


    
    //用户层头文件
#include "test.h"
#include "iap.h"
#include "tool.h"
#include "comm.h"
#include "version.h"

#include "mqtt_app.h"
#include "mqtt_send.h"
#include "rs485_process.h"


/*
*********************************************************************************************************
*                                           OS
*********************************************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"
//#include "FreeRTOS_CLI.h"



/*
*********************************************************************************************************
*                                        APP / BSP
*********************************************************************************************************
*/

#include  <bsp.h>





#endif

