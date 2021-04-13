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
 * @file        k_task.c
 * @brief       task management C file
 *              l2
 * @version     V1.2021.01
 * @authors     Yiqing Huang
 * @date        2021 JAN
 *
 * @attention   assumes NO HARDWARE INTERRUPTS
 * @details     The starter code shows one way of implementing context switching.
 *              The code only has minimal sanity check.
 *              There is no stack overflow check.
 *              The implementation assumes only two simple privileged task and
 *              NO HARDWARE INTERRUPTS.
 *              The purpose is to show how context switch could be done
 *              under stated assumptions.
 *              These assumptions are not true in the required RTX Project!!!
 *              Understand the assumptions and the limitations of the code before
 *              using the code piece in your own project!!!
 *
 *****************************************************************************/

//#include "VE_A9_MP.h"
#include "Serial.h"
#include "k_task.h"
#include "k_rtx.h"
#include "printf.h"

#ifdef DEBUG_0
//#include "printf.h"
#endif /* DEBUG_0 */

/*
 *==========================================================================
 *                            GLOBAL VARIABLES
 *==========================================================================
 */

TCB             *gp_current_task = NULL;	// the current RUNNING task
TCB             g_tcbs[MAX_TASKS];			// an array of TCBs
RTX_TASK_INFO   g_null_task_info;			// The null task info
U32             g_num_active_tasks = 0;		// number of non-dormant tasks

TIMEVAL last_checkpoint;
TIMEVAL	system_time;
unsigned int a9_timer_last = 0xFFFFFFFF; // the initial value of free-running timer

unsigned int a9_timer_curr;
U32 a9_delta;

/*---------------------------------------------------------------------------
The memory map of the OS image may look like the following:

                       RAM_END+---------------------------+ High Address
                              |                           |
                              |                           |
                              |    Free memory space      |
                              |   (user space stacks      |
                              |         + heap            |
                              |                           |
                              |                           |
                              |                           |
 &Image$$ZI_DATA$$ZI$$Limit-->|---------------------------|-----+-----
                              |         ......            |     ^
                              |---------------------------|     |
                              |      PROC_STACK_SIZE      |     |
             g_p_stacks[15]-->|---------------------------|     |
                              |                           |     |
                              |  other kernel proc stacks |     |
                              |---------------------------|     |
                              |      PROC_STACK_SIZE      |  OS Image
              g_p_stacks[2]-->|---------------------------|     |
                              |      PROC_STACK_SIZE      |     |
              g_p_stacks[1]-->|---------------------------|     |
                              |      PROC_STACK_SIZE      |     |
              g_p_stacks[0]-->|---------------------------|     |
                              |   other  global vars      |     |
                              |                           |  OS Image
                              |---------------------------|     |
                              |      KERN_STACK_SIZE      |     |                
             g_k_stacks[15]-->|---------------------------|     |
                              |                           |     |
                              |     other kernel stacks   |     |                              
                              |---------------------------|     |
                              |      KERN_STACK_SIZE      |  OS Image
              g_k_stacks[2]-->|---------------------------|     |
                              |      KERN_STACK_SIZE      |     |                      
              g_k_stacks[1]-->|---------------------------|     |
                              |      KERN_STACK_SIZE      |     |
              g_k_stacks[0]-->|---------------------------|     |
                              |   other  global vars      |     |
                              |---------------------------|     |
                              |        TCBs               |  OS Image
                      g_tcbs->|---------------------------|     |
                              |        global vars        |     |
                              |---------------------------|     |
                              |                           |     |          
                              |                           |     |
                              |                           |     |
                              |                           |     V
                     RAM_START+---------------------------+ Low Address
    
---------------------------------------------------------------------------*/ 


/*
 *==========================================================================
 *                             UTILITY
 *==========================================================================
 */

// returns 1 if t1 is earlier than t2
int compare_timeval(TIMEVAL t1, TIMEVAL t2) {
	return t1.sec < t2.sec || (t1.sec == t2.sec && t1.usec <= t2.usec);
}

TIMEVAL increment_tv(TIMEVAL t1, TIMEVAL t2) {
	TIMEVAL t = {t1.sec + t2.sec, t1.usec + t2.usec};
	if (t.usec > 1000000) {
		t.sec ++;
		t.usec = t.usec % 1000000;
	}

	return t;
}

static int time_counter = 1;

