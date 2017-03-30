CSCI340 Virtual Paging Assignment
==================================

CSCI340 (Operating Systems)  
California State University, Chico  
Public Code

Slight additions   
By Bryan Dixon - 2014

Adopted from assignment  
By Andy Sayler - 2012  
http://www.andysayler.com

That was adopted from  
assignment by Dr. Alva Couch  
http://www.cs.tufts.edu/~couch/

With help from:  
Junho Ahn - 2012

##Assignment Introduction

All modern operating systems use virtual memory and paging in order to effectively utilize the computer’s memory hierarchy. Paging is an effective means of providing memory space protection to processes, of enabling the system to utilize secondary storage for additional memory space, and of avoiding the need to allocate memory sequentially for each process.

We have studied how virtual memory systems are structured and how the MMU converts virtual memory addresses to physical memory addresses by means of a page table and a translation lookaside buffer (TLB). When a page has a valid mapping from VM address to physical address, we say the page is “swapped in”. When no valid mapping is available, the page is either invalid (a segmentation fault), or more likely, “swapped out”. When the MMU determines that a memory request requires access to a page that is currently swapped out, it calls the operating system’s page-fault handler. This handler must swap-in the necessary page, possibly evicting another page to secondary memory in the process. It then retries the offending memory access and hands control back to the MMU.

As you might imagine, how the OS chooses which page to evict when it has reached the limit of available physical pages (sometime called frames) can have a major effect on the performance of the memory access on a given system. In this assignment, we will look at various strategies for managing the system page table and controlling when pages are paged in and when they are paged out.

##Your Task

The goal of this assignment is to implement a paging strategy that maximizes the performance of the memory access in a set of predefined programs. You will accomplish this task by using a paging simulator that has already been created for you. Your job is to write the paging strategy that the simulator utilizes (roughly equivalent to the role the page fault handler plays in a real OS). Your initial goal will be to create a Least Recently Used paging implementation. You will then need to implement some form of predictive page algorithm to increase the performance of your solution. You will be graded on the throughput of your solution (the ratio of time spent doing useful work vs time spent waiting on the necessary paging to occur).

###2.1 The Simulator Environment
The simulator has been provided for you. You have access to the source code if you wish to review it (simulator.c and simulator.h), but you should not need to modify this code for the sake of this assignment. You will be graded using the stock simulator, so any enhancements to the simulator program made with the intention of improving your performance will be for naught.

The simulator runs a random set of programs utilizing a limited number of shared physical pages. Each process has a fixed number of virtual pages, the process’s virtual memory space, that it might try to access. For the purpose of this simulation, all memory access is due to the need to load program code. Thus, the simulated program counter (PC) for each process dictates which memory location that process currently requires access to, and thus which virtual page must be swapped-in for the process to successfully continue.

The values of the constants mentioned above are available in the simulator.h file. For the purposes of grading your assignment, the default values will be used:
* 20 virtual pages per process (MAXPROCPAGES)
* 100 physical pages (frames) total (PHYSICALPAGES)
* 20 simultaneous processes competing for pages (MAXPROCESSES) 
* 128 memory unit page size (PAGESIZE)
* 100 tick delay to swap a page in or out (PAGEWAIT)

As you can see, you are working in a very resource constrained environment. You will have to deal with attempts to access up to 400 virtual pages (20 processes times 20 virtual pages per process), but may only have, at most, 100 physical pages swapped in at any given time.

In addition, swapping a page in or out is an expensive operation, requiring 100 ticks to complete. A tick is the minimum time measurement unit in the simulator. Each instruction or step in the simulated programs requires 1 tick to complete. Thus, in the worst case where every instruction is a page miss (requiring a swap-in), you will spend 100 ticks of paging overhead for every 1 tick of useful work. If all physical pages are in use, this turns into 200 ticks per page miss since you must also spend 100 ticks swapping a page out in order to make room for the required page to be swapped in. This leads to an “overhead to useful work” ratio of 200 to 1: very, very, poor performance. Your goal is to implement a system that does much better than this worst case scenario.

###2.2 The Simulator Interface

The simulator exports three functions through which you will interact with it. The first function is called pageit. This is the core paging function. It is roughly equivalent to the page-fault handler in your operating system. The simulator calls pageit anytime something interesting happens (memory access, page fault, process completion, etc). It passes the function a page map for each process, as well as the current value of the program counter for each process. See simulator.h for details. You will implement your paging strategy in the body of this function.

