/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>
 * Name: Jayesh Tambe
 * Login: jtambe
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"



/* Global variables */
// environment variable pointer to array of strings
// called environment
extern char **environ;



char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */



/* Function prototypes */
int KILL(pid_t , int );
int Sigemptyset(sigset_t*);
int Sigaddset(sigset_t *, int);
int Sigprocmask(int, sigset_t*, void*);
pid_t Fork(void);

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);


int KILL(pid_t pid, int signal){
  // send signal integer to given process id
  // or group of processes when pid is passed in as negative
  int status = kill(pid,signal);
  if((status < 0)){
    unix_error("Error in KILL");
  }
  return status;
}


int Sigemptyset(sigset_t* set){
  // initializes the signal set given by set to empty,
  // with all signals excluded from the set.
  // returns 0 on success
  int status = sigemptyset(set);
  if((status)){
    unix_error("Error in Sigemptyset");
  }
  return status;
}


int Sigaddset(sigset_t* set, int signal){
  // add given signal number in set
  // returns 0 on success
  int status = sigaddset(set, signal);
  if((status)){
    unix_error("Error in Sigaddset");
  }
  return status;
}


int Sigprocmask(int action, sigset_t* set, void * t){
  // used to fetch and/or change the signal mask of the calling thread
  // The signal mask is the set of signals whose delivery is
  // currently blocked for the caller
  // returns 0 on success
  // action is carried out on signal set
  int status = sigprocmask(action, set, NULL);
  if((status)){
    unix_error("Error in Sigprocmask");
  }
  return status;
}


int Setpgid(int pid, int pgid){
  // sets the PGID of the process specified by pid to pgid
  // setpgid(pid_t pid, pid_t pgid)
  // returns 0 on success
  int status = setpgid(pid, pgid);
  if((status < 0)){
    unix_error("Error in Setpgid");
  }
  return status;
}

pid_t Fork(void){
  pid_t pid;
  pid = fork();
  // On success, the PID of the child process is returned in the parent,
  // and 0 is returned in the child
  if(pid < 0){
      unix_error("Error in Fork");
  }
  return pid;
}


/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline)
{
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
  //
  char *argv[MAXARGS];
  pid_t pid;
  // structure to contain set of signals
  sigset_t mask;
  struct job_t * jd;


  //
  // The 'bg' variable is TRUE if the job should run
  // in background mode or FALSE if it should run in FG
  //
  int bg = parseline(cmdline, argv);

  // generate empty signal set
  Sigemptyset(&mask);
  // add SIGCHLD to signal set to be blocked
  Sigaddset(&mask, SIGCHLD);
  // add SIGINT to signal set to be blocked
  Sigaddset(&mask, SIGINT);
  // add SIGTSTP to signal set to be blocked
  Sigaddset(&mask, SIGTSTP);


  if (argv[0] == NULL)
    return;   /* ignore empty lines */

  // check if command is built in
  if(builtin_cmd(argv) == 0){
    // blocking signal set created above using SIG_BLOCK
    Sigprocmask(SIG_BLOCK, &mask, NULL);
    printf("Command is %s\n", argv[0]);

    if((pid = Fork()) == 0){
      // unblocking signal set created above using SIG_BLOCK if it is child
      Sigprocmask(SIG_UNBLOCK, &mask, NULL);
      printf("In child command: %s\n", argv[0]);
      // creating new process ids for new jobs
      // otherwise it would terminate shell
      Setpgid(0,0);
      // executes a program pointed by argv[0]
      // argv is array of argument strings passed to executable program
      // environ is pinter to array of
      // strings called environment defined in lib.c
      if(execve(argv[0], argv, environ) < 0){
        //if execution was unsuccessful
        printf("Command is %s:\n", argv[0]);
        exit(0);
      }
      else{
        printf("Command is %s:\n", argv[0]);
      }
    }
    else{
      printf("In parent, command: %s\n", argv[0]);
    }
    if(bg == 0){
      // add process in job list
      printf("In parent, jobs: {%d}\n", pid);
      addjob(jobs, pid, FG, cmdline);
      // unblock set of signals once process is added in list
      Sigprocmask(SIG_UNBLOCK, &mask, NULL);
      // allow the job in the foreground to finish
      // pid = parent in this case so parents waits for foreground job to finish
      waitfg(pid);
    }
    else{
      // add process in job list
      addjob(jobs, pid, BG, cmdline);
      // unblock set of signals once process is added in list
      Sigprocmask(SIG_UNBLOCK, &mask, NULL);
      // get the job details in job_t structure
      // by using pid so that detaisl can be printed
      jd = getjobpid(jobs, pid);
      // print the job details
      printf("[%d] (%d) %s", jd->jid, jd->pid, jd->cmdline);
    }


  }

  return;
}


/////////////////////////////////////////////////////////////////////////////
//
// builtin_cmd - If the user has typed a built-in command then execute
// it immediately. The command name would be in argv[0] and
// is a C string. We've cast this to a C++ string type to simplify
// string comparisons; however, the do_bgfg routine will need
// to use the argv array as well to look for a job number.
//
int builtin_cmd(char **argv)
{

  if(strcmp(argv[0],"quit") == 0){
    // used to get out of program
    exit(0);
  }
  else if(strcmp(argv[0],"jobs") == 0){
    // print the jobs
    listjobs(jobs);
    return 1;
  }
  else if(strcmp(argv[0],"fg") == 0){
    // execute the command
    do_bgfg(argv);
    return 1;
  }
  else if(strcmp(argv[0],"bg") == 0){
    // execute the command
    do_bgfg(argv);
    return 1;
  }
  else {
    // else it is not built in command
    return 0;
  }


  return 0;     /* not a builtin command */
}

