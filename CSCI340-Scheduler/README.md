#Writing a Scheduler
California State University - Chico

By Bryan Dixon

##Introduction
The purpose of this assignment is for you gain insight into how schedulers work on the system.

##Logistics
The only “hand-in” will be electronic. Any clarifications and revisions to the assignment will be posted here and emailed out to the class.

##Hand Out Instructions

I recommend you use an Ubuntu Linux virtual machine to complete this assignment. Alternatively, you can use the jaguar machines or your native Linux install.

You should download the project as a zip or clone it locally to the protected directory (the *lab directory*) in which you plan to do your work. Assuming you wish to put it in your home directory and are there:

<pre>
~$ git clone git@github.com:CSUChico-CSCI340/CSCI340-Scheduler.git
#Will now have folder CSCI340-Scheduler with files
$ cd CSCI340-Scheduler/
$ ls
LICENSE			images/			multilevelRR/		simpleRR/
README.md		multilevelFeedback/	simple/
</pre>

As you can see there are 5 folders contained in this repo. The images folder you can ignore as this is a holder for the images in the README. The other four folders contain the starter API code and simulator to test your code for each of the four schedulers you will be writing in this assignment.

To start, you should add your name in a header comment at the top of each *schedule.c*.

Looking at each *schedule.c* file, you will see that it contains a rudimentary scheduler API to add a process, remove a process, and get the next process from your scheduler. It will your job to figure out how each scheduler works and add the appropriate code to make the given scheduler schedule correctly.

I would recommend implementing these in the order discussed later in this document as to some extent they will build upon the concepts of the earlier schedule as they get more complicated.


##General Overview of Schedulers
Taken from Wikipedia[1]
In computer science, scheduling is the method by which threads, processes or data flows are given access to system resources (e.g. processor time, communications bandwidth). This is usually done to load balance and share system resources effectively or achieve a target quality of service.  The need for a scheduling algorithm arises from the requirement for most modern systems to perform multitasking (executing more than one process at a time) and multiplexing (transmit multiple data streams simultaneously across a single physical channel).

The scheduler is concerned mainly with:

* Throughput - The total number of processes that complete their execution per time unit.
* Latency, specifically:
	* Turnaround time - total time between submission of a process and its completion.
	* Response time - amount of time it takes from when a request was submitted until the first response is produced.
* Fairness - Equal CPU time to each process (or more generally appropriate times according to each process’ priority and workload).
* Waiting Time - The time the process remains in the ready queue.

In practice, these goals often conflict (e.g. throughput versus latency), thus a scheduler will implement a suitable compromise. Preference is given to any one of the above mentioned concerns depending upon the user’s needs and objectives. In real-time environments, such as embedded systems for automatic control in industry (for example robotics), the scheduler also must ensure that processes can meet deadlines; this is crucial for keeping the system stable. Scheduled tasks can also be distributed to remote devices across a network and managed through an administrative back end.



##Your Task
The task for this assignment is to implement the scheduler APIs provided to you in the schedule.c files for the following schedulers:

* Simple - A simple FCFS scheduler
* Simple Round Robin - A simple Round Robin scheduler with a quanta of 4 time units.
* Multi Level Round Robin - A variant of a MultiLevel priority scheduler using Round Robin schedulers. This scheduler iterates through the different levels starting with the highest priority (priority 0) and choses the process next in the queue for that priority level to schedule for a specified quanta or time. The higher the priority the more quanta is given. Your implementation should mirror the quanta and number of priorities shown in Figure 1.  

![MultiLevel Queue](https://raw.githubusercontent.com/CSUChico-CSCI340/CSCI340-Scheduler/master/images/multilevel.png "MultiLevel Queue")

Figure 1: Multi Level Round Robin Priority Scheduler

* Multi Level Feedback - A MultiLevel priority scheduler with feedback. This queue will consist of a FCFS scheduler for the highest priority tasks and two round robin queues for the lower priority tasks. High priority will be represented with 0 increasing in value as the priority gets lower. Your implementation should mirror the number of priorities and implementation in Figure 2. Additionally, after a task hasn't been scheduled for 1000 time cycles your process should then be added to the higher priority scheduler.

![MultiLevel Feedback Queue](https://raw.githubusercontent.com/CSUChico-CSCI340/CSCI340-Scheduler/master/images/multilevelfeedback.png "MultiLevel Feedback Queue")

Figure 2: Multi Level Feedback Priority Scheduler

You aren’t allowed to import any libraries not already provided in the
schedule.c file.

##Data Structures in C
As you can’t include any libraries for data structures you’ll likely want to implement your own data structure to implement the FCFS scheduler. As such you’ll need to do this in C code. As an example here is a simple implementation of a linked list in C using structs:

<pre>
#include <stdio.h>
#include <stdlib.h>
struct node {
	int value;
	struct node* next;
};

int main(){
	int i;
	/* This will be the unchanging first node */
	struct node* root;
	/* cur node for manipulating linked list */
	struct node* cur;
	/*
 	 * Now root points to a node struct
 	 * Dynamic allocation of memory the size of
 	 * a node in C (similar to new node in C++)
  	 */
	root = (struct node*) malloc( sizeof(struct node));
	cur = root;
	for(i=0; i<10;i++){
		cur->value = i;
		cur->next = (struct node*) malloc( sizeof(struct node));
		cur = cur->next;
	}
	//Create final values
	cur->value = 10;
	cur->next = NULL;
	/* Now let’s print it out */
	cur=root;
	while(cur){
		printf("%d ", cur->value);
		cur=cur->next;
	}
	printf("\n");
	/* Let’s free up memory */
	while(root){
		cur = root;
		root = root->next;
		free(cur); //like delete in C++
	}
}
</pre>

##Checking Your Work
I have provided some tools to help you check your work.

* **Reference solution.** - I’ve included a reference output file with the expected solution for each scheduler you need to write. Your program should produce identical output to that of the ref.out file.


##Hints

* Read the [CPU Scheduling](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf) and [Multi-Level Feedback Scheduling](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-mlfq.pdf) sections from the online textbook.

##Evaluation
Your solution will be tested agains the reference output. You will get full credit if your assignment reproduces the reference output. Grades for this assignment will be assigned as follows:

* 10% - Simple Scheduler working
* 20% - Simple Round Robin working
* 30% - Multi-Level Round Robin working
* 40% - Multi-Level Feedback working

So if you get all of them working you'll get 100%.

##Hand In Instructions
You only have to change schedule.c. You need to upload schedule.c to the https://turnin.ecst.csuchico.edu/ page to mark your completion time. There will be a turnin submission for each of the different schedulers so make sure you submit to the correct scheduler submission.

##References
1. Wikipedia. “Scheduling (computing)”. Wikipedia, The Free Encyclopedia. 2012. http://en.wikipedia.org/wiki/Process_scheduler. Online; accessed 16-February-2014.
