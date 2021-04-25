/*
 ****************************************************************************
 *
 *                  UNIVERSITY OF WATERLOO ECE 350 RTOS LAB
 *
 *                 Copyright 2020-2021 ECE 350 Teaching Team
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

#include "ae.h"

extern void kcd_task(void);

void create_task_spec(TASK_RT* task_spec, U32 tv_s, U32 tv_us, void (*entry)(), U16 stack_size, size_t mailbox_size){
  task_spec->task_entry = entry;
  task_spec->u_stack_size =stack_size;
  task_spec->rt_mbx_size = mailbox_size;
  task_spec->p_n.sec = tv_s;
  task_spec->p_n.usec = tv_us;
}

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
    printf("[ERROR] RTX_TASK_INFO undefined\n\r");
    return;
  }

#if TEST == 1
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_01!\r\n");
  printf("Info: Initializing system with a single user task with priority HIGH!\r\n");
  tasks[0].prio = HIGH;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask1;
  tasks[0].u_stack_size = 0x200;
#endif

#if TEST == 2
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_02!\r\n");
  printf("Info: Initializing system with a single user task with priority MEDIUM!\r\n");
  tasks[0].prio = MEDIUM;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask1;
  tasks[0].u_stack_size = 0x400;
#endif

#if TEST == 3
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_03!\r\n");
  printf("Info: Initializing system with a user task with priority MEDIUM and an rt-user task with period 10 second!\r\n");
  tasks[0].prio = MEDIUM;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask2;
  tasks[0].u_stack_size = 0x400;

  tasks[1].prio = PRIO_RT;
  tasks[1].priv = 0;
  tasks[1].ptask = &rt_utask1;
  tasks[1].u_stack_size = 0x400;
  tasks[1].rt_mbx_size = 0x400;
  tasks[1].p_n.sec = 10;
  tasks[1].p_n.usec = 0;
#endif

#if TEST == 4
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_04!\r\n");
  printf("Info: Initializing system with a rt-user task this task deadline is 6 second!\r\n");
  tasks[0].prio = PRIO_RT;
  tasks[0].priv = 0;
  tasks[0].ptask = &rt_utask1;
  tasks[0].u_stack_size = 0x400;
  tasks[0].rt_mbx_size = 0;
  tasks[0].p_n.sec = 6;
  tasks[0].p_n.usec = 0;
#endif

#if TEST == 5
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_05!\r\n");
  printf("Info: Initializing system with two rt-user tasks with the same deadline equal to 10 second!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].priv = 0;
  tasks[0].ptask = &rt_utask1;
  tasks[0].u_stack_size = 0x400;
  tasks[0].rt_mbx_size = 0x200;
  tasks[0].p_n.sec = 10;
  tasks[0].p_n.usec = 0;

  tasks[1].prio = PRIO_RT;
  tasks[1].priv = 0;
  tasks[1].ptask = &rt_utask2;
  tasks[1].u_stack_size = 0x400;
  tasks[1].rt_mbx_size = 0x200;
  tasks[1].p_n.sec = 10;
  tasks[1].p_n.usec = 0;
#endif

#if TEST == 11
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_11!\r\n");
  printf("Info: Initializing system with a single user task!\r\n");

  tasks[0].prio = HIGH;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask2;
  tasks[0].u_stack_size = 0x200;
#endif

#if TEST == 12
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_12!\r\n");
  printf("Info: Initializing system with a single user task!\r\n");

  tasks[0].prio = HIGH;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask2;
  tasks[0].u_stack_size = 0x200;
#endif

#if TEST == 13
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_13!\r\n");
  printf("Info: Initializing system with a single user task!\r\n");

  tasks[0].prio = HIGH;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask2;
  tasks[0].u_stack_size = 0x200;
#endif

#if TEST == 14
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_14!\r\n");
  printf("Info: Initializing system with a single user task!\r\n");

  tasks[0].prio = HIGH;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask3;
  tasks[0].u_stack_size = 0x200;
#endif

#if TEST == 15
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_15!\r\n");
  printf("Info: Initializing system with two RT user tasks one with period 5s and one with period 10s!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].priv = 0;
  tasks[0].ptask = &rt_utask1;
  tasks[0].u_stack_size = 0x400;
  tasks[0].rt_mbx_size = 0x200;
  tasks[0].p_n.sec = 5;
  tasks[0].p_n.usec = 0;

  tasks[1].prio = PRIO_RT;
  tasks[1].priv = 0;
  tasks[1].ptask = &rt_utask2;
  tasks[1].u_stack_size = 0x400;
  tasks[1].rt_mbx_size = 0x200;
  tasks[1].p_n.sec = 10;
  tasks[1].p_n.usec = 0;
#endif

#if TEST == 16
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_16!\r\n");
  printf("Info: Initializing system with a single user task!\r\n");

  tasks[0].prio = 1;
  tasks[0].priv = 0;
  tasks[0].ptask = &utask1;
  tasks[0].u_stack_size = 0x200;

  config_hps_timer(3, 0xFFFFFFFF, 0, 1);
#endif

#if TEST == 21
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_21!\r\n");
  printf("Info: Initializing system with 2 user tasks: one RT with period 10s, one HIGH!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].ptask = &rt_utask1; 
  tasks[0].rt_mbx_size = 0;
  tasks[0].u_stack_size = 0x200;
  TIMEVAL timeval = {.sec = 10, .usec = 0};
  tasks[0].p_n = timeval;
  tasks[0].priv = 0;

  tasks[1].prio = HIGH;
  tasks[1].ptask = &utask2;
  tasks[1].u_stack_size = 0x200;
  tasks[1].priv = 0;
#endif

#if TEST == 22
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_22!\r\n");
  printf("Info: Initializing system with 2 user tasks: one RT with period 2s, one LOWEST and 2 kernel tasks: both HIGH!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].ptask = &rt_utask1; 
  tasks[0].rt_mbx_size = 0;
  tasks[0].u_stack_size = 0x200;
  TIMEVAL timeval = {.sec = 2, .usec = 0};
  tasks[0].p_n = timeval;
  tasks[0].priv = 0;

  tasks[1].prio = LOWEST;
  tasks[1].ptask = &utask2;
  tasks[1].u_stack_size = 0x200;
  tasks[1].priv = 0;

  tasks[2].prio = HIGH;
  tasks[2].ptask = &ktask1;
  tasks[2].priv = 1;

  tasks[3].prio = HIGH;
  tasks[3].ptask = &ktask2;
  tasks[3].priv = 1;
#endif

#if TEST == 23
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_23!\r\n");
  printf("Info: Initializing system with 2 user tasks: one HIGH, one LOW!\r\n");

  tasks[0].prio = HIGH;
  tasks[0].ptask = &utask1;
  tasks[0].u_stack_size = 0x200;

  tasks[1].prio = LOW;
  tasks[1].ptask = &utask2;
  tasks[1].u_stack_size = 0x200;
#endif


#if TEST == 24
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_24!\r\n");
  printf("Info: Initializing system with 2 user tasks: one RT with period of 5s, one LOW and 1 RT kernel task!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].ptask = &rt_utask1; 
  tasks[0].rt_mbx_size = 0;
  tasks[0].u_stack_size = 0x200;
  TIMEVAL timeval = {.sec = 5, .usec = 0};
  tasks[0].p_n = timeval;
  tasks[0].priv = 0;

  tasks[1].prio = PRIO_RT;
  tasks[1].ptask = &rt_ktask1; 
  tasks[1].rt_mbx_size = 0;
  tasks[1].p_n = timeval;
  tasks[1].priv = 1;

  tasks[2].prio = HIGH;
  tasks[2].ptask = &utask2;
  tasks[2].u_stack_size = 0x200;
  tasks[2].priv = 0;
#endif

#if TEST == 25
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_25!\r\n");
  printf("Info: Initializing system with one RT user tasks with period of 2s!\r\n");


  tasks[0].prio = PRIO_RT;
  tasks[0].ptask = &rt_utask1;
  tasks[0].rt_mbx_size = 0;
  tasks[0].u_stack_size = 0x200;
  TIMEVAL timeval = {.sec = 2, .usec = 0};
  tasks[0].p_n = timeval;
  tasks[0].priv = 0;

  printf("Info: Setting up HPS timer 3 for testing!\r\n");
  config_hps_timer(3, 0xFFFFFFFF, 0, 1);

#endif

#if TEST == 26
  printf("============================================\r\n");
  printf("============================================\r\n");
  printf("Info: Starting T_26!\r\n");
  printf("Info: Initializing system with one RT user tasks with period of 1 sec and 5000 microseconds!\r\n");

  tasks[0].prio = PRIO_RT;
  tasks[0].ptask = &rt_utask1;
  tasks[0].rt_mbx_size = 0;
  tasks[0].u_stack_size = 0x200;
  TIMEVAL timeval = {.sec = 1, .usec = 5000};
  tasks[0].p_n = timeval;
  tasks[0].priv = 0;
#endif

  return;

}

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
