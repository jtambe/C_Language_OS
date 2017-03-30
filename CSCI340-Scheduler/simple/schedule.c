#include "schedule.h"
#include <stdlib.h>

typedef struct Node{
  PCB* proc;
  // must write struct here even though typedef is used
  // because definition of struct Node is not completed yet
  struct Node* next;
  struct Node* prev;
}Node;

Node * head;
Node * tail;


/**
 * Function to initialize any global variables for the scheduler.
 */
void init(){
  head = NULL;
  tail = NULL;
}

/**
 * Function to add a process to the scheduler
 * @Param PCB * - pointer to the PCB for the process/thread to be added to the
 *      scheduler queue
 * @return true/false response for if the addition was successful
 */
int addProcess(PCB *process){

    Node* temp = (Node*)malloc(sizeof(Node));
    temp->proc = process;
    temp->next = NULL;

    if(head == NULL && tail == NULL){
      head = tail = temp;
      return 0;
    }
    else{
      tail->next = temp;
      tail = temp;
      return 0;
    }
    return 1;
}

/**
 * Function to get the next process from the scheduler
 *
 * @Return returns the Process Control Block of the next process that should be
 *      executed, returns NULL if there are no processes
 */
PCB* nextProcess(){

  Node * temp = head;
  if(head == NULL){
    return NULL;
  }
  else if(head == tail){
    PCB * returnProc;
    returnProc = head->proc;
    head = tail = NULL;
    free(temp);
    return returnProc;
  }
  else{
    PCB * returnProc;
    returnProc = head->proc;
    head = head->next;
    free(temp);
    return returnProc;
  }
	return NULL;
}

/**
 * Function that returns a boolean 1 True/0 False based on if there are any
 * processes still scheduled
 * @Return 1 if there are processes still scheduled 0 if there are no more
 *		scheduled processes
 */
int hasProcess(){
  if(head == NULL && tail == NULL){
    return 0;
  }
  else{
    return 1;
  }
}
