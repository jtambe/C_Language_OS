#include "schedule.h"
#include <stdlib.h>

typedef struct Node{
  PCB* proc;
  // must write struct here even though typedef is used
  // because definition of struct Node is not completed yet
  struct Node* next;
  struct Node* prev;
}Node;

Node * head1;
Node * tail1;
Node * head2;
Node * tail2;
Node * head3;
Node * tail3;
Node * head4;
Node * tail4;
int count = -1;


/**
 * Function to initialize any global variables for the scheduler.
 */
void init(){
	head1 = NULL;
  tail1 = NULL;
	head2 = NULL;
	tail2 = NULL;
	head3 = NULL;
  tail3 = NULL;
	head4 = NULL;
  tail4 = NULL;
}

/**
 * Function to add a process to the scheduler
 * @Param process - Pointer to the process control block for the process that
 * 			needs to be scheduled. PCB is defined in the header.
 * @return true/false response for if the addition was successful
 */
int addProcess(PCB* process){

	Node* temp = (Node*)malloc(sizeof(Node));
	temp->proc = process;
	temp->next = NULL;
	if(process->priority == 0){
		if(head1 == NULL && tail1 == NULL){
			head1 = tail1 = temp;
			return 0;
		}
		else{
			tail1->next = temp;
			tail1 = temp;
			return 0;
		}
	}
	if(process->priority == 1){
		if(head2 == NULL && tail2 == NULL){
			head2 = tail2 = temp;
			return 0;
		}
		else{
			tail2->next = temp;
			tail2 = temp;
			return 0;
		}
	}
	if(process->priority == 2){
		if(head3 == NULL && tail3 == NULL){
			head3 = tail3 = temp;
			return 0;
		}
		else{
			tail3->next = temp;
			tail3 = temp;
			return 0;
		}
	}
	if(process->priority == 3){
		if(head4 == NULL && tail4 == NULL){
			head4 = tail4 = temp;
			return 0;
		}
		else{
			tail4->next = temp;
			tail4 = temp;
			return 0;
		}
	}
	return 1;
}


PCB* getProcess(int QueueId, int * time){

	if(QueueId == 1){
		Node * temp = head1;
		PCB * returnProc;
		returnProc = head1->proc;
		*time = 4;
		head1 = head1->next;
		if(head1==NULL){tail1=NULL;}
		free(temp);
		return returnProc;
	}
	if(QueueId == 2){
		Node * temp = head2;
		PCB * returnProc;
		returnProc = head2->proc;
		*time = 3;
		head2 = head2->next;
		if(head2==NULL){tail2=NULL;}
		free(temp);
		return returnProc;
	}
	if(QueueId == 3){
		Node * temp = head3;
		PCB * returnProc;
		returnProc = head3->proc;
		*time = 2;
		head3 = head3->next;
		if(head3==NULL){tail3=NULL;}
		free(temp);
		return returnProc;
	}
	if(QueueId == 4){
		Node * temp = head4;
		PCB * returnProc;
		returnProc = head4->proc;
		*time = 1;
		head4 = head4->next;
		if(head4==NULL){tail4=NULL;}
		free(temp);
		return returnProc;
	}
	return NULL;
}

/**
 * Function to get the next process from the scheduler
 * @Param time - pass by reference variable to store the quanta of time
 * 		the scheduled process should run for
 * @Return returns pointer to process control block that needs to be executed
 * 		returns NULL if there is no process to be scheduled.
 */
PCB* nextProcess(int *time){

	count = (count+1)%4;
	//PCB * returnProc;

	if(head1 == NULL && head2 == NULL && head3 == NULL && head4 == NULL ){
		return NULL;
	}
	if(count == 0){
			if(head1 != NULL){ return getProcess(1, time);}
			if(head2 != NULL){ count = (count+1)%4;return getProcess(2, time);}
			if(head3 != NULL){ count = (count+2)%4;return getProcess(3, time);}
			if(head4 != NULL){ count = (count+3)%4;return getProcess(4, time);}
	}
	if(count == 1){
			if(head2 != NULL){ return getProcess(2, time);}
			if(head3 != NULL){ count = (count+1)%4;return getProcess(3, time);}
			if(head4 != NULL){ count = (count+2)%4;return getProcess(4, time);}
			if(head1 != NULL){ count = (count+3)%4;return getProcess(1, time);}
	}
	if(count == 2){
			if(head3 != NULL){ return getProcess(3, time);}
			if(head4 != NULL){ count = (count+1)%4;return getProcess(4, time);}
			if(head1 != NULL){ count = (count+2)%4;return getProcess(1, time);}
			if(head2 != NULL){ count = (count+3)%4;return getProcess(2, time);}
	}
	if(count == 3){
			if(head4 != NULL){ return getProcess(4, time);}
			if(head1 != NULL){ count = (count+1)%4;return getProcess(1, time);}
			if(head2 != NULL){ count = (count+2)%4;return getProcess(2, time);}
			if(head3 != NULL){ count = (count+3)%4;return getProcess(3, time);}
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
	if(head1 == NULL && head2 == NULL && head3 == NULL && head4 == NULL ){
    return 0;
  }
  else{
    return 1;
  }
}
