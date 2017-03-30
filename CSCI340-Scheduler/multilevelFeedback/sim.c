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


/*
Scheduled Process: 0, Priority:0
Scheduled Process: 1, Priority:1
Scheduled Process: 2, Priority:2
Scheduled Process: 3, Priority:0
Scheduled Process: 4, Priority:1
Scheduled Process: 5, Priority:2
Scheduled Process: 6, Priority:0
Scheduled Process: 7, Priority:1
Scheduled Process: 8, Priority:2
Scheduled Process: 9, Priority:0
*/




//
// main - The simulator's main routine
//
int main(int argc, char **argv){
    int processes[1000];
    init();
    int i;
    for(i=0;i<10;i++){
        processes[i]=200;
        int priority = (i+1)%3;
        printf("Scheduled Process: %d, Priority:%d\n", i, priority);
        PCB* proc = (PCB *) malloc(sizeof(PCB));
        proc->pid = i;
        proc->priority=priority;
        proc->age=0;
        addProcess(proc);
    }
    //exit(0);
    PCB* process = 0;
    int count = 0;
    int time = 0;
    while(hasProcess()){
        process = nextProcess(&time);
        if(!process){
            printf("NULL Process, something went wrong in your code.\n");
            exit(1);
        }
        if(time>0){
          for(;time>0;time--){
              age();
              printf("Process %d executed\n", process->pid);
              processes[process->pid]--;
              if(processes[process->pid]<0){
                  printf("Process %d Finished\n", process->pid);
                  break;
              }
              count++;
              if(count%300==0&&count!=0){
                  if(i<1000){
                    processes[i]=200;
                    printf("Scheduled Process: %d, Priority:0\n", i);
                    PCB* proc = (PCB *) malloc(sizeof(PCB));
                    proc->pid = i;
                    proc->priority=0;
                    proc->age=0;
                    addProcess(proc);
                    i++;
                  }
              }
          }
          if(processes[process->pid]>=0){
            addProcess(process);
          }
        }
        else{
            while(processes[process->pid]>=0){
              age();
              printf("Process %d executed\n", process->pid);
              processes[process->pid]--;
              count++;
              if(count%400==0&&count!=0){
                  if(i<1000){
                    processes[i]=100;
                    printf("Scheduled Process: %d, Priority:0\n", i);
                    PCB* proc = (PCB *) malloc(sizeof(PCB));
                    proc->pid = i;
                    proc->priority=0;
                    proc->age=0;
                    addProcess(proc);
                    i++;
                  }
              }
              if(count==1800){
                while(i<30){
                  processes[i]=100;
                  printf("Scheduled Process: %d, Priority:0\n", i);
                  PCB* proc = (PCB *) malloc(sizeof(PCB));
                  proc->pid = i;
                  proc->priority=0;
                  proc->age=0;
                  addProcess(proc);
                  i++;
                }
              }
            }
            printf("Process %d Finished\n", process->pid);
        }
    }


    exit(0); //control never reaches here
}
