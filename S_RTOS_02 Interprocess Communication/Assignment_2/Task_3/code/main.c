/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"

#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"
/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

#define LED_400_MS_PERIODOCITY 		400
#define LED_100_MS_PERIODOCITY 		100


#define BTN_ONE_TASK_DELAY 		30
#define BTN_TWO_TASK_DELAY 		30

#define PERIDIC_TASK_DELAY 		100
#define CONSUMER_TASK_DELAY 	20

#define TWO_SEC_DURATION  	2000
#define FOUR_SEC_DURATION  	4000

#define BUTTON_VALID_PRESSED    1
#define BUTTON_INVALID_PRESSED  0

#define STRING_MAX_SIZE		24
#define QUEUE_MAX_SIZE		10
#define NO_TICK_FOR_BLOCK_STATE  0

#define TRUE    1
#define FALSE   0

#define FIRST_ITERATION_INDEX 						0
#define MAX_NUM_ITERATION_FOR_STRING	 		10
#define MAX_NUM_ITERATION_FOR_LOAD_LOOP		100000
/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

TaskHandle_t BtnOneTask_Handler  	 = NULL;
TaskHandle_t BtnTwoTask_Handler  	 = NULL;

TaskHandle_t PeriodicCheckTask_Handler  	 = NULL;
TaskHandle_t ConsumerTask_Handler  	 			 = NULL;

QueueHandle_t xQueueMesseges 	 = NULL;
 
static pinState_t en_sg_btn_1_status= PIN_IS_LOW;
static pinState_t en_sg_btn_2_status= PIN_IS_LOW;

static uint8_t u8_sg_btn_1_IsPushed = FALSE;
static uint8_t u8_sg_btn_2_IsPushed = FALSE;

int8_t *chPtr_g_tempBuffer = NULL;


int8_t chArr_g_btn_1_pressedMsg[STRING_MAX_SIZE]= "\nBtn_1_Falling_Edge\n";
int8_t chArr_g_btn_1_releaseMsg[STRING_MAX_SIZE]= "\nBtn_1_Rising_Edge\n";
																									 
int8_t chArr_g_btn_2_pressedMsg[STRING_MAX_SIZE]= "\nBtn_2_Falling_Edge\n";
int8_t chArr_g_btn_2_releaseMsg[STRING_MAX_SIZE]= "\nBtn_2_Rising_Edge\n";

int8_t chArr_g_PeriodicTaskStr[STRING_MAX_SIZE] = "\nHello\n";
																	 
/*--------------------------Task_1----------------------------*/
void Button_One_Task(void* pvParameters)
{
	int8_t *chPtr_l_temp = NULL;
	for(;;)
	{	
		
		en_sg_btn_1_status = GPIO_read(PORT_0,PIN0)	;
		
		if((PIN_IS_LOW==en_sg_btn_1_status) && (FALSE==u8_sg_btn_1_IsPushed) )
		{
			u8_sg_btn_1_IsPushed = TRUE;
			chPtr_l_temp = chArr_g_btn_1_pressedMsg;
			xQueueSend( xQueueMesseges, &chPtr_l_temp, portMAX_DELAY );
		}
		else if((PIN_IS_HIGH==en_sg_btn_1_status) && (TRUE==u8_sg_btn_1_IsPushed))
		{
			u8_sg_btn_1_IsPushed = FALSE; 
			chPtr_l_temp = chArr_g_btn_1_releaseMsg;			
			xQueueSend( xQueueMesseges, &chPtr_l_temp, portMAX_DELAY );
		}
		else
		{
			// do nothing
		}
		
		vTaskDelay(BTN_ONE_TASK_DELAY);
	}
}

/*--------------------------Task_2----------------------------*/
void Button_Two_Task(void* pvParameters)
{
	
	int8_t *chPtr_l_temp = NULL;
	for(;;)
	{	
	
		en_sg_btn_2_status = GPIO_read(PORT_0,PIN1)	;
		
		if((PIN_IS_LOW==en_sg_btn_2_status) && (FALSE==u8_sg_btn_2_IsPushed) )
		{
			u8_sg_btn_2_IsPushed = TRUE;
			chPtr_l_temp = chArr_g_btn_2_pressedMsg;
			xQueueSend( xQueueMesseges, &chPtr_l_temp, portMAX_DELAY );
			
		}
		else if((PIN_IS_HIGH==en_sg_btn_2_status) && (TRUE==u8_sg_btn_2_IsPushed))
		{
			u8_sg_btn_2_IsPushed = FALSE;
			chPtr_l_temp = chArr_g_btn_2_releaseMsg;
			xQueueSend( xQueueMesseges, &chPtr_l_temp, portMAX_DELAY );
			  
		}
		else
		{
			// do nothing
		}

		vTaskDelay(BTN_TWO_TASK_DELAY);
	}
}

/*--------------------------Task_3----------------------------*/
void Periodec_Check_Task(void* pvParameters)
{
	int8_t *chPtr_l_temp = NULL;
	for(;;)
	{	
		GPIO_write(PORT_0, PIN4, PIN_IS_HIGH);
		chPtr_l_temp = chArr_g_PeriodicTaskStr;
	 //xQueueSend( xQueueMesseges,&chArr_g_PeriodicTaskStr, portMAX_DELAY );
		xQueueSend( xQueueMesseges,&chPtr_l_temp, portMAX_DELAY );
		GPIO_write(PORT_0, PIN4, PIN_IS_LOW);
		vTaskDelay(PERIDIC_TASK_DELAY);
	}
}
/*--------------------------Task_4----------------------------*/

void Consumer_Task(void* pvParameters)
{
	

	for(;;)
	{
		
		xQueueReceive( xQueueMesseges, &chPtr_g_tempBuffer, portMAX_DELAY );
		vSerialPutString(chPtr_g_tempBuffer,STRING_MAX_SIZE);
		
		vTaskDelay(CONSUMER_TASK_DELAY);
	}
}

/*-------------------------------------------------------------------------------*/

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	// to activate configSUPPORT_DYNAMIC_ALLOCATION must be set to 1 in FreeRTOSConfig.h
	xQueueMesseges = xQueueCreate(QUEUE_MAX_SIZE,sizeof(int8_t *));
	
	
    /* Create Tasks here */
	xTaskCreate(
								Button_One_Task,
								"Button_One_Task",
								configMINIMAL_STACK_SIZE,
								(void * ) NULL,
								1,
								&BtnOneTask_Handler
								);
								
	xTaskCreate(
								Button_Two_Task,
								"Button_Two_Task",
								configMINIMAL_STACK_SIZE,
								(void * ) NULL,
								1,
								&BtnTwoTask_Handler
								);
								
	xTaskCreate(
								Periodec_Check_Task,
								"Periodec_Check_Task",
								configMINIMAL_STACK_SIZE,
								(void * ) NULL,
								2,
								&PeriodicCheckTask_Handler
								);
	xTaskCreate(
								Consumer_Task,
								"Consumer_Task",
								configMINIMAL_STACK_SIZE,
								(void * ) NULL,
								1,
								&ConsumerTask_Handler
								);							
 
   	 		/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


