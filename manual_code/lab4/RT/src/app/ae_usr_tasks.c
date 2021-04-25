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
#include "ae_usr_tasks.h"
#include "rtx.h"
#include "Serial.h"
#include "printf.h"

task_t get_tid_unpriv(void (*entry_point)()){
  RTX_TASK_INFO buffer;
  for(task_t i=1; i<255; i++){
    if(tsk_get(i, &buffer) == RTX_OK){
      if(buffer.ptask == entry_point)
        return buffer.tid;
    }
  }
  return 0;
}

# if TEST == 16


#define INIT_TASKS_NUM 21
#define VALIDATION_STEPS (3 * INIT_TASKS_NUM)

volatile int validators[VALIDATION_STEPS];
volatile int index = 0;

U16 task_stack_size = 0x200;

unsigned char tids[INIT_TASKS_NUM];

int fibbonacci(int n) {
  if(n == 0) {
    return 0;
  } else if(n == 1) {
    return 1;
  } else {
    return (fibbonacci(n-1) + fibbonacci(n-2));
  }
}

void utask1(void)
{
  printf("[UT1] Info: Entering user task 1!\r\n");

  unsigned int t1, t2;  // used for timer
  int performance = 2147483647;
  int tid_index = 0;
  int sflag = 0;
  char tasks_creation_failed = 0;

  for (int i = 0; i < VALIDATION_STEPS; i++) {
    validators[i] = 0;
  }

  printf("[UT1] Info: Reading timer 3!\r\n");
  t1 = timer_get_current_val(3);

  printf("[UT1] Info: Creating %d non-rt tasks with priorities between 2 and %d! (3 tasks per priority)\r\n", INIT_TASKS_NUM, (2 + INIT_TASKS_NUM/3));
  printf("- Each task does the following:\r\n");
  printf(" - Yield!\r\n");
  printf(" - Create a mailbox!\r\n");
  printf(" - Call receive message!\r\n");
  printf(" - Get its priority!\r\n");
  printf(" - Write its priority to a global array!\r\n");
  printf(" - Add %d to its priority and yield!\r\n", INIT_TASKS_NUM/3);
  printf(" - Write its priority to a global array!\r\n");
  printf(" - Suspend for 500 usec!\r\n");
  printf(" - Call recursive Fibonacci function for index 10 and yield!\r\n");
  printf(" - Add %d to its priority and yield!\r\n", INIT_TASKS_NUM/3);
  printf(" - Write its priority to a global array and yield!\r\n");
  printf(" - Exit!\r\n");

  for (int i = 0; i < 3; i++) {
    for (int j = 2; j < (2 + INIT_TASKS_NUM/3); j++) {
      if (tsk_create(&tids[tid_index], &utask2, j, task_stack_size) == RTX_ERR) {
        printf("[UT1] Failed: Failed to create user task!\r\n");
        tasks_creation_failed = 1;
        break;
      } else {
        tid_index++;
      }
    }

    if (tasks_creation_failed == 1)
      break;
  }

  if (tasks_creation_failed == 0) {
    printf("[UT1] Info: Getting tid of the current task!\r\n");
    task_t my_tid = get_tid_unpriv(&utask1);
    if(my_tid != 0) {
      printf("[UT1] Info: Setting the priority of UT1 to 254!\r\n");
      printf("- Waiting for the created tasks to call receive and get blocked!\r\n");
      printf("[IMPORTANT] To GTAs: Wait for about 1 min, and if test doesn't terminate, manually terminate and give 0 to the test!\r\n");

      if (tsk_set_prio(my_tid, 254) == RTX_OK) {
        printf("[UT1] Info: Setting priority of task 1 back to 1!\r\n");

        if(tsk_set_prio(my_tid, 1) == RTX_OK) {
          RTX_MSG_HDR *msg = NULL;
          msg = mem_alloc(sizeof(RTX_MSG_HDR) + 1);
          if (msg != NULL) {
            printf("[UT1] Info: Setting up messages to be sent!\r\n");
            msg->type = DEFAULT;
            msg->length = sizeof(RTX_MSG_HDR) + 1;

            printf("[UT2] Info: Sending created tasks their tid!\r\n");
            tid_index = 0;
            for (int i = 0; i < INIT_TASKS_NUM; i++) {
              *((char *)(msg + 1)) = tids[i];
              if(send_msg(tids[i], msg) == RTX_ERR)
                break;
              tid_index++;
            }

            if (tid_index == INIT_TASKS_NUM) {
              printf("[UT1] Info: Setting the priority of task 1 again to 254!\r\n");
              printf("- Waiting for the created tasks to exit!\r\n");
              printf("[IMPORTANT] To GTAs: Wait for about 1 min, and if test doesn't terminate, manually terminate and give 0 to the test!\r\n");

              if (tsk_set_prio(my_tid, 254) == RTX_OK) {
                while(index < INIT_TASKS_NUM);
                printf("[UT1] Info: Checking the context of the global array!\r\n");
                int check_index = 0;
                sflag = 1;
                for (int i = 2; i <= INIT_TASKS_NUM; i++) {
                  for (int j = 0; j < 3; j ++) {
                    if (validators[check_index] != i) {
                      sflag = 0;
                      break;
                    }
                    check_index++;
                  }
                  if (sflag == 0) {
                    printf("[UT1] Failed: The content of the global array is not ordered correctly!");
                    break;
                  }
                }

                printf("[UT1] Info: Reading HPS timer 3!\r\n");
                t2 = timer_get_current_val(3);
                if (t1 > t2)
                  performance = (int) (t1 - t2);
                else
                  performance = (int)(0xFFFFFFFF - t2 + t1);
              } else {
                printf("[UT1] Failed: Could not set priority of task 1 to 254!\r\n");
              }
            } else {
              printf("[UT1] Failed: Could not send all TIDs to created tasks!\r\n");
            }
          } else {
            printf("[UT1] Failed: Could not allocate memory for messages!\r\n");
          }
        } else {
          printf("[UT1] Failed: Could not set priority of task 1 back to 1!\r\n");
        }
      } else {
        printf("[UT1] Failed: Could not set priority of task 1 to 254!\r\n");
      }
    } else {
      printf("[UT1] Failed: Could not find the tid of the current task!\r\n");
    }
  } else {
    printf("[UT1] Failed: Could not create user tasks!\r\n");
  }

  printf("============================================\r\n");
  printf("=============Final test results=============\r\n");
  printf("============================================\r\n");
  printf("[T_16] %d out of 1 tests passed!\r\n", sflag);
  printf("[T_16] Execution time: %d \r\n", performance);
  printf("[IMPORTANT] To GTAs: If the test is passed, run for 3 more times and record the lowest execution time among all 4 runs!\r\n");
  tsk_exit();
}