TIMEVAL get_system_time() {
    a9_timer_curr = timer_get_current_val(2);	//get the current value of the free running timer
    
    // a9_delta: time elapsed since start of previous HPS0 interrupt
    if(a9_timer_curr <= a9_timer_last){
        a9_delta = a9_timer_last - a9_timer_curr;
    } else {
        a9_delta = a9_timer_last + (0xFFFFFFFF - a9_timer_curr);
    }

    TIMEVAL curr_time;
    curr_time.sec = last_checkpoint.sec;
    curr_time.usec = last_checkpoint.usec + a9_delta;

    // fix overflow (1 s = 1,000,000 us)
    if(curr_time.usec > 1000000) {
        curr_time.sec += curr_time.usec / 1000000;
        curr_time.usec = curr_time.usec % 1000000;
    }

    system_time = curr_time;

    time_counter += 1;

   // printf("Current time: %d, %d\n\r", curr_time.sec, curr_time.usec);

    if (time_counter % 10 == 0) {
    	int j = 0;
    	int i = 1;
    }

    return curr_time;
}

TIMEVAL next_quanta(TIMEVAL time) {
	if (time.usec % 100 == 0) {
		return time;
	}

	if (time.usec + 100 >= 1000000) {
		TIMEVAL ret = {time.sec + 1, time.usec + 100 - 10000000 };
		return ret;
	}

	TIMEVAL ret = {time.sec, time.usec + 100 - time.usec % 100};

	return ret;
}

/*
 *==========================================================================
 *                            SCHEDULER VARIABLES
 *==========================================================================
 */

static TCB* heap[MAX_TASKS * 2];
static const int h_array_size = MAX_TASKS;
static U32 current_size = 0;
static U32 g_task_count = 0;

/*
 *===========================================================================
 *                            SCHEDULER FUNCTIONS
 *===========================================================================
 */

int compare(TCB *t1, TCB* t2) {
	if (t1 == t2) { return 1; }

	if (t1->prio == PRIO_RT && t2->prio == PRIO_RT) {
		return compare_rt(t1, t2);
	}

    int result = (*t1).prio < (*t2).prio || ((*t1).prio == (*t2).prio && (*t1).task_count <= (*t2).task_count);
  //  printf("V1: %d, %d, %d - V2: %d, %d, %d - Result: %d\r\n", t1->tid, t1->prio, t1->task_count, t2->tid, t2->prio, t2->task_count, result);
    return result;
}

void swap(TCB** t1, TCB** t2) {
    TCB* t;
    t = *t1;
    *t1 = *t2;
    *t2 = t;
}

int parent(int i) {
    return (i > 0 && i < h_array_size) ? (i - 1) / 2 : -1;
}

int left_child(int i) {
    return (2*i + 1 < current_size && i >= 0) ? 2*i + 1: -1;
}

int right_child(int i) {
    return (2*i + 2 < current_size && i >= 0) ? 2*i + 2 : -1;
}

void increase_key(TCB* heap[], int i) {
    while(i >= 0 && parent(i) >= 0 && !compare(heap[parent(i)], heap[i])) {
        swap(&heap[i], &heap[parent(i)]);
        i = parent(i);
    }
}

void decrease_key(TCB* heap[], int i) {
    int max_index = i;

    int l = left_child(i);

    if (l > 0 && compare(heap[l], heap[max_index])) {
        max_index = l;
    }

    int r = right_child(i);

    if (r > 0 && compare(heap[r], heap[max_index])) {
        max_index = r;
    }

    if (i != max_index) {
        swap(&heap[i], &heap[max_index]);
        decrease_key(heap, max_index);
    }
}

int16_t maximum(TCB* heap[]) {
    return current_size == 0 ? -1 : heap[0]->tid;
}

int16_t extract_max(TCB* heap[]) {
	if (current_size == 0) { return 0; }

    int16_t max_value = maximum(heap);

    current_size --;
    heap[0] = heap[current_size];
    decrease_key(heap, 0);

    return max_value;
}

int find_value(TCB* heap[], U8 tid) {
    int i;
    for (i = 0; i < current_size; i++) {
        if (heap[i]->tid == tid) {
            return i;
        }
    }

    return -1;
}

int insert(TCB* value) {
	if (find_value(heap, value->tid) >= 0 || gp_current_task == value) {
		return -1;
	}

	value->rt_finished = 0;

    if (current_size + 1== h_array_size) {
        return -1;
    }

    g_task_count += 1;
    (*value).task_count = g_task_count;
    heap[current_size] = value;
    increase_key(heap, current_size);
    current_size ++;

    return 0;
}

