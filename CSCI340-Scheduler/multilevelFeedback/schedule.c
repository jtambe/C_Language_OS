/* Multilevel feedback
Name : Jayesh Tambe
*/


#include "schedule.h"
#include <stdlib.h>
#include <stdio.h>

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
}

int addProcessInQueue(PCB* process, int queueId){
	Node* temp = (Node*)malloc(sizeof(Node));
	temp->proc = process;
	temp->next = NULL;
	temp->prev = NULL;
	if(queueId == 1){
		if(head1 == NULL && tail1 == NULL){
			head1 = tail1 = temp;
			return 0;
		}
		else{
			tail1->next = temp;
			temp->prev = tail1;
			tail1 = temp;
			return 0;
		}
	}
	if(queueId == 2){
		if(head2 == NULL && tail2 == NULL){
			head2= tail2 = temp;
			return 0;
		}
		else{
			tail2->next = temp;
			temp->prev = tail2;
			tail2 = temp;
			return 0;
		}
	}
	return 1;
}



/**
 * Function called every simulated time period to provide a mechanism
 * to age the scheduled processes and trigger feedback if needed.
 */
void age(){

  if(head3 != NULL){
      Node* temp = (Node*)malloc(sizeof(Node));
      temp = head3;
      while(temp != NULL){
    		int currentAge = temp->proc->age;
    		currentAge = currentAge+1;
    		temp->proc->age = currentAge;
    		if(currentAge == 1000){
          temp->proc->priority = 1;
          //temp->proc->age = 0;
          //addProcessInQueue(temp->proc,2);
          addProcess(temp->proc);
          Node* erase = (Node*)malloc(sizeof(Node));
          erase = temp;
          if(erase == head3 && erase == tail3){
            head3 = tail3 = NULL;
            free(erase);
          }
          else if(erase == head3 && erase != tail3){
            head3 = head3->next;
            if(head3 != NULL){ head3->prev = NULL;}
            if(head3 == NULL){ head3 = tail3= NULL;}
            free(erase);
          }
          else if(erase == tail3 && erase != head3){
            tail3 = tail3->prev;
            tail3->next = NULL;
            free(erase);
          }
          else{
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            free(erase);
          }
        }
        //printf("Going through process %d\n", temp->proc->pid);
        temp = temp->next;
      }
	}

	if(head2 != NULL){
      Node* temp = (Node*)malloc(sizeof(Node));
      temp = head2;
      while(temp != NULL){

    		int currentAge = temp->proc->age;
    		currentAge = currentAge+1;
    		temp->proc->age = currentAge;
    		if(currentAge == 1000){
          temp->proc->priority = 0;
          //temp->proc->age = 0;
          //addProcessInQueue(temp->proc,1);
          addProcess(temp->proc);
          Node* erase = (Node*)malloc(sizeof(Node));
          erase = temp;
          if(erase == head2 && erase == tail2){
            head2 = tail2 = NULL;
            free(erase);
          }
          else if(erase == head2 && erase != tail2){
            head2 = head2->next;
            if(head2 != NULL){ head2->prev = NULL;}
            if(head2 == NULL){ head2 = tail2= NULL;}
            free(erase);
          }
          else if(erase == tail2 && erase != head2){
            tail2 = tail2->prev;
            tail2->next = NULL;
            free(erase);
          }
          else{
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            free(erase);
          }
        }
        //printf("Going through process %d\n", temp->proc->pid);
        temp = temp->next;
      }
	}

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
	temp->prev = NULL;
	if(process->priority == 0){
		if(head1 == NULL && tail1 == NULL){
			head1 = tail1 = temp;
			return 0;
		}
		else{
			tail1->next = temp;
			temp->prev = tail1;
			tail1 = temp;
			return 0;
		}
	}
	if(process->priority == 1){
		if(head2 == NULL && tail2 == NULL){
			head2= tail2 = temp;
			return 0;
		}
		else{
			tail2->next = temp;
			temp->prev = tail2;
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
			temp->prev = tail3;
			tail3 = temp;
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
    returnProc->age = 0;
		//*time = 4;
		head1 = head1->next;
		if(head1==NULL){tail1=NULL;}
		free(temp);
		return returnProc;
	}
	if(QueueId == 2){
		Node * temp = head2;
		PCB * returnProc;
		returnProc = head2->proc;
    returnProc->age = 0;
		*time = 4;
		head2 = head2->next;
		if(head2==NULL){tail2=NULL;}
		free(temp);
		return returnProc;
	}
	if(QueueId == 3){
		Node * temp = head3;
		PCB * returnProc;
		returnProc = head3->proc;
    returnProc->age = 0;
		*time = 1;
		head3 = head3->next;
		if(head3==NULL){tail3=NULL;}
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
	count = (count+1)%3;
	//PCB * returnProc;
	if(head1 == NULL && head2 == NULL && head3 == NULL ){
		return NULL;
	}
  if(head1 != NULL){
    //while(head1 != NULL){
  		return getProcess(1,time);
  	//}
  }
  if(head1 == NULL && head2 != NULL){
    //while(head2 != NULL){
      return getProcess(2,time);
    //}
  }
  if(head1 == NULL && head2 == NULL && head3 != NULL){
    //while(head3 != NULL){
      return getProcess(3,time);
    //}
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
	if(head1 == NULL && head2 == NULL && head3 == NULL ){
		return 0;
	}
	else{
		return 1;
	}
}
