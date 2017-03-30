#ifndef _schedule_h_
#define _schedule_h_


typedef struct
{
 int pid;
 int priority;
} PCB;

void init();
int addProcess(PCB* process);
PCB* nextProcess(int *time);
int hasProcess();

#endif