The pageit function is passed an array of pentry structs, one per process. This struct contains a copy of all of the necessary memory information that the simulator maintains for each process. You will need the information contained in this struct to make intelligent paging decisions. It is the simulator’s job to maintain this information. You should just read it as necessary. The struct contains:

* long active
	- A flag indicating whether or not the process has completed. 1 if running, 0 if exited.
* long pc
	- The value of the program counter for the process. The current page can be calculated
as page = pc/PAGESIZE.
* long npages
	- The number of pages in the processes memory space. If the process is active (running),
this will be equal to MAXPROCPAGES. If the process has exited, this will be 0.
* long pages[MAXPROCPAGES]
	- A bitmap array representing the page map for a given process. If pages[X] is 0, page X is swapped out, swapping out, or swapping in. If pages[X] is 1, page X is currently swapped in.

The simulator also exports a function called pagein and a function called pageout. These functions request that a specific page for a specific process be swapped in or swapped out, respectively. You will use these functions to control the allocation of virtual and physical pages when writing your paging strategy. Each of these functions returns 1 if they succeed in starting a paging operation, if the requested paging operation is already in progress, or if the requested state already exists. 100 ticks after requesting a paging operation, the operation will complete. When calling pagein, the page maps passed to pageit will reflect the new state of the simulator after the request completes 100 ticks later. When calling pageout, the page maps passed to pageit will reflect the new state of the simulator in the first call to pageit after the request is made. In short, a page is recognized as swapped out as soon as a pageout request is made, but is not recognized as swapped in until after a pagein request completes. These functions return 0 if the paging request can not be processed (due to exceeding the limit of physical pages or because another paging operation is currently in process on the requested page) or if the request is invalid (paging operation requests non- existent page, etc). See Figure 1 for more details on the behavior of pagein and pageout.



