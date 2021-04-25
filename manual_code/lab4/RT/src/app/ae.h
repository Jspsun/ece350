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

#ifndef AE_
#define AE_

#include "Serial.h"
#include "printf.h"
#include "rtx.h"
#include "ae_priv_tasks.h"
#include "ae_usr_tasks.h"

void create_task_spec(TASK_RT* task_spec, U32 tv_s, U32 tv_us, void (*entry)(), U16 stack_size, size_t mailbox_size);


#define TEST 1 


#if TEST == 1
#define init_tasks 1
#endif

#if TEST == 2
#define init_tasks 1
#endif

#if TEST == 3
#define init_tasks 2
#endif

#if TEST == 4
#define init_tasks 1
#endif

#if TEST == 5
#define init_tasks 2
#endif

#if TEST == 11
#define init_tasks 1
#endif

#if TEST == 12
#define init_tasks 1
#endif

#if TEST == 13
#define init_tasks 1
#endif

#if TEST == 14
#define init_tasks 1
#endif

#if TEST == 15
#define init_tasks 2
#endif

#if TEST == 16
#define init_tasks 1
#endif

#if TEST == 21
#define init_tasks 2
#endif

#if TEST == 22
#define init_tasks 4
#endif

#if TEST == 23
#define init_tasks 2
#endif

#if TEST == 24
#define init_tasks 3
#endif

#if TEST == 25
#define init_tasks 1
#endif

#if TEST == 26
#define init_tasks 1
#endif

/*
 *===========================================================================
 *                            FUNCTION PROTOTYPES
 *===========================================================================
 */

int  ae_init          (RTX_SYS_INFO *sys_info, \
    RTX_TASK_INFO *task_info, int num_tasks);
int  ae_set_sys_info  (RTX_SYS_INFO *sys_info);
void ae_set_task_info (RTX_TASK_INFO *tasks, int num_tasks);
int  ae_start(void);

int  test_mem(void);

#endif // ! AE_
/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */

