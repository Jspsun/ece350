/*
 * k_event_queue.h
 *
 *  Created on: Apr. 6, 2021
 *      Author: h43ye
 */

#include "k_task.h"

#ifndef SRC_KERNEL_K_EVENT_QUEUE_H_
#define SRC_KERNEL_K_EVENT_QUEUE_H_

/*
 *==========================================================================
 *                            EDF VARIABLES
 *==========================================================================
 */

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


#endif /* SRC_KERNEL_K_EVENT_QUEUE_H_ */