![Simulator Page State](https://raw.githubusercontent.com/CSUChico-CSCI340/CSCI340-Virtual-Paging-Assignment/master/writeup/simulator-PageState.png "Figure 1: Simulator Page State")

Figure 1 shows the possible states that a virtual page can occupy, as well as the possible transitions between these states. Note that the page map values alone do not define all possible page states. We must also account for the possible operations currently underway on a page to fully define its state. While the page map for each process can be obtained from the pageit input array of structs, there is no interface to directly reveal any operations underway on a given page. If knowing whether or not a paging operation is underway on a given page (and thus knowing the full state of a page) is necessary for your pageit implementation, you must maintain this data yourself.

###2.3 The Simulated Programs

The simulator populates its 20 processes by randomly selecting processes from a collection of 5 simulated “programs”. Pseudo code for each of the possible 5 programs is provided in Listings 1 through 5.

```
# loop with inner branch
	for 10 30 
		run 500
		if.4
			run 900
		else
			run 131
		endif
	end
	exit
endprog
```
####Listing 1: Test Program 1 - A loop with an inner branch

```
# one loop
	for 20 50
		run 1129
	end
	exit
endprog
```
####Listing 2: Test Program 2 - Single loop

```
# double-nested loop
	for 10 20
		run 1166
		for 10 20
			run 516
		end
	end
endprog
```
#### Listing 3: Test Program 3 - Double Nested Loop

```
#entirely linear
	run 1911
	exit
endprog
```
#### Listing 4: Test Program 4 - Linear

```
#probablistic backward branch
	for 10 20
label:
		run 500
		if .5
			goto label
		endif
	end
	exit
endprog
```
#### Listing 5: Test Program 5 - Probablistic backward branch

This simple pseudo code notation shows you what will happen in each process:
* **for X Y:** A “for” loop with between X and Y iterations (chosen randomly)
* **run Z:** Run Z (unspecified) instructions in sequence
* **if P:** Run next clause with probability P, run else clause (if any) with probability (1-P).
* **goto label:** Jump to “label

As we discuss in the next section, you may wish to use this knowledge about the possible programs to:

1. Profile processes and know which kind of programs each is an instance of.
2. Use this knowledge to predict what pages a process will need in the future with rather
high accuracy.

Note that while you know the structure of these programs, the programs flow is still probabilistic in nature. Which branch a specific process takes or how many loop iterations occur will be dependent upon the random seed generated by the simulator. Thus, you may never be able to perfectly predict the execution of a process, only the probabilistic likelihood of a collection of possible execution paths.

## Some Implementation Ideas

In general, your pageit() implementation will need to follow the basic flow presented in Figure 2. You will probably spend most of your time deciding how to implement the “Select a Page to Evict” element.

A basic “one-process-at-a-time” implementation is provided for you. This implementation never actually ends up having to swap out any pages. Since only one process is allocated pages at a time, no more than 20 pages are ever in use. When each process completes, it releases all of its pages and the next process is allowed to allocate pages and run. This is a very simple solution, and as you might expect, does not provide very good performance. Still, it provides a simple starting point that demonstrates the simulator API. See pager-basic.c for more information.

To start, create some form of “Least Recently Used” (LRU) paging algorithm. An LRU algorithm selects a page that has not been accessed for some time when it must swap a page out to make room for a new page to be swapped in. An LRU algorithm can either operate globally, or with respect to a given process. In the latter case, you may wish to pre-reserve a number of physical pages for each process and only allow each process to compete for pages from this subset. An stub for implementing your LRU version of pageit() has been created for you in the pager-lru.c file. Note the use of static variables in order to preserve local state between calls to pageit(). Your LRU algorithm should perform much better than the trivial solution discussed above, but will still suffer from performance issues. We can do better.

![Reactive pageit flow chart](https://raw.githubusercontent.com/CSUChico-CSCI340/CSCI340-Virtual-Paging-Assignment/master/writeup/pageit-Fig1.png "Figure 2: Basic Reactive pageit() Flow Chart")

To really do well on this assignment, you must create some form of predictive paging algorithm. A predictive algorithm attempts to predict what pages each process will require in the future and then swaps these pages in before they are needed. Thus, when these pages are needed, they are already swapped in and ready to go. The process need not block to wait for the required pages to be swapped in, greatly increasing performance. Figure 3 shows a modified version of the Figure 2 flowchart for a predictive implementation of pageit. As for the LRU implementation, a simple predictive stub has been created for you in the pager-predict.c file.

![Predictive pageit flow chart](https://raw.githubusercontent.com/CSUChico-CSCI340/CSCI340-Virtual-Paging-Assignment/master/writeup/pageit-Fig2.png "Figure 3: Basic Predictive pageit() Flow Chart")

There are effectively two approaches to predictive algorithms. The first approach is to leverage your knowledge of the possible program types (see previous section). In this approach, one generally attempts to heuristically determine which program each process is an instance of by tracking the movement of the process’s program counter (PC). Once each process is classified, you can use further PC heuristics to determine where in its execution the process is, and then implement a paging strategy that attempts to swap in pages required by upcoming program actions before they occur. Since the programs all have probabilistic elements, this approach will never be perfect, but it can do very well.

The second approach to predictive algorithms is to ignore the knowledge you have been given regarding the various program types. Instead, you might track each process’s program counter to try to detect various common patterns (loops, jumps to specific locations, etc). If you detect a pattern, you assume that the pattern will continue to repeat and attempt to swap in the necessary pages touched during the execution of the pattern before the process needs them. Working set algorithms are a subset of this approach.

Note that in any predictive operation, you ideally wish to stay 100-200 ticks ahead of the execution of each process. This is the necessary predictive lead time in which you must make paging decisions in order to insure that the necessary pages are available when the process reaches them and that no blocking time is required. As Figure 3 shows, in addition to swapping in pages predicatively, you must still handle the case where your prediction has failed and are thus forced to reactively swap in the necessary page. This is referred to as a predictive miss. A good predictive algorithm will minimize misses, but still must handle them when they occur. In other words, you can not assume that your predictions will always work and that every currently needed page is already available. Doing so will most likely lead to deadlock.

There are a number of additional predictive notions that might prove useful involving state-space analysis [5], Markov chains [6], and similar techniques. We will leave such solutions to the student to investigate if she wishes. Please see the references section for additional information and ideas.

## What's included

We provide some code to help get you started. Feel free to use it as a jumping off point (appropriately cited).

###Folders

-  handout - Assignment description and documentation
-  writeup - Images for write up README.md file

###Files

-  Makefile - GNU makefile to build all relevant code
-  pager-basic.c - Basic paging strategy implementation that runs one process at a time.  
-  pager-lru.c - LRU paging strategy implementation (you code this).
-  pager-predict.c - Predictive paging strategy implementation (you code this).
-  api-test.c - A pageit() implmentation that tests that simulator state changes
-  simulator.c - Core simualtor code (look but don't touch)
-  simulator.h - Exported functions and structs for use with simulator
-  programs.c - Defines test "programs" for simulator to run
-  pgm*.pseudo - Pseudo code of test programs from which programs.c was generated.

###Executables
-  test-* - Runs simulator using "programs" defined in programs.c and paging strategy defined in pager-*.c. Includes various run-time options. Run with '-help' for details.
-  test-api - Runs a test of the simulator state changes
-  see.R - An R script for displaying a visualization of the process run/block activity in a simulation. You must first run ./test-* -csv to generate the necessary trace files.
	- Before you can run R you need to install it if you don’t already have it installed. On Ubuntu you can do the following to install it:
	```bash
		$ sudo apt-get update
		$ sudo apt-get install r-base
	```
	- To run visualization, lunch R in windowed graphics mode (in Linux: R -g Tk & at the command prompt) from the directory containing the trace files (or use setwd to set your working directory to the directory containing the trace files). Then run source(‘‘see.r’’) at the R command prompt to lunch the visualization.
	- *setwd* requires the working directory you wish to set to follow it in the R command line. To exit from R you just call quit().
	- This is a useful visualization as it’ll allow you to see when your processes have pages in memory and when you don’t. Additionally, will let you see how long each process spends waiting or how long it takes to run for comparison to previous executions or paging implementations.

## What You Must Provide
When you submit your assignment, you must provide the following:

1. A copy of your LRU paging implementation (pager-lru.c)
2. A copy of your best predictive paging implementation (pager-predict.c)

If you need to provide additional code for your solution to work you will need to send an
archived tar package to me via email in addition to submitting code to Turnin. The archived tar package will need to include all of your files and in addition to all of your files should also include the following:

1. A makefile that builds any necessary code
2. A README explaining how to build and run your code

## Grading
100% of your grade will be based on the performance of the best pager implementation that you provide. The following simulation scores will earn the corresponding number of points:
* Code does not compile without errors : 0 Points
* 0.64 <= score < 1.28 : 70 Points
* 0.32 <= score < 0.64 : 75 Points (Basic LRU implementation)
* 0.16 <= score < 0.32 : 80 Points
* 0.08 <= score < 0.16 : 85 Points
* 0.04 <= score < 0.08 : 90 Points
* 0.02 <= score < 0.04 : 95 Points
* 0.01 <= score < 0.02 : 100 Points (Good predictive implementation)
* 0.005 <= score < 0.01 : 100 Points + 5 Points EC
* score < 0.005 : 100 Points + 10 Points EC (Excellent predictive implementation)

During grading, we will run your code using several random seeds and will take the average of these runs as your score. Thus, if your program’s performance varies widely from run-to-run, you may get bitten during grading. In the words of Client Eastwood, “Do I feel lucky?”.

If your code generates warnings when building under gcc on the VM using -Wall and -Wextra you will be penalized 1 point per warning. In addition, to receive full credit your submission must:
* Meet all requirements elicited in this document
* Code must adhere to good coding practices.
* Code must be submitted to Turnin prior to due date.

## Resources
Refer to your textbook and class notes of OS paging policies and implementations.
* If you require a good C language reference, consult K&R[3].
* The internet[4] is also a good resource for finding information related to solving this
assignment.
* You may wish to consult the man pages for the following items, as they will be useful
and/or required to complete this assignment. Note that the first argument to the “man” command is the chapter, ensuring that you access the appropriate version of each man page.
* See man man for more information.
	* man 1 make

## Bash Examples

Build:
```bash
$ make
```

Clean:
```bash
$ make clean
```
View test options:
```bash
$ ./test-* -help
```
Run pager-basic test:
```bash
$ ./test-basic
```
Run API test:
```bash
$ ./test-api
```

## References
1. Andrew Sayler, Junho Ahn, and Richard Han CSCI3753 - PA4. University of Colorado at Boulder: Spring 2012 https://github.com/asayler/CU-CS3753-PA4
2. Couch, Alva. Comp111 - A5. Tufts University: Fall 2011. http://www.cs.tufts.edu/ comp/111/assignments/a5.html.
3. Kernighan, Brian and Dennis, Ritchie. The C Programming Language. Second Edition: 2009. Prentice Hall: New Jersey.
4. Stevens, Ted. Speech on Net Neutrality Bill. 2006. http://youtu.be/f99PcP0aFNE.
5. Wikipedia. “State space (dynamical system)”. Wikipedia, The Free Encyclopedia. 2012. http://en.wikipedia.org/w/index.php?title=State_space_(dynamical_ system)&oldid=478401148. Online; accessed 12-April-2012.
6. Wikipedia. “Markov chain”. Wikipedia, The Free Encyclopedia. 2012. http:// en.wikipedia.org/w/index.php?title=Markov_chain&oldid=486910550. Online; ac- cessed 12-April-2012.
