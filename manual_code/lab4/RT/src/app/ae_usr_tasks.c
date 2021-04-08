/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO ECE 350 RTOS LAB
 *
 *                     Copyright 2020-2021 Yiqing Huang
 *                          All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice and the following disclaimer.
 *
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************
 */

/**************************************************************************//**
 * @file        a usr_tasks.c
 * @brief       Two user/unprivileged  tasks: task1 and task2
 *
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *****************************************************************************/

#include "ae_usr_tasks.h"
#include "rtx.h"
#include "Serial.h"
#include "printf.h"

void check_sys_timer(void){
	printf("check_sys_timer: entering \n\r");
	while (1) {
//		for(int j = 0; j < 100; j++){
			int a = 0;
			for (int i=0; i<0xFFFFFF; i++) {
				a++; // artifical delay
			}
			printf("sec: %d usec: %d\n\r", system_time.sec, system_time.usec);
		}
//		int wackho = 7;
//	}
}

void check_sys_timer_after_SVC(void){
	SER_PutStr(0, "check_sys_timer_after_SVC: entering \n\r");
	while (1) {
		for(int j = 0; j < 4; j++){
			int a = 0;
			for (int i=0; i<0xFFFFFF; i++){
				a++; // artifical delay
			}

			printf("system time sec: %d ", system_time.sec);
			printf("system time usec: %d \n\r", system_time.usec);
		}
		tsk_yield();
	}
}

static int counter = 0;

void rtask1(void)
{
//	SER_PutStr(0, "utask2: entering \n\r");
	if (counter == 3) {
		int j = 0;
		int i = j;
	}

	printf("rtask1, sec: %d usec: %d, count: %d\n\r", system_time.sec, system_time.usec, counter);
	TIMEVAL start = system_time;

	counter += 1;
	int a = 0;
	int delay = counter % 10 == 0 ? 0x3FFFFF : 0;

	for (int i=0; i < delay; i++) {
		a++; // artifical delay
	}

	TIMEVAL t = get_system_time();

//	printf("rtask1 done, elapsed, sec: %d, usec: %d\n\r", t.sec - start.sec, t.usec - start.usec);

//	printf("utask2: done, %d \n\r", counter);

	tsk_done_rt();
}

void rtask2(void)
{
	printf("rtask2, sec: %d usec: %d\n\r", system_time.sec, system_time.usec);

	tsk_done_rt();
}

/**
 * @brief: a dummy task1
 */
void utask1(void)
{
	TIMEVAL deadline = {0, 500000};
	TASK_RT info = {deadline,  rtask1, PROC_STACK_SIZE, MIN_MBX_SIZE};
	task_t tid;
	tsk_create_rt(&tid, &info);

	TIMEVAL deadline2 = {2, 0};
	TASK_RT info2 = {deadline2, rtask2, PROC_STACK_SIZE, MIN_MBX_SIZE};
	task_t tid2;
	tsk_create_rt(&tid2, &info2);
//	printf("rtask1, sec: %d usec: %d, count: %d\n\r", system_time.sec, system_time.usec, counter);

	//TIMEVAL suspend_time = {3, 0};
	//tsk_suspend(&suspend_time);

//	printf("rtask1, sec: %d usec: %d, count: %d\n\r", system_time.sec, system_time.usec, counter);

	while(1) {
	//	SER_PutStr(0, "utask1: entering \n\r");
		int a = 0;
		for (int i=0; i<0xFFFFFF; i++) {
			a++; // artifical delay
		}
	//	printf("task1, sec: %d usec: %d\n\r", system_time.sec, system_time.usec);
	}

}

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
