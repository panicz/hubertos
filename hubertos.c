#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>

#define NELEMS(array) (sizeof(array)/sizeof(array[0]))
#define MAX_TASKS 4

typedef enum {
    EXECUTION_POINT_INITIALIZED = 0,
    EXECUTION_POINT_REENTERED = 1
} EXECUTION_POINT;

typedef struct {
    int id;
    jmp_buf context;
    int sleep;
    void (*code)();
    void *data;
 } task_t;

static task_t tasks[MAX_TASKS];
static int next_task = 0;
static task_t *current_task = NULL;
static jmp_buf scheduler;

#define save(context) setjmp(context)
#define jump(context) longjmp(context, EXECUTION_POINT_REENTERED)

task_t *create_task(void (*code)(), void *data) {
    if(next_task >= NELEMS(tasks)) {
	return NULL;
    }

    task_t *new_task = &tasks[next_task];
    
    new_task->id = next_task++;
    new_task->sleep = 0;
    new_task->code = code;
    new_task->data = data;
    
    if(setjmp(new_task->context) == EXECUTION_POINT_REENTERED) {
	// WATCH OUT: any parameters or local variables do not
	// contain any meaningful values here
	(current_task->code)();
    }
    
    return new_task;
}

#define wait(time)							\
    if(save(current_task->context) == EXECUTION_POINT_INITIALIZED) {	\
	current_task->sleep = time;					\
	jump(scheduler);						\
    }

void some_task() {
    while(1) {
	printf("task %i\n", current_task->id);
	wait(current_task->id + 1);
    }
}

void timer() {
    int i;
    for(i = 0; i < next_task; ++i) {
	if(tasks[i].sleep > 0) {
	    --tasks[i].sleep;
	}
    }
    sleep(1);
}

void proceed(task_t *task) {
    current_task = task;
    if(save(scheduler) == EXECUTION_POINT_INITIALIZED) {
	jump(current_task->context);
    }
}

void schedule() {
    static int i;
    while (1) {	
	for(i = 0; i < next_task; ++i) { // round-robin
	    if(tasks[i].sleep == 0) {
		proceed(&tasks[i]);
	    }
	}
	puts("");
	timer();
    }
}

int main(void)
{
    create_task(some_task, NULL);
    create_task(some_task, NULL);
    create_task(some_task, NULL);
    //create_task(some_task, NULL);
    schedule();
}
