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
#include "ae.h"

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

#if TEST==0
void utask1(void){
	printf("ktask1: entering \n\r");
	counter += 1;
	RTX_TASK_INFO buffer;
	task_t tid = 1;

	if(tsk_get(tid, &buffer) != RTX_OK){
		SER_PutStr(0, "k_tsk_get failed\n\r");
	} else {
		if(buffer.rt_mbx_size == MIN_MBX_SIZE){
			SER_PutStr(0, "Got the right mbx size\n\r");
		}
		if(buffer.p_n.sec == 1 && buffer.p_n.usec == 0){
			SER_PutStr(0, "Got the right p_n\n\r");
		}
	}

	printf("Attempting to change own prio %d\n\r", counter);

//	SER_PutStr(0, "Attempting to change own prio\n\r");
	if(tsk_set_prio(tid, HIGH) == RTX_ERR){
		SER_PutStr(0, "set_prio returns RTX_ERR, as expected\n\r");
	} else {
		SER_PutStr(0, "set_prio did not return RTX_ERR!\n\r");
	}

	tsk_done_rt();
}

void utask2(void){
	SER_PutStr(0, "ktask4: entering \n\r");
//	printf("Task ID: %d", gp_current_task->tid);
	printf("System Time: %d sec, %u sec", system_time.sec, system_time.usec);
	tsk_done_rt();
}
#endif

#if TEST==1
void utask1(void){
	SER_PutStr(0, "ktask1: entering \n\r");
	printf("System Time: %d sec, %u sec \n\r", system_time.sec, system_time.usec);
	tsk_done_rt();
}

void utask2(void){
	SER_PutStr(0, "ktask3: entering \n\r");
	printf("Task ID: %d", gp_current_task->tid);
	printf("System Time: %d sec, %u sec", system_time.sec, system_time.usec);
	tsk_done_rt();
}
#endif

#if TEST==2
void utask1(void){
	SER_PutStr(0, "utask1: entering \n\r");
	// printf("System Time: %d sec, %u sec\n\r", system_time.sec, system_time.usec);
	if(numDeadlines == 0){
		printf("Suspending for 3 seconds\n\r");
		TIMEVAL temp;
		temp.sec = 3;
		temp.usec = 0;
		tsk_suspend(&temp);
	}
	printf("Deadline %d Completion Time: %d %d\n\r", numDeadlines, system_time.sec, system_time.usec);
	numDeadlines++;
	if(numDeadlines == 10){
		tsk_exit();
	} else {
		tsk_done_rt();
	}
}

void utask2(void){
	SER_PutStr(0, "ktask3: entering \n\r");
	printf("Task ID: %d", gp_current_task->tid);
	printf("System Time: %d sec, %u sec", system_time.sec, system_time.usec);
	tsk_done_rt();
}
#endif

#if TEST==3
void utask1(void){
	printf("User Task 1 setting up RTX Task\n\r");
	TIMEVAL period;
	period.sec = 0;
	period.usec = 100;	// Period shorter than 500usec

	TASK_RT temp;
	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0x200;
	temp.rt_mbx_size = MIN_MBX_SIZE;

	task_t tid;

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 1);
	}

	period.sec = 0;
	period.usec = 600;	// Period not a multiple of 500usec

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 2);
	}

	period.sec = 1;
	period.usec = 200;	// Period not a multiple of 500usec

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 3);
	}

	period.sec = 1;
	period.usec = 0;

	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0x100; // Testing stack size too small
	temp.rt_mbx_size = MIN_MBX_SIZE;

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 4);
	}

	period.sec = 1;
	period.usec = 0;

	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0xFFFFFFFF;	// Stack size too big
	temp.rt_mbx_size = MIN_MBX_SIZE;

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 5);
	}

	period.sec = 1;
	period.usec = 0;

	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0x200;
	temp.rt_mbx_size = 0xFFFFFFFF;	// Mailbox size too big

	if(tsk_create_rt(&tid, &temp) == RTX_ERR){
		printf("create_rt failed as expected: %d\n\r", 6);
	}

	period.sec = 1;
	period.usec = 0;

	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0x200;
	temp.rt_mbx_size = MIN_MBX_SIZE;

	if(tsk_create_rt(&tid, &temp) == RTX_OK){
		printf("create_rt succeeded as expected: %d\n\r", 7);
	}

	if(tsk_set_prio(tid, HIGH) == RTX_ERR){
		printf("tsk_set_prio failed as expected: %d\n\r", 8);
	} else {
		printf("set_prio did not fail");
	}

	mbx_create(KCD_MBX_SIZE);
	task_t sender_tid;
	char* recv_buf = mem_alloc(KCD_MBX_SIZE);

	while(1){
		if(recv_msg_nb(&sender_tid, recv_buf, KCD_MBX_SIZE) == RTX_OK){
			SER_PutStr(0, "User task received a message\n\r");
		} else {
			SER_PutStr(0, "Checked mailbox, it was empty\n\r");
		}
		tsk_yield();
	}
}

void utask2(void){
	SER_PutStr(0, "utask2: entering \n\r");
	printf("Suspending for 3 seconds\n\r");
	TIMEVAL temp;
	temp.sec = 3;
	temp.usec = 0;
	tsk_suspend(&temp);
	tsk_exit();
}
#endif

#if TEST==4
void utask1(void){
	TIMEVAL t2Start;
	t2Start.sec = 0;
	t2Start.usec = 500000;

	while(compare_timeval(t2Start, system_time) == 0){
		;
	}

	TIMEVAL period;
	period.sec = 1;
	period.usec = 0;

	TASK_RT temp;
	temp.p_n = period;
	temp.task_entry = &utask2;
	temp.u_stack_size = 0x200;
	temp.rt_mbx_size = MIN_MBX_SIZE;

	task_t tid;

	if(tsk_create_rt(&tid, &temp) == RTX_OK){
		printf("Task Creation successful\n\r");
	} else {
		printf("Task creation failed\n\r");
	}

	mbx_create(KCD_MBX_SIZE);
	task_t sender_tid;
	char* recv_buf = mem_alloc(KCD_MBX_SIZE);

	while(1){
		int a;
		for(int i = 0; i < 0xFFF; i++){
			a++;
		}
		if(recv_msg_nb(&sender_tid, recv_buf, KCD_MBX_SIZE) == RTX_OK){
			SER_PutStr(0, "User task received a message\n\r");
		} else {
			SER_PutStr(0, "Checked mailbox, it was empty\n\r");
		}
		tsk_yield();
	}

	tsk_done();
}

void utask2(void){
	SER_PutStr(0, "utask2: entering \n\r");
	printf("Task ID: %d\n\r", gp_current_task->tid);
	printf("System Time: %d sec, %u sec\n\r", system_time.sec, system_time.usec);
	tsk_done_rt();
}
#endif

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
