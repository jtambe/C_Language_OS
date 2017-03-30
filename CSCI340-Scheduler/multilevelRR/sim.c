//
//
//
// <Put your name and ID here>
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#include "schedule.h"





//
// main - The simulator's main routine
//
int main(int argc, char **argv){
    int processes[14];
    init();
    int i;
    for(i=0;i<10;i++){
        processes[i]=100;
        int priority = i%4;
        printf("Scheduled Process: %d, Priority:%d\n", i, priority);
        PCB* proc = (PCB *) malloc(sizeof(PCB));
        proc->pid = i;
        proc->priority=priority;
        addProcess(proc);
    }
    PCB* process = NULL;
    int count = 0;
    int time = 0;
    while(hasProcess()){
        process = nextProcess(&time);
        if(!process){
            printf("NULL Process, something went wrong in your code.\n");
            exit(1);
        }
        for(;time>0;time--){
            printf("Process %d executed\n", process->pid);
            processes[process->pid]--;
            if(processes[process->pid]<0){
                printf("Process %d Finished\n", process->pid);
            }
            count++;
        }
        if(processes[process->pid]>=0){
            addProcess(process);
        }
        if(count==400){
            for(;i<14;i++){
                processes[i]=100;
                int priority = i%4;
                printf("Scheduled Process: %d, Priority:%d\n", i, priority);
                PCB* proc = (PCB *) malloc(sizeof(PCB));
                proc->pid = i;
                proc->priority=priority;
                addProcess(proc);
            }
        }
    }


    exit(0); //control never reaches here
}
