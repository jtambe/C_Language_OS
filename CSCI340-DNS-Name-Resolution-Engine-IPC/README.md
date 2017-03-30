CSCI340-DNS-Name-Resolution-Engine-IPC
==================================
##CSCI340 - Operating Systems
Adapted from University of Colorado at Boulder CSCI3753 Assignment


##Introduction
In this assignment you will develop a multi-process application that resolves domain names to IP addresses, similar to the operation preformed each time you access a new website in your web browser. The application is composed of two sub-systems, each with one pool of processes: requesters and resolvers. The sub-systems communicate with each other using a bounded buffer in shared memory.

This type of system architecture is referred to as a Producer-Consumer architecture. It is also used in search engine systems, like Google. In these systems, a set of crawler processors place URLs onto a queue. This queue is then serviced by a set of indexer processes which connect to the websites, parse the content, and then add an entry to a search index. Refer to Figure 1 for a visual description.

![Figure 1](https://github.com/CSUChico-CSCI340/CSCI340-DNS-Name-Resolution-Engine-IPC/raw/master/handout/pa2.png "System Architecture: Basic Idea of Implementation")
Figure 1: System Architecture

##Description
###Name Files
Your application will take as input a set of name files. Names files contain one hostname per line. Each name file should be serviced by a single requester process from the requester process pool.

###Requester Processes
The requester process pool services a set of name files, each of which contains a list of domain names. Each name that is read from each of the files is placed into a bounded buffer in shared memory. You need to use conditional variables, such that the requester will wait while bounded buffer is full.

###Resolver Processes
The second process pool is comprised of a set of **PROCESS_MAX** resolver processes. The resolver process pool services the FIFO queue by taking a name out of the bounded buffer and resolving its IP address. After the name has been mapped to an IP address, the output is written to a line in the results.txt file in the following format:
<pre>
www.google.com,74.125.224.81
</pre>

###Synchronization and Deadlock
Your application should synchronize access to shared resources and avoid deadlock. You are required to use mutexes and conditional variables to meet this requirement. There are at least two shared resources that must be protected: the bounded buffer and the output file. Neither of these resources is thread-safe by default, and you are only required to use conditional variables for the bounded buffer.

###Ending the Program
Your program must end after all the names in each file have been serviced by the application. This means that all the hostnames in all the input files have received a corresponding line in the output file.

##What's Included
Some files are included with this assignment for your benefit. You are not required to use these files, but they may prove helpful.


1. **util.c** and **util.h** These two files contain the DNS lookup utility function. This function abstracts away a lot of the complexity involved with performing a DNS lookup. The function accepts a hostname as input and generates a corresponding dot-formatted IPv4 IP address string as output.
	* Please consult the *util.h* header file for more detailed descriptions of each available function.
2. **input/names*.txt** This is a set of sample name files. They follow the same format as mentioned earlier. Use them to test your program.
3. **results-ref.txt** This result file is a sample output of the IPs for the hostnames from all the **names*.txt** files used as input.
4. **lookup.c** This program represents an un-threaded/single process solution to this assignment. Feel free to use it as a starting point for your program, or as a reference for using the utility functions and performing file i/o in C.
5. **Makefile** A GNU Make makefile to build all the code.

##Additional Specifications
Many of the specifications for your program are embedded in the descriptions above. This section details additional specifications to which you must adhere.

###Program Arguments
Your executable program should be named "multi-lookup". When called, it should interpret the last argument as the file path for the file to which results will be written. All proceeding arguments should be interpreted as input files containing hostnames in the aforementioned format.

An example call involving three input files might look like:
<pre>
multi-lookup names1.txt names2.txt names3.txt result.txt
</pre>

###Limits
If necessary, you may impose the following limits on your program. If the user specifies input that would require the violation of an imposed limit, your program should gracefully alert the user to the limit and exit with an error.
* **MAX_INPUT_FILES**: 10 Files (This is an optional upper-limit. Your program may also handle more files, or an unbounded number of files, but may not be limited to less than 10 input files.)
* **MAX_RESOLVER_PROCESSES**: 10 Processes (This is an optional upper-limit. Your program may also handle more processes, or match the number of processes to the number of processor cores.)
* **MIN_RESOLVER_PROCESSES**: 2 Processes (This is a mandatory lower-limit. Your program may handle more processes, or match the number of processes to the number of processor cores, but must always provide at least 2 resolver processes.)
* **MIN_REQUESTER_PROCESSES**: 1 Processes (This is a mandatory lower-limit. Your program may handle more processes, or match the number of processes to the number of processor cores, but must always provide at least 1 resolver processes.)
* **MAX_NAME_LENGTH**: 1025 Characters, including null terminator (This is an optional upper-limit. Your program may handle longer names, but you may not limit the name length to less than 1025 characters.)
* **MAX_IP_LENGTH**: INET6_ADDRSTRLEN (This is an optional upper-limit. Your program may handle longer IP address strings, but you may not limit the name length to less than INET6_ADDRSTRLEN characters including the null terminator.)

###Error Handling
You must handle the following errors in the following manners:
* **Bogus Hostname**: Given a hostname that can not be resolved, your program should output a blank string for the IP address, such that the output file contains the hostname, followed by a comma, followed by a line return. You should also print a message to stderr alerting the user to the bogus hostname.
* **Bogus Output File Path**: Given a bad output file path, your program should exit and print an appropriate error to stderr.
* **Bogus Input File Path**: Given a bad input file path, your program should print an appropriate error to stderr and move on to the next file.

All system and library calls should be checked for errors. If you encounter errors not listed above, you should print an appropriate message to stderr, and then either exit or continue, depending upon whether or not you can recover from the error gracefully.

##External Resources
You may use the following libraries and code to complete this assignment, as well as anything you have written for this assignment:
* Any functions listed in util.h
* Any functions in the C Standard Library
* Standard Linux pthread functions
* Standard Linux Random Number Generator functions
* Standard Linux i/o functions

If you would like to use additional external libraries, you must clear it with me first. You will not be allowed to use pre-existing thread-safe queue or file i/o libraries since the point of this assignment is to teach you how to make non-thread-safe resources thread-safe.

##What You Must Provide
To receive full credit, you must submit the following items to Turnin by the due date.

* **multi-lookup.c**: Your program, conforming to the above requirements
* **multi-lookup.h**: A header file containing prototypes for any function you write as part of your program.

##Extra Credit
For extra credit you can implement this assignment again using c11 synchronization's mutexes and conditional variables.

##Grading
To received full credit your program must:
* Meet all requirements elicited in this document
* Build with "-Wall" and "-Wextra" enabled, producing no errors or warnings.
* Run without leaking any memory, as measured using valgrind
* Document any resources you use to solve your assignment in the header comment of your file
* Include your name in the header comment of your file

This includes adhering to good coding style practices. To verify that you do not leak memory, I may use *valgrind* to test your program. To install *valgrind*, use the following command:
<pre>
sudo apt-get install valgrind
</pre>
And to use *valgrind* to monitor your program, use this command:
<pre>
valgrind ./multi-lookup text1.txt text2.txt ...... textN.txt results.txt
</pre>
Valgrind should report that you have freed all allocated memory and should not produce any additional warnings or errors.
You can write your code in any environment you like. But you have to make sure that your programs can be compiled and executed on Ubuntu 16.04.

##References
Refer to your textbook and class notes for descriptions of producer/consumer and reader/writer problems and the different strategies used to solve them.
The Internet is also a good resource for finding information related to solving this assignment.

You may wish to consult the man pages for the following items, as they will be useful and/or required to complete this assignment. Note that the first argument to the "man" command is the chapter, insuring that you access the appropriate version of each man page. See example *man man* for more information.


* man 3 pthread_mutex_init()
* man 3 pthread_mutex_destroy()
* man 3 pthread_mutex_lock()
* man 3 pthread_mutex_unlock()
* man 3 pthread_cond_init()
* man 3 pthread_cond_wait()
* man 3 pthread_cond_signal()
* man 3 pthread_cond_broadcast()
* man 3 fopen()
* man 3 fclose()
* man 3 fprintf()
* man 3 fscanf()
* man 3 usleep()
* man 3 random()
* man 3 perror()
* man 2 fork()
* man 1 valgrind