void reset_priority(TCB* heap[], U8 tid, U8 priority) {
    int index = find_value(heap, tid);
    if (index < 0) { return; }

    TCB v = *heap[index];

    g_task_count += 1;
    heap[index]->prio = priority;
    heap[index]->task_count = g_task_count;

    if (compare(heap[index], &v)) {
        increase_key(heap, index);
    } else {
        decrease_key(heap, index);
    }
}

void remove_id(TCB* heap[], U8 tid) {
	if (current_size == 0) { return; }

    int i = find_value(heap, tid);

    if (i < 0) { return; }

    //printf("found id: %d\n", i);
    heap[i] = heap[0];
    heap[i]->task_count += 1;
    increase_key(heap, i);
    //print_heap(heap, current_size);
    extract_max(heap);
}

/*
 *===========================================================================
 *                            FUNCTIONS
 *===========================================================================
 */

/**************************************************************************//**
 * @brief   scheduler, pick the TCB of the next to run task
 *
 * @return  TCB pointer of the next to run task
 * @post    gp_curret_task is updated
 *
 *****************************************************************************/

int currently_running() {
	return gp_current_task && gp_current_task->state == RUNNING;
}

TCB *scheduler(void)
{
	int16_t tid = maximum(heap);
	//int16_t tid = extract_max(heap);
	if (tid < 0) { return gp_current_task; }

	TCB* current_tcb = gp_current_task;
	TCB* max_ready_tcb = &g_tcbs[(U8)tid];

	// current task has higher priority
	if (gp_current_task->state != DORMANT   &&
		gp_current_task->state != BLK_MSG   &&
		gp_current_task->state != SUSPENDED &&
		current_tcb->prio < max_ready_tcb->prio) {

		return gp_current_task;
	}

	gp_current_task = max_ready_tcb;
    extract_max(heap);

    if (!(current_tcb->state == BLK_MSG || current_tcb->state == SUSPENDED || (current_tcb->prio == PRIO_RT && current_tcb->rt_finished))) {
    	insert(current_tcb);
    }

    return gp_current_task;

}

/**************************************************************************//**
 * @brief       initialize all boot-time tasks in the system,
 *
 *
 * @return      RTX_OK on success; RTX_ERR on failure
 * @param       task_info   boot-time task information structure pointer
 * @param       num_tasks   boot-time number of tasks
 * @pre         memory has been properly initialized
 * @post        none
 *
 * @see         k_tsk_create_new
 *****************************************************************************/

int k_tsk_init(RTX_TASK_INFO *task_info, int num_tasks)
{
    extern U32 SVC_RESTORE;

    RTX_TASK_INFO *p_taskinfo = &g_null_task_info;
    g_num_active_tasks = 0;

    if (num_tasks > MAX_TASKS - 1) { // num_tasks + null task <= MAX_TASKS
    	return RTX_ERR;
    }

    // create the first task
    TCB *p_tcb = &g_tcbs[0];
    p_tcb->prio     = PRIO_NULL;
    p_tcb->priv     = 1;
    p_tcb->tid      = TID_NULL;
    p_tcb->state    = RUNNING;
    p_tcb->mailbox  = NULL;
    p_tcb->rt_finished = 0;
    p_tcb->was_suspended = 0;
    g_num_active_tasks++;
    gp_current_task = p_tcb;

    // initialize all TCB task ids and states to DORMANT
    for(int i = 1; i < MAX_TASKS; i++){
        g_tcbs[i].state = DORMANT;
        g_tcbs[i].tid = i;
        g_tcbs[i].mailbox = NULL;
        g_tcbs[i].rt_finished = 0;
        g_tcbs[i].was_suspended = 0;
    }

    // create the rest of the tasks
    p_taskinfo = task_info;
    int numCreated = 0;
    int tcb_index = 1;
    while(numCreated != num_tasks){
        if(p_taskinfo->ptask == kcd_task){                  // TODO: Check the Extern for KCD_TASK use function.
            TCB *p_tcb = &g_tcbs[TID_KCD];
            if (k_tsk_create_new(p_taskinfo, p_tcb, TID_KCD) == RTX_OK) {
                g_num_active_tasks++;
            }
        } else {
            if(p_taskinfo->prio == PRIO_RT){
                TASK_RT temp; // Does not need to be mem alloc'd
                temp.p_n = p_taskinfo->p_n;
                temp.task_entry = p_taskinfo->ptask;
                temp.u_stack_size = p_taskinfo->u_stack_size;
                temp.rt_mbx_size = p_taskinfo->rt_mbx_size;

                task_t * dummy;
                if (k_tsk_create_rt_priv(dummy, &temp, p_taskinfo->priv) == RTX_OK) {
                    tcb_index++;
                } else {
                    return RTX_ERR;
                }

            } else {
                TCB *temp_tcb = &g_tcbs[tcb_index];
                if (k_tsk_create_new(p_taskinfo, temp_tcb, tcb_index) == RTX_OK) {
                    g_num_active_tasks++;
                    tcb_index++;
                }
            }
        }
        p_taskinfo++;
        numCreated++;
    }

    return RTX_OK;
}