void utask2(void) {

  task_t my_tid = 0;
  int init_prio = 0;

  tsk_yield();

  mbx_create(0x1FF);

  RTX_MSG_HDR *msg = mem_alloc(sizeof(RTX_MSG_HDR) + 1);
  recv_msg(NULL, msg, sizeof(RTX_MSG_HDR) + 1);
  my_tid = *((char*) (msg + 1));

  RTX_TASK_INFO task_info;
  tsk_get(my_tid, &task_info);
  init_prio = task_info.prio;

  validators[index] = task_info.prio;
  index++;

  tsk_set_prio(my_tid, init_prio + INIT_TASKS_NUM/3);

  tsk_yield();

  validators[index] = init_prio + INIT_TASKS_NUM/3;
  index++;

  TIMEVAL timeval = {.sec = 0, .usec = 500};
  tsk_suspend(&timeval);

  fibbonacci(10);

  tsk_yield();

  tsk_set_prio(my_tid, init_prio + 2 *(INIT_TASKS_NUM/3));

  tsk_yield();

  validators[index] = init_prio + 2 *(INIT_TASKS_NUM/3);
  index++;

  tsk_yield();

  tsk_exit();
}

#endif


#if TEST == 26

volatile int count = 0;

void rt_utask1(void){
  printf("[RTUT1] Info: Entering RT user task 1!\r\n");

  printf("[RTUT1] Info: Checking global counter!\r\n");

  if(count == 0){

    TIMEVAL timeval = {.sec = 5, .usec = 0};

    printf("[RTUT1] Info: Calling suspend for 5 seconds in first run!\r\n");
    tsk_suspend(&timeval);

    printf("[RTUT1] Info: Returned from suspend. Increase the global counter and Calling tsk_done_rt!\r\n");
    count++;

    tsk_done_rt();
  } else if(count == 15) {

    printf("============================================\r\n");
    printf("=============Final test results=============\r\n");
    printf("============================================\r\n");
    printf("[T_26] To GTAs: Count the number of \"Job xx of task yy missed its deadline\" messages!\r\n");
    printf("- Give the group 1 out of 1 if there are only 5 messages!\r\n");
    printf("- If there are more or less error messages, run 2 more times!\r\n");
    printf("- If there are more or less error messages in all 3 runs, then give the group 0 out of 1!\r\n");

    tsk_exit();
  } else {
    count++;

    printf("[RTUT1] Info: Calling tsk_done_rt in %d run!\r\n", count);
    tsk_done_rt();
  }
}

#endif



/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
