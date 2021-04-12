/*
 * k_event_queue.h
 *
 *  Created on: Apr. 6, 2021
 *      Author: h43ye
 */

#include "k_task.h"
#include "printf.h"
#include "Serial.h"

#ifndef SRC_KERNEL_K_EVENT_QUEUE_H_
#define SRC_KERNEL_K_EVENT_QUEUE_H_

/*
 *==========================================================================
 *                            EDF VARIABLES
 *==========================================================================
 */

extern TIMEVAL system_time;
extern TIMEVAL last_checkpoint;
extern TIMEVAL get_system_time(void);

typedef struct Event {
	int type;
	task_t tid;
	TIMEVAL timestamp;
} EVENT;

typedef struct edf_task_rt {
	TASK_RT info;
	TIMEVAL deadline;
	int suspended;
	int flag; // if flag is 0, not in use.
	int job_count;
	int deadline_count;
} EDF_TASK_RT;

static EDF_TASK_RT edf_array[MAX_TASKS];

void event_remove(task_t tid);
void event_suspend(task_t tid, TIMEVAL wake_up);
int update(TIMEVAL time);
void create_deadline(task_t tid, TIMEVAL p_n, TIMEVAL start);

int compare_rt(TCB *t1, TCB* t2);
void edf_insert(task_t tid, TASK_RT info);
void edf_suspend(task_t tid, TIMEVAL suspend_time);
void edf_remove(task_t tid);
int edf_done(task_t tid);

void edf_get(task_t tid, TIMEVAL *p_n, size_t* rt_mbx_size);


#endif /* SRC_KERNEL_K_EVENT_QUEUE_H_ */