int validate_stack_size(int stack_size) {
    // stack size too small
    if (stack_size < PROC_STACK_SIZE){
        return RTX_ERR;
    }

    // stack size too big
    size_t ALL_HEAP = 0xFFFFFFFF;
    size_t suitable_regions = k_mem_count_extfrag(ALL_HEAP) - k_mem_count_extfrag(stack_size);
    if (!suitable_regions){
        return RTX_ERR;
    }

    // stack_size not 8 bytes aligned
    if (stack_size % 8 != 0){
        return RTX_ERR;
    }

    return RTX_OK;
}
/**************************************************************************//**
 * @brief       initialize a new task in the system,
 *              one dummy kernel stack frame, one dummy user stack frame
 *
 * @return      RTX_OK on success; RTX_ERR on failure
 * @param       p_taskinfo  task information structure pointer
 * @param       p_tcb       the tcb the task is assigned to
 * @param       tid         the tid the task is assigned to
 *
 * @details     From bottom of the stack,
 *              we have user initial context (xPSR, PC, SP_USR, uR0-uR12)
 *              then we stack up the kernel initial context (kLR, kR0-kR12)
 *              The PC is the entry point of the user task
 *              The kLR is set to SVC_RESTORE
 *              30 registers in total
 *
 *****************************************************************************/
int k_tsk_create_new(RTX_TASK_INFO *p_taskinfo, TCB *p_tcb, task_t tid)
{
    extern U32 SVC_RESTORE;
    extern U32 K_RESTORE;

    U32 *sp;

    if (p_taskinfo == NULL || p_tcb == NULL)
    {
        return RTX_ERR;
    }

    p_tcb->tid = tid;
    p_tcb->state = READY;

    /*---------------------------------------------------------------
     *  Step1: allocate kernel stack for the task
     *         stacks grows down, stack base is at the high address
     * -------------------------------------------------------------*/

    ///////sp = g_k_stacks[tid] + (KERN_STACK_SIZE >> 2) ;
    sp = k_alloc_k_stack(tid);

    // 8B stack alignment adjustment
    if ((U32)sp & 0x04) {   // if sp not 8B aligned, then it must be 4B aligned
        sp--;               // adjust it to 8B aligned
    }

    /*-------------------------------------------------------------------
     *  Step2: create task's user/sys mode initial context on the kernel stack.
     *         fabricate the stack so that the stack looks like that
     *         task executed and entered kernel from the SVC handler
     *         hence had the user/sys mode context saved on the kernel stack.
     *         This fabrication allows the task to return
     *         to SVC_Handler before its execution.
     *
     *         16 registers listed in push order
     *         <xPSR, PC, uSP, uR12, uR11, ...., uR0>
     * -------------------------------------------------------------*/

    // if kernel task runs under SVC mode, then no need to create user context stack frame for SVC handler entering
    // since we never enter from SVC handler in this case
    // uSP: initial user stack
    if ( p_taskinfo->priv == 0 ) { // unprivileged task
        // xPSR: Initial Processor State
        *(--sp) = INIT_CPSR_USER;
        // PC contains the entry point of the user/privileged task
        *(--sp) = (U32) (p_taskinfo->ptask);

        //********************************************************************//
        //*** allocate user stack from the user space, not implemented yet ***//
        //********************************************************************//
        p_tcb->u_stack_size = p_taskinfo->u_stack_size;
        *(--sp) = (U32)k_alloc_p_stack(tid);

        // store user stack hi pointer in TCB
        p_tcb->u_stack_hi = *sp;    // user stack hi grows downwards

        // uR12, uR11, ..., uR0
        for ( int j = 0; j < 13; j++ ) {
            *(--sp) = 0x0;
        }
    }


    /*---------------------------------------------------------------
     *  Step3: create task kernel initial context on kernel stack
     *
     *         14 registers listed in push order
     *         <kLR, kR0-kR12>
     * -------------------------------------------------------------*/
    if ( p_taskinfo->priv == 0 ) {
        // user thread LR: return to the SVC handler
        *(--sp) = (U32) (&SVC_RESTORE);
    } else {
        // kernel thread LR: return to the entry point of the task
        *(--sp) = (U32) (p_taskinfo->ptask);
        p_tcb->u_stack_hi = p_taskinfo->u_stack_size;    // user stack hi grows downwards
        p_tcb->u_stack_size = p_taskinfo->u_stack_size;
    }

    // kernel stack R0 - R12, 13 registers
    for ( int j = 0; j < 13; j++) {
        *(--sp) = 0x0;
    }

    p_tcb->msp = sp;                        // store msp in TCB
    p_tcb->initial_msp = sp;
    p_tcb->ptask = p_taskinfo->ptask;       // store task entry in TCB
    p_tcb->prio = p_taskinfo->prio;         // store priority in TCB
    p_tcb->priv = p_taskinfo->priv;         // store privilege in
    p_tcb->mailbox = 0;

    if (p_tcb->prio != PRIO_RT) {
        insert(p_tcb);
    }

    return RTX_OK;
}

