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
 * @file        priv_tasks.c
 * @brief       Two privileged tasks: priv_task1 and priv_task2
 *
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *
 * @note        Each task is in an infinite loop. These Tasks never terminate.
 *
 *****************************************************************************/

#include "ae_priv_tasks.h"
#include "ae_usr_tasks.h"
#include "Serial.h"
#include "printf.h"

#if TEST == 0

void ktask1(void) {
	SER_PutStr(0, "ktask1: entering \n\r");
	RTX_TASK_INFO buffer;
	task_t tid= 1;

	if(k_tsk_get(tid, &buffer) != RTX_OK){
		SER_PutStr(0, "k_tsk_get failed\n\r");
	} else {
		if(buffer.rt_mbx_size == MIN_MBX_SIZE){
			SER_PutStr(0, "Got the right mbx size\n\r");
		}
		if(buffer.p_n.sec == 1 && buffer.p_n.usec == 0){
			SER_PutStr(0, "Got the right p_n\n\r");
		}
	}

	SER_PutStr(0, "Attempting to change own prio\n\r");
	if(k_tsk_set_prio(tid, HIGH) == RTX_ERR){
		SER_PutStr(0, "set_prio returns RTX_ERR, as expected\n\r");
	} else {
		SER_PutStr(0, "set_prio did not return RTX_ERR!\n\r");
	}

}

void ktask2(void) {
	SER_PutStr(0, "ktask2: entering \n\r");
	k_tsk_exit();
}

void ktask3(void) {
	SER_PutStr(0, "ktask3: entering \n\r");
	k_tsk_exit();
}

#endif

#if TEST == 1

void ktask1(void) {
	SER_PutStr(0, "ktask1: entering \n\r");

}

void ktask2(void) {
	SER_PutStr(0, "ktask2: entering \n\r");

}

void ktask3(void) {
	SER_PutStr(0, "ktask3: entering \n\r");

}

#endif

#if TEST == 2

void ktask1(void) {
	SER_PutStr(0, "ktask1: entering \n\r");

}

void ktask2(void) {
	SER_PutStr(0, "ktask2: entering \n\r");

}

void ktask3(void) {
	SER_PutStr(0, "ktask3: entering \n\r");

}

#endif

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
