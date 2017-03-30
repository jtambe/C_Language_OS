#ifndef _schedule_h_
#define _schedule_h_

typedef struct
{
 int pid;
 int priority;
 int age;
} PCB;

void init();
int addProcess(PCB* process);
PCB* nextProcess(int *time);
void age();
int hasProcess();

#endif
