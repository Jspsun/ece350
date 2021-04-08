/*
 * k_event_queue.c
 *
 *  Created on: Apr. 6, 2021
 *      Author: h43ye
 */

#include "k_event_queue.h"

/*
 *==========================================================================
 *                            SCHEDULER VARIABLES
 *==========================================================================
 */

#define SUSPENDED_EVENT 0
#define DEADLINE_EVENT 1

static EVENT e_heap[MAX_TASKS];
static U32 e_queue_size = 0;

int e_compare(EVENT t1, EVENT t2) {
	return compare_timeval(t1.timestamp, t2.timestamp);
}

void e_swap(EVENT *t1, EVENT *t2) {
    EVENT t;
    t = *t1;
    *t1 = *t2;
    *t2 = t;
}

int e_parent(int i) {
    return (i > 0 && i < MAX_TASKS) ? (i - 1) / 2 : -1;
}

int e_left_child(int i) {
    return (2*i + 1 < e_queue_size && i >= 0) ? 2*i + 1: -1;
}

int e_right_child(int i) {
    return (2*i + 2 < e_queue_size && i >= 0) ? 2*i + 2 : -1;
}

void e_increase_key(EVENT heap[], int i) {
    while(i >= 0 && e_parent(i) >= 0 && !e_compare(heap[e_parent(i)], heap[i])) {
        e_swap(&heap[i], &heap[e_parent(i)]);
        i = e_parent(i);
    }
}

void e_decrease_key(EVENT heap[], int i) {
    int max_index = i;

    int l = e_left_child(i);

    if (l > 0 && e_compare(heap[l], heap[max_index])) {
        max_index = l;
    }

    int r = e_right_child(i);

    if (r > 0 && e_compare(heap[r], heap[max_index])) {
        max_index = r;
    }

    if (i != max_index) {
        e_swap(&heap[i], &heap[max_index]);
        e_decrease_key(heap, max_index);
    }
}

int e_insert(EVENT heap[], EVENT value) {
    if (e_queue_size + 1== MAX_TASKS) {
        return -1;
    }

    heap[e_queue_size] = value;
    e_increase_key(heap, e_queue_size);
    e_queue_size ++;

    return 0;
}

int e_next(EVENT heap[], TIMEVAL *time) {
	if (e_queue_size == 0) { return -1; }
	*time = heap[0].timestamp;
    return 0;
}

EVENT e_extract_max(EVENT heap[]) {
    EVENT max_value = heap[0];

    e_queue_size --;
    heap[0] = heap[e_queue_size];
    e_decrease_key(heap, 0);

    return max_value;
}

int e_find_value(EVENT heap[], U8 tid) {
    int i;
    for (i = 0; i < e_queue_size; i++) {
        if (heap[i].tid == tid) {
            return i;
        }
    }

    return -1;
}

void e_remove_id(EVENT heap[], U8 tid) {
	if (e_queue_size == 0) { return; }

	int idx;
    while (1) {
    	idx = e_find_value(heap, tid);
    	if (idx < 0) { break; }

        heap[idx] = heap[0];
        e_increase_key(heap, idx);
        e_extract_max(heap);

    }
}

void create_deadline_event(task_t tid, TIMEVAL deadline) {
	EVENT e = {DEADLINE_EVENT, tid, deadline};
	e_insert(e_heap, e);
}

void create_deadline(task_t tid, TIMEVAL p_n, TIMEVAL start) {
	create_deadline_event(tid, increment_tv(start, p_n));
}

int handle_event(EVENT e) {
	if (e.type == DEADLINE_EVENT) {
		edf_array[e.tid].deadline_count ++;
		create_deadline_event(e.tid, increment_tv(e.timestamp, edf_array[e.tid].info.p_n));
		return 0;
	} else if (e.type == SUSPENDED_EVENT) {
		TCB* tcb = &g_tcbs[e.tid];
		tcb->state = READY;
		insert(tcb);
		return 1;
	}

	return 0;
}

int update(TIMEVAL time) {
	TIMEVAL next_time;
	int flag = 0;
	while (e_next(e_heap, &next_time) >= 0) {
		if (compare_timeval(time, next_time)) {
			return flag;
		}

		flag |= handle_event(e_extract_max(e_heap));
	}

	return flag;
}

void event_suspend(task_t tid, TIMEVAL wake_up) {
	EVENT e = {SUSPENDED_EVENT, tid, wake_up};
	e_insert(e_heap, e);
}

void event_remove(task_t tid) {
	e_remove_id(e_heap, tid);
}

/*
 *===========================================================================
 *                            EDF FUNCTIONS
 *===========================================================================
 */

int compare_rt(TCB *t1, TCB* t2) {
	if (t1->prio != PRIO_RT || t2->prio != PRIO_RT) {
		return compare(t1, t2);
	}

	EDF_TASK_RT rt1 = edf_array[t1->tid];
	EDF_TASK_RT rt2 = edf_array[t2->tid];

	// TODO: compare rt1 and rt2 using info + last_run
	return compare_timeval(rt1.deadline, rt2.deadline);
}

void edf_insert(task_t tid, TASK_RT info) {

	TIMEVAL time = get_system_time();
	TIMEVAL wakeup = next_quanta(time);
	edf_array[tid].info = info;

	// TODO: fix this to use the timers
	edf_array[tid].deadline = increment_tv(wakeup, info.p_n);
	edf_array[tid].job_count = 0;
	edf_array[tid].deadline_count = 0;

	create_deadline(tid, info.p_n, wakeup);

	if (wakeup.usec <= time.usec) {
		// Start running right away
		insert(&g_tcbs[tid]);
		edf_array[tid].suspended = 0;
	} else {
		edf_array[tid].suspended = 1;
		event_suspend(tid, wakeup);
	}

}

void edf_remove(task_t tid) {
	edf_array[tid].suspended = 0;
	edf_array[tid].job_count = 0;
	event_remove(tid);
}

void edf_suspend(task_t tid, TIMEVAL suspend_time) {

	TIMEVAL time = get_system_time();
	TIMEVAL new_wakeup = increment_tv(time, suspend_time);

	edf_array[tid].suspended = 1;
	g_tcbs[tid].state = SUSPENDED;
	event_suspend(tid, new_wakeup);
}

int edf_done(task_t tid) {
	TIMEVAL time = get_system_time();
	edf_array[tid].job_count += 1;

	TCB* tcb = &g_tcbs[tid];
	tcb->rt_finished = 1;

	if (edf_array[tid].job_count <= edf_array[tid].deadline_count) {
		// missed deadline, reinsert and try running right away
		edf_array[tid].deadline = increment_tv(edf_array[tid].deadline, edf_array[tid].info.p_n);
		tcb->state = READY;
		printf("Missed deadline for job %d, task %d\n\r", edf_array[tid].job_count, tid);
		printf("Time: %d, %d\n\r", system_time.sec, system_time.usec);
		insert(tcb);
		return 1;
	}
	// suspend

	g_tcbs[tid].state = SUSPENDED;
	edf_array[tid].suspended = 1;
	event_suspend(tid, edf_array[tid].deadline);
	edf_array[tid].deadline = increment_tv(edf_array[tid].deadline, edf_array[tid].info.p_n);

	return 0;
}

void edf_get(task_t tid, TIMEVAL *p_n, size_t* rt_mbx_size) {
	*p_n = edf_array[tid].info.p_n;
	*rt_mbx_size = edf_array[tid].info.rt_mbx_size;
}
