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
 * @file        ae.c
 * @brief       Automated Evaluation (AE) Framework Source File
 *
 * @version     V1.2021.01.lab2
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *
 * @attention
 * @note
 * @details
 *
 *****************************************************************************/

#include "ae.h"

/**************************************************************************//**
 * @brief   	ae_init
 * @return		RTX_OK on success and RTX_ERR on failure
 * @param[out]  sys_info system initialization struct AE writes to
 * @param[out]	task_info boot-time tasks struct array AE writes to
 *
 *****************************************************************************/

int ae_init(RTX_SYS_INFO *sys_info, RTX_TASK_INFO *task_info, int num_tasks) {

	if (ae_set_sys_info(sys_info) != RTX_OK) {
		return RTX_ERR;
	}

	ae_set_task_info(task_info, num_tasks);
	return RTX_OK;
}

/**************************************************************************//**
 * @brief       fill the sys_info struct with system configuration info.
 * @return		RTX_OK on success and RTX_ERR on failure
 * @param[out]  sys_info system initialization struct AE writes to
 *
 *****************************************************************************/
int ae_set_sys_info(RTX_SYS_INFO *sys_info) {
	if (sys_info == NULL) {
		return RTX_ERR;
	}

	// Scheduling sys info set up, only do DEFAULT in lab2
	sys_info->sched = DEFAULT;

	/************* NOT USED in LAB2 ********************
	 struct timeval_rt budget;
	 struct timeval_rt period;

	 budget.sec = 0;
	 budget.usec = MIN_RTX_QTM * 10;

	 period.sec = 0;
	 period.usec = MIN_RTX_QTM * 100;


	 sys_info->rtx_time_qtm = 10 * MIN_RTX_QTM;

	 sys_info->server.b_n = budget;
	 sys_info->server.p_n = period;
	 ****************************************************/
	return RTX_OK;
}

/**************************************************************************//**
 * @brief       fill the tasks array with information
 * @param[out]  tasks 		An array of RTX_TASK_INFO elements to write to
 * @param[in]	num_tasks	The length of tasks array
 * @return		None
 *****************************************************************************/

void ae_set_task_info(RTX_TASK_INFO *tasks, int num_tasks) {

    if (tasks == NULL) {
        return;
    }

	#if TEST == 0 // Run as privledged first, then unprivileged
		SER_PutStr(0, "Setting up Test 0\n\r");
		TIMEVAL temp;
		temp.sec = 1;
		temp.usec = 0;

		tasks[0].u_stack_size = 0x200;
		tasks[0].ptask = &utask1;
		tasks[0].prio = PRIO_RT;
		tasks[0].priv = 0;
		tasks[0].p_n = temp;
		tasks[0].rt_mbx_size = MIN_MBX_SIZE;
	#endif

	#if TEST == 1 // Run as privledged first, then unprivileged
		SER_PutStr(0, "Setting up Test 1\n\r");
		TIMEVAL temp;
		temp.sec = 0;
		temp.usec = 500000;

		tasks[0].u_stack_size = 0x200;
		tasks[0].ptask = &ktask1;
		tasks[0].prio = PRIO_RT;
		tasks[0].priv = 1;
		tasks[0].p_n = temp;
		tasks[0].rt_mbx_size = MIN_MBX_SIZE;
	#endif

	#if TEST == 2 // Run as privledged first, then unprivileged
		SER_PutStr(0, "Setting up Test 2\n\r");
		TIMEVAL temp;
		temp.sec = 1;
		temp.usec = 0;

		tasks[0].u_stack_size = 0x200;
		tasks[0].ptask = &ktask1;	// Change this to ktask 1/2/3
		tasks[0].prio = PRIO_RT;
		tasks[0].priv = 1;
		tasks[0].p_n = temp;
		tasks[0].rt_mbx_size = MIN_MBX_SIZE;
	#endif

	#if TEST == 3
		SER_PutStr(0, "Setting up Test 3\n\r");
		TIMEVAL temp;
		temp.sec = 0;
		temp.usec = 500000;

		tasks[0].u_stack_size = 0x200;
		tasks[0].ptask = &utask1;	// Change this to ktask 1/2/3
		tasks[0].prio = HIGH;
		tasks[0].priv = 0; // Change this to switch between privileged and unprivileged
	#endif

	#if TEST == 4
		SER_PutStr(0, "Setting up Test 4\n\r");
		TIMEVAL temp;
		temp.sec = 1;
		temp.usec = 0;

		tasks[0].u_stack_size = 0x200;
		tasks[0].ptask = &ktask1;	// Change this to ktask 1/2/3
		tasks[0].prio = PRIO_RT;
		tasks[0].priv = 1;
		tasks[0].p_n = temp;
		tasks[0].rt_mbx_size = MIN_MBX_SIZE;

		tasks[1].u_stack_size = 0x200;
		tasks[1].ptask = &utask1;	// Change this to ktask 1/2/3
		tasks[1].prio = HIGH;
		tasks[1].priv = 0;
	#endif

	#if TEST==5
		SER_PutStr(0, "Setting up Test 5\n\r");
		for(int i = 0; i < MAX_TASKS - 1; i++){
			tasks[i].u_stack_size = 0x200;
			tasks[i].ptask = &utask1;	// Change this to ktask 1/2/3
			tasks[i].prio = HIGH;
			tasks[i].priv = 0;
		}
	#endif

    return;
}

/* only used in LAB1 */
#ifdef AE_LAB1
int ae_start(void) {
    return test_mem();
}
#endif

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