/**************************************************************************//**
 * @brief       switching kernel stacks of two TCBs
 * @param:      p_tcb_old, the old tcb that was in RUNNING
 * @return:     RTX_OK upon success
 *              RTX_ERR upon failure
 * @pre:        gp_current_task is pointing to a valid TCB
 *              gp_current_task->state = RUNNING
 *              gp_crrent_task != p_tcb_old
 *              p_tcb_old == NULL or p_tcb_old->state updated
 * @note:       caller must ensure the pre-conditions are met before calling.
 *              the function does not check the pre-condition!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @attention   CRITICAL SECTION
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *****************************************************************************/
__asm void k_tsk_switch(TCB *p_tcb_old)
{
        PUSH    {R0-R12, LR}
        STR     SP, [R0, #TCB_MSP_OFFSET]   ; save SP to p_old_tcb->msp
K_RESTORE
        LDR     R1, =__cpp(&gp_current_task);
        LDR     R2, [R1]
        LDR     SP, [R2, #TCB_MSP_OFFSET]   ; restore msp of the gp_current_task

        POP     {R0-R12, PC}
}

__asm void k_tsk_reset()
{
    	PUSH    {R0-R12, LR}
    	LDR     R1, =__cpp(&gp_current_task);
    	LDR     R2, [R1]
		LDR     SP, [R2, #TCB_MSP_OFFSET]   ; restore msp of the gp_current_task
	    POP     {R0-R12, PC}
}

int k_tsk_run_new_no_update(void)
{
    TCB *p_tcb_old = NULL;

    if (gp_current_task == NULL) {
    	return RTX_ERR;
    }

    p_tcb_old = gp_current_task;
    gp_current_task = scheduler();

    if ( gp_current_task == NULL  ) {
        gp_current_task = p_tcb_old;        // revert back to the old task
        return RTX_ERR;
    }

    // at this point, gp_current_task != NULL and p_tcb_old != NULL
    if (gp_current_task != p_tcb_old) {
    	gp_current_task->state = RUNNING;   // change state of the to-be-switched-in  tcb
    	if(p_tcb_old->state != DORMANT &&
    	   p_tcb_old->state != BLK_MSG &&
		   p_tcb_old->state != SUSPENDED){
            p_tcb_old->state = READY;       // change state of the to-be-switched-out tcb
    	}

    	if (gp_current_task->prio == PRIO_RT && gp_current_task->priv == 1 && !gp_current_task->was_suspended) {
    		gp_current_task->msp = gp_current_task->initial_msp;
    		printf("Resetting msp\n\r");
    	}

    	gp_current_task->was_suspended = 0;

        k_tsk_switch(p_tcb_old);            // switch stacks
    }

    return RTX_OK;
}


/**************************************************************************//**
 * @brief       run a new thread. The caller becomes READY and
 *              the scheduler picks the next ready to run task.
 * @return      RTX_ERR on error and zero on success
 * @pre         gp_current_task != NULL && gp_current_task == RUNNING
 * @post        gp_current_task gets updated to next to run task
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @attention   CRITICAL SECTION
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *****************************************************************************/
int k_tsk_run_new(void)
{
    TCB *p_tcb_old = NULL;
    
    if (gp_current_task == NULL) {
    	return RTX_ERR;
    }

    TIMEVAL now = get_system_time();
    update(now);

    p_tcb_old = gp_current_task;
    gp_current_task = scheduler();
    
    if ( gp_current_task == NULL  ) {
        gp_current_task = p_tcb_old;        // revert back to the old task
        return RTX_ERR;
    }

    // at this point, gp_current_task != NULL and p_tcb_old != NULL
    if (gp_current_task != p_tcb_old) {
    	gp_current_task->state = RUNNING;   // change state of the to-be-switched-in  tcb
    	if(p_tcb_old->state != DORMANT &&
    	   p_tcb_old->state != BLK_MSG &&
		   p_tcb_old->state != SUSPENDED){
            p_tcb_old->state = READY;       // change state of the to-be-switched-out tcb
    	}

    	if (gp_current_task->prio == PRIO_RT && gp_current_task->priv == 1 && !gp_current_task->was_suspended) {
    		gp_current_task->msp = gp_current_task->initial_msp;
    		printf("Resetting msp\n\r");
    	}

    	gp_current_task->was_suspended = 0;

        k_tsk_switch(p_tcb_old);            // switch stacks
    }

    return RTX_OK;
}

/**************************************************************************//**
 * @brief       yield the cpu
 * @return:     RTX_OK upon success
 *              RTX_ERR upon failure
 * @pre:        gp_current_task != NULL &&
 *              gp_current_task->state = RUNNING
 * @post        gp_current_task gets updated to next to run task
 * @note:       caller must ensure the pre-conditions before calling.
 *****************************************************************************/
int k_tsk_yield(void)
{
	g_task_count++;
	gp_current_task->task_count = g_task_count;
    return k_tsk_run_new();
}


/*
 *===========================================================================
 *                             TO BE IMPLEMETED IN LAB2
 *===========================================================================
 */

int k_tsk_create(task_t *task, void (*task_entry)(void), U8 prio, U16 stack_size)
{
#ifdef DEBUG_0
    printf("k_tsk_create: entering...\n\r");
    printf("task = 0x%x, task_entry = 0x%x, prio=%d, stack_size = %d\n\r", task, task_entry, prio, stack_size);
#endif /* DEBUG_0 */

    // TODO: `k_tsk_create` is non-blocking, but can be preempted by `scheduler`
    // TODO: check pointers
    // TODO: error checking additional conditions

    // TID pointer NULL
    if (task == NULL){
        return RTX_ERR;
    }

    // Task entry pointer NULL
    if (task_entry == NULL){
        return RTX_ERR;
    }

    // Maximum number of tasks reached
    if (g_num_active_tasks == MAX_TASKS){
        return RTX_ERR;
    }

    if (validate_stack_size(stack_size) == RTX_ERR) {
    	return RTX_ERR;
    }

    // prio invalid
    if (prio != HIGH && prio != MEDIUM && prio != LOW && prio != LOWEST){
        return RTX_ERR;
    }

    RTX_TASK_INFO task_info;
    TCB * tcb = NULL;

    // linear traverse to find free TID in g_tcbs;
    for (int i=0; i < MAX_TASKS; i++){
        // get dormant TCB
        if(g_tcbs[i].state == DORMANT){
            tcb = &g_tcbs[i];
            break;
        }
    }

    // get TID and store in buffer
    *task = tcb->tid;

    // fill in task_info
    task_info.prio = prio;
    task_info.priv = 0;
    task_info.u_stack_size = stack_size;
    task_info.ptask = task_entry;

    // call k_tsk_create_new
    if (k_tsk_create_new(&task_info, tcb, *task) == RTX_ERR){
        return RTX_ERR;
    }

    g_num_active_tasks++;
    //scheduler(); do we need to call?

//    if (currently_running()) {
//    	k_tsk_yield();
//    } else {
//    	return RTX_ERR;
//    }


    return RTX_OK;
}

void k_tsk_exit(void) 
{

#ifdef DEBUG_0
    printf("k_tsk_exit: entering...\n\r");
#endif /* DEBUG_0 */

    gp_current_task->state = DORMANT;

    TCB* p_tcb_old = gp_current_task;
    gp_current_task = scheduler();

    if(p_tcb_old->priv == 0){
    	k_mem_dealloc((void *) ((U32)p_tcb_old->u_stack_hi - p_tcb_old->u_stack_size));
    }

    if (p_tcb_old->mailbox) {
    	k_mem_dealloc((void*)p_tcb_old->mailbox);
    }

    if (p_tcb_old->prio == PRIO_RT) {
    	get_system_time();
    	update(system_time);
    	edf_remove(p_tcb_old->tid);
    	// TODO: Check deadline against timer!
    }

    g_num_active_tasks--;
    remove_id(heap, p_tcb_old->tid);

    if ( gp_current_task == NULL  ) {
        gp_current_task = p_tcb_old;        // revert back to the old task
        return;
    }

    // at this point, gp_current_task != NULL and p_tcb_old != NULL
    if (gp_current_task != p_tcb_old) {
        gp_current_task->state = RUNNING;   // change state of the to-be-switched-in  tcb
        k_tsk_switch(p_tcb_old);            // switch stacks
    }

    return;
}

int k_tsk_set_prio(task_t task_id, U8 prio) 
{
#ifdef DEBUG_0
    printf("k_tsk_set_prio: entering...\n\r");
    printf("task_id = %d, prio = %d.\n\r", task_id, prio);
#endif /* DEBUG_0 */

    // TODO: This function never blocks, but can be preempted. what does this mean?
    // TODO: if try to set null task to prio PRIO_NULL, do I let it or error?

    // prio invalid or PRIO_NULL or PRIO_RT
    if (prio == PRIO_RT || g_tcbs[task_id].prio == PRIO_RT){
        return RTX_ERR;
    }

    // dormant TCB
    if (g_tcbs[task_id].state == DORMANT){
        return RTX_ERR;
    }

    // RT task prio cannot be modified
    if (g_tcbs[task_id].prio == PRIO_RT) {
        return RTX_ERR;
    }

    if (prio == g_tcbs[task_id].prio) {
    	return RTX_OK;
    }

    // valid TID
    if (task_id > 0 && task_id < MAX_TASKS){
        // user-mode task can change prio of any user-mode task
        // user-mode task cannot change prio of kernel task
        // kernel task can change prio of any user-mode or kernel task

        if(gp_current_task->priv == 1){
            g_tcbs[task_id].prio = prio;
        } else {
            if(g_tcbs[task_id].priv == 1){
                return RTX_ERR;
            } else {
                g_tcbs[task_id].prio = prio;
            }
        }

        reset_priority(heap, task_id, prio);

        k_tsk_yield();

        return RTX_OK;
    }else{
        return RTX_ERR;
    }
}

int k_tsk_get(task_t task_id, RTX_TASK_INFO *buffer)
{
#ifdef DEBUG_0
    printf("k_tsk_get: entering...\n\r");
    printf("task_id = %d, buffer = 0x%x.\n\r", task_id, buffer);
#endif /* DEBUG_0 */    

    // error checking
    if (buffer == NULL) {
        return RTX_ERR;
    }
    // Dormant TCB
    if (g_tcbs[task_id].state == DORMANT){
        return RTX_ERR;
    }

    // valid TID excluding 0
    if (task_id > 0 && task_id < MAX_TASKS){
        if (g_tcbs[task_id].prio == PRIO_RT){
            buffer->p_n = edf_array[task_id].info.p_n;
            buffer->rt_mbx_size = edf_array[task_id].info.rt_mbx_size;
        }
        buffer->tid = task_id;
        buffer->prio = g_tcbs[task_id].prio;
        buffer->state = g_tcbs[task_id].state;
        buffer->priv = g_tcbs[task_id].priv;
        buffer->ptask = g_tcbs[task_id].ptask;
        buffer->k_stack_hi = (U32) (&g_k_stacks[task_id+1]);  // kernel stack hi grows downwards
        buffer->k_stack_size = KERN_STACK_SIZE;
        buffer->u_stack_hi = g_tcbs[task_id].u_stack_hi;
        buffer->u_stack_size = g_tcbs[task_id].u_stack_size;
        edf_get(task_id, &buffer->p_n, &buffer->rt_mbx_size);

        buffer->u_sp = * (U32*)((U32) g_tcbs[task_id].msp + 108);

        if (task_id == gp_current_task->tid){
            int regVal = __current_sp();         // store value of SP register in regVal
            buffer->k_sp = regVal;
        }else{
            buffer->k_sp = (U32) g_tcbs[task_id].msp;
        }
        return RTX_OK;

    }else{
        return RTX_ERR;
    }
}

int k_tsk_ls(task_t *buf, int count){
#ifdef DEBUG_0
    printf("k_tsk_ls: buf=0x%x, count=%d\r\n", buf, count);
#endif /* DEBUG_0 */
    return 0;
}

void k_tsk_block(void) {
	if (gp_current_task->state != RUNNING) {
		return;
	}

	gp_current_task->state = BLK_MSG;
	k_tsk_run_new();
}

void k_tsk_unblock (TCB *task) {
	if (!task) { return; }

	if (task->state != BLK_MSG) {
		return;
	}

	task->state = READY;

	// Unblocked task has higher priority
	if (compare(task, gp_current_task)) {
		// Preempt current task

	    TCB *p_tcb_old = NULL;

	    p_tcb_old = gp_current_task;
	    gp_current_task = task;

	    // We move this preempted task to the front of the ready queue
	    // We do not change the fifo counter of the preempted task
	    // So it should remain at the front
	    insert(p_tcb_old);

	    // at this point, gp_current_task != NULL and p_tcb_old != NULL
	    if (gp_current_task != p_tcb_old) {
	    	gp_current_task->state = RUNNING;   // change state of the to-be-switched-in  tcb
	        p_tcb_old->state = READY;       // change state of the to-be-switched-out tcb
	        k_tsk_switch(p_tcb_old);            // switch stacks
	    }
	} else {
		// Add unblocked lower priority task to the back of the ready queue
		// We increment fifo counter so it explicitly goes to the back
		g_task_count++;
		task->task_count = g_task_count;
		insert(task);
	}

	return;
}

/*
 *===========================================================================
 *                             TO BE IMPLEMETED IN LAB4
 *===========================================================================
 */

int k_tsk_create_rt_priv(task_t *tid, TASK_RT *task, int priv)
{
	if (task->p_n.usec % 500 != 0        ||
	    !task->task_entry                ||
		g_num_active_tasks == MAX_TASKS  ||
		validate_stack_size(task->u_stack_size) == RTX_ERR) {
		return RTX_ERR;
	}

	if (g_num_active_tasks == MAX_TASKS) {
		return RTX_ERR;
	}

	RTX_TASK_INFO task_info;
	TCB * tcb = NULL;

	for (int i = 0; i < MAX_TASKS; i++) {
		if (g_tcbs[i].state == DORMANT) {
			tcb = &g_tcbs[i];
			break;
		}
	}

	*tid = tcb->tid;
	task_info.prio = PRIO_RT;
	task_info.priv = priv;
	task_info.u_stack_size = task->u_stack_size;
	task_info.ptask = task->task_entry;

	edf_insert(tcb->tid, *task);

	if (k_tsk_create_new(&task_info, tcb, *tid) == RTX_ERR) {
		return RTX_ERR;
	}

    TCB * calling_task = gp_current_task;
    if (task->rt_mbx_size != 0){
        gp_current_task = tcb;
        if (k_mbx_create(task->rt_mbx_size) == RTX_ERR){
            gp_current_task = calling_task;
            tcb->state = DORMANT;
            return RTX_ERR;
        } else {
            gp_current_task = calling_task;
        }
    }

    g_num_active_tasks ++;

    return RTX_OK;
}

int k_tsk_create_rt(task_t *tid, TASK_RT *task)
{
	return k_tsk_create_rt_priv(tid, task, 0);
}

void k_tsk_done_rt(void) {
#ifdef DEBUG_0
    printf("k_tsk_done: Entering\r\n");
#endif /* DEBUG_0 */

    if(gp_current_task->prio != PRIO_RT){
        return;
    }

    RTX_TASK_INFO info;
    k_tsk_get(gp_current_task->tid, &info);

    // TODO: logic for suspended
    int ret_val = edf_done(gp_current_task->tid);

    U32* sp = (U32*)info.k_stack_hi;

    if (gp_current_task->priv == 0) {
    	// Walk through sp to reset values
    	--sp;
    	*(--sp) = (U32)info.ptask;
    	*(--sp) = info.u_stack_hi;

        for ( int j = 0; j < 13; j++ ) {
            *(--sp) = 0x0;
        }

        --sp;
    } else {
    	*(--sp) = (U32) info.ptask;
    }

    // kernel stack R0 - R12, 13 registers
    for ( int j = 0; j < 13; j++) {
        *(--sp) = 0x0;
    }

    gp_current_task->msp = sp;

    if (ret_val == 0) {
        k_tsk_run_new_no_update();
    } else {
    	int16_t tid = maximum(heap);
    	//int16_t tid = extract_max(heap);
    	if (tid < 0) {
    		k_tsk_reset();
    		return;
    	}

    	TCB* current_tcb = gp_current_task;
    	TCB* max_ready_tcb = &g_tcbs[(U8)tid];

    	if (compare(current_tcb, max_ready_tcb)) {
    		k_tsk_reset();
    		return;
    	}

    	k_tsk_run_new_no_update();
    }


    return;
}

void k_tsk_suspend(struct timeval_rt *tv)
{
#ifdef DEBUG_0
    printf("k_tsk_suspend: Entering\r\n");
#endif /* DEBUG_0 */
    if ((tv->sec == 0 && tv->usec == 0) || tv->usec % 500 != 0) { return; }

    edf_suspend(gp_current_task->tid, *tv);

	k_tsk_run_new_no_update();

    return;
}

/*
 *===========================================================================
 *                             END OF FILE
 *===========================================================================
 */
