# hubertos

Non-preemptive multitasking for embedded systems
using setjmp/longjmp

The caveats are that the task functions should not use any automatic variables (that is, all variables should be either global or preferably static), and system calls (currently the "wait" macro) can only be used from within the tasks main functions.