/////////////////////////////////////////////////////////////////////////////
//
// do_bgfg - Execute the builtin bg and fg commands
//
void do_bgfg(char **argv)
{
  struct job_t *jobp=NULL;

  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return;
  }

  /* Parse the required PID or %JID arg */
  if (isdigit(argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = getjobpid(jobs, pid))) {
      printf("(%d): No such process\n", pid);
      return;
    }
  }
  else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = getjobjid(jobs, jid))) {
      printf("%s: No such job\n", argv[1]);
      return;
    }
  }
  else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return;
  }

  //
  // You need to complete rest. At this point,
  // the variable 'jobp' is the job pointer
  // for the job ID specified as an argument.
  //
  // Your actions will depend on the specified command
  // so we've converted argv[0] to a string (cmd) for
  // your benefit.
  //

  // sending signal to the group using negative pid from job pointer
  KILL(-jobp->pid, SIGCONT);

  // strcmp returns 0 if strings match
  if(strcmp(argv[0],"bg") == 0){
    // change job state to gackground
    jobp->state = BG;
    // print details of the job
    printf("[%d] (%d) %s",jobp->jid,jobp->pid,jobp->cmdline);
  }
  else{
    // change job state to foreground
    jobp->state = FG;
    // allow the job to finish
    waitfg(jobp->pid);
  }

  return;
}



/////////////////////////////////////////////////////////////////////////////
//
// waitfg - Block until process pid is no longer the foreground process
//
void waitfg(pid_t pid)
{
  // get job using process pid
  struct job_t * foreground_job = getjobpid(jobs,pid);
  if(foreground_job == 0){
    return;
  }
  // use sleep to hold till pid process is FG process
  while(foreground_job->pid == pid && foreground_job->state == FG){
    sleep(1);
    printf("Process (%d) \n", foreground_job->pid);
  }

  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// Signal handlers
//


/////////////////////////////////////////////////////////////////////////////
//
// sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
//     a child job terminates (becomes a zombie), or stops because it
//     received a SIGSTOP or SIGTSTP signal. The handler reaps all
//     available zombie children, but doesn't wait for any other
//     currently running children to terminate.
//
void sigchld_handler(int sig){

  pid_t child_pid;
  int status;

  // -1 : wait for any child process
  // WNOHANG : return immediately if no child has exited
  // returns 0 if no child has exited
  // WUNTRACED : return if a child has stopped
  while((child_pid = waitpid(-1,&status,WNOHANG|WUNTRACED)) >0){
    struct job_t * jd = getjobpid(jobs,child_pid);
    if(!jd){
      printf("((%d): No such child", child_pid);
      return;
    }
    // returns true if child process was stopped by a signal
    if(WIFSTOPPED(status)){
      // change the state of job
      jd->state = ST;

      printf("Job [%d] (%d) stopped by signal 20 \n", jd->jid, child_pid);
      //printf("Job [%d] (%d) stopped by signal %d \n", jd->jid, child_pid, WSTOPSIG(status));
    }
    // returns true if child process was terminated by a signal
    else if(WIFSIGNALED(status)){
      // delete terminated process
      deletejob(jobs, child_pid);
      printf("Job [%d] (%d) terminated by signal 2 \n", jd->jid, child_pid);
      //printf("Job [%d] (%d) terminated by signal %d \n", jd->jid, child_pid, WTERMSIG(status));
    }
    // returns true if child process terminated normally
    else if(WIFEXITED(status)){
      // delete terminated process
      deletejob(jobs, child_pid);
      //printf("Job [%d] (%d) terminated normally with exit status %d \n", jd->jid, child_pid, WEXITSTATUS(status));
    }
    else{
      unix_error("Error in waitp_id" );
    }

  }

  return;

}


/////////////////////////////////////////////////////////////////////////////
//
// sigint_handler - The kernel sends a SIGINT to the shell whenver the
//    user types ctrl-c at the keyboard.  Catch it and send it along
//    to the foreground job.
//
void  sigint_handler(int sig){
  pid_t foregroundpid;
  // get current foreground job
  foregroundpid = fgpid(jobs);

  if(foregroundpid > 0){
    // sending signal to group of processes by using process group id
    KILL(-foregroundpid,SIGINT);
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
//     the user types ctrl-z at the keyboard. Catch it and suspend the
//     foreground job by sending it a SIGTSTP.
//
void sigtstp_handler(int sig){
  pid_t foregroundpid;
  // get current foreground job
  foregroundpid = fgpid(jobs);

  if(foregroundpid > 0){
    // sending signal to group of processes by using process group id
    KILL(-foregroundpid,SIGTSTP);
  }
  return;
}

/*********************
 * End signal handlers
 *********************/


// Pages referred
//http://www.tutorialspoint.com/unix_system_calls/waitpid.htm
