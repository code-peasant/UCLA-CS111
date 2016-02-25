#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/mman.h>

//this will be responsible for the signal action
struct sigaction sa;
//this struct will store all the crucial information for a --command
struct process_t{
  int pid;
  int argnum;
  char** str;
};

//this array will be used to indicate the catch-or-not status for 30 signals
//1 for catch, 0 for ignore
int signal_array[30];

//catch the signal 
void catch_signal_sigaction(int signal_index, siginfo_t *s, void * arg)
{
  fprintf(stderr,"%i caught", signal_index);
  exit(signal_index);
}

//ignore the signal
void ignore_signal_sigaction(int signal_index, siginfo_t *si, void *arg)
{
  //when ignore is enabled
  ucontext_t *context = (ucontext_t*)arg;
  context->uc_mcontext.gregs[REG_RIP]++;
}

int main(int argc, char** argv)
{
  struct process_t process_array[30];
  struct rusage usage;
  int process_index = 0;
  int wait_status = 0;
  int c;
  int fd[20];
  int z = 0;
  //initialize all the file descriptors to 0
  while (z < 20)
    {
      fd[z] = -1;
      z++;
    }
  //initialize the signal_array, don't catch any signals in the beginning
  z = 0;
  while (z < 30)
    {
      signal_array[z] = 0;
      z++;
    }
   fd[0] = 0;
   fd[1] = 1;
   fd[2] = 2;
  int fd_index = 0;
  // the variable will indicate the status of option --verbose
  int verbose = 0;
  int profile = 0;
  int file_flag = 0;
  int first_flag = 1;
  double user_time = 0;
  double kernel_time = 0;
  while (1)
  {
    int option_index = 0;
    static struct option long_options[] =
      {
	{"rdonly", required_argument,0,'a' },
	{"wronly", required_argument,0,'b' },
	{"command",required_argument,0,'c'},
	{"verbose", no_argument,0,'d'},
	{"append", no_argument, 0, 'e'},
	{"cloexec", no_argument, 0, 'f'},
	{"creat", no_argument, 0, 'g'},
	{"directory", no_argument, 0, 'h'},
	{"dsync", no_argument, 0, 'i'},
	{"excl", no_argument, 0, 'j'},
	{"nofollow",no_argument, 0, 'k'},
	{"nonblock", no_argument, 0, 'l'},
	{"rsync",no_argument, 0, 'm'},
	{"sync", no_argument, 0, 'n'},
	{"trunc", no_argument, 0, 'o'},
	{"pipe", no_argument, 0, 'p'},
	{"close", no_argument, 0, 'q'},
	{"profile", no_argument, 0, 'r'},
	{"abort", no_argument, 0, 's'},
	{"catch", required_argument, 0, 't'},
	{"ignore", required_argument, 0, 'u'},
	{"default", required_argument, 0, 'v'},
	{"pause",no_argument, 0, 'w'},
	{"wait", no_argument, 0, 'x'},
	{"rdwr", required_argument,0,'y'},
	{0,0,0,0}
      };
    c = getopt_long(argc, argv, "", long_options, &option_index);
    if (c == -1)
      break;
    switch (c)
      {
      case 'w':
	{
	  pause();
	}
      case 'r':
	{
	  profile = 1;
	  break;
	}
      case 'y':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      }
	  if (verbose == 1)
	    {
	      printf("--rdwr %s\n", optarg);
	    }
	  fd[fd_index] = open(optarg, file_flag|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
	  if (fd[fd_index] < 0)
	    {
	      fprintf(stderr,"can't open file %s\n", optarg);
	    }
	  fd_index++;
	  file_flag = 0;
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
	      printf("user_time: %f, kernel_time: %f for %s\n", user_time, kernel_time, optarg);
	      first_flag = 1;
	    }
	  break;
	}
      case 'x':
	{
	  wait_status = 1;
	  break;
	}
	//close
      case 'q':
	{
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000;
	    }
	  if (verbose == 1)
	    printf("--close %s\n", optarg);
	  char* endptr;
	  int fd_num;
	  fd_num = strtol(optarg, &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "--close\n");
	      break;
	    }
	  else
	    {
	      if (profile == 1)
		{
		  getrusage(RUSAGE_SELF, &usage);
		  user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
		  kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
		  printf("user_time: %f, kernel_time: %f for --close %s\n", user_time, kernel_time, optarg);
		}
	      break;
	    }
	}
	//default
      case 'v':
	{
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000;
	    }
	  if (verbose == 1)
	    printf("--default %s\n", optarg);
	  char* endptr;
	  int signum;
	  signum = strtol(optarg, &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "--default\n");
	      break;
	    }
	  else
	    {
	      sa.sa_handler=SIG_DFL;
	      sigaction(signum,&sa, NULL);
	      if (profile == 1)
		{
		  getrusage(RUSAGE_SELF, &usage);
		  user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
		  kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
		  printf("user_time: %f, kernel_time: %f for --default \n", user_time, kernel_time, optarg);
		}
	      break;
	    }
	 }
	//ignore
      case 'u':
	{
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000;
	    }
	  if (verbose == 1)
	    printf("ignore %s\n", optarg);
	  char* endptr;
	  int signum;
	  signum = strtol(optarg, &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "--ignore\n");
	      break;
	    }
	  else
	    {
	      if (profile == 1)
		{
		  getrusage(RUSAGE_SELF, &usage);
		  user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
		  kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
		  printf("user_time: %f, kernel_time: %f for --ignore \n", user_time, kernel_time, optarg);
		}
	      sa.sa_sigaction = ignore_signal_sigaction;
	      sigaction(signum,&sa, NULL);
	      break;
	    }
	}
	//abort
      case 's':
	{
	  if (verbose == 1)
	    printf("abort");
	  int *a = NULL;
	  int b = *a;
	  break;
	}
	//catch
      case 't':
	{
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000;
	    }
	  if (verbose == 1)
	    printf("--catch %s\n",optarg);
	  char* endptr;
	  int signum;
	  signum = strtol(optarg, &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "--catch \n");
	      break;
	    }
	  else
	    {
	      if (profile == 1)
		{
		  getrusage(RUSAGE_SELF, &usage);
		  user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
		  kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
		  printf("user_time: %f, kernel_time: %f for --catch %s\n", user_time, kernel_time, optarg);
		}
	      sa.sa_sigaction = catch_signal_sigaction;
	      sigaction(signum,&sa, NULL);
	      break;
	    }
	}
	//the pipe option
      case 'p':
	{
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000;
	    }
	  if (verbose == 1)
	    printf("--pipe");
	  int pipe_fd[2];
	  pipe(pipe_fd);
	  fd[fd_index] = pipe_fd[0];
	  fd_index++;
	  fd[fd_index] = pipe_fd[1];
	  fd_index++;
	  if (profile == 1)
		{
		  getrusage(RUSAGE_SELF, &usage);
		  user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
		  kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
		  printf("user_time: %f, kernel_time: %f for --pipe %\n", user_time, kernel_time, optarg);
		}
	  break;
	}
	//the cases of file flags
      case 'e':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--append ");
	  file_flag |=O_APPEND;
	  break;
	}
      case 'f':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--cloexec ");
	  file_flag |= O_CLOEXEC;
	  break;
	}
      case 'g':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--creat ");
	  file_flag |= O_CREAT;
	  break;
	}
      case 'h':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--directory ");
	  file_flag |= O_DIRECTORY;
	  break;
	}
      case 'i':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	     }
	  if (verbose == 1)
	    printf("--dsync ");
	  file_flag |= O_DSYNC;
	  break;
	}
      case 'j':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--excl ");
	  file_flag |= O_EXCL;
	  break;
	}
      case 'k':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("--nofollow ");
	  file_flag |= O_NOFOLLOW;
	  break;
	}
      case 'l':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	     }
	  if (verbose == 1)
	    printf("--nonblock ");
	  file_flag |= O_NONBLOCK;
	  break;
	}
      case 'm':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	     }
	  if (verbose == 1)
	    printf("--rsync ");
	  file_flag |= O_RSYNC;
	  break;
	}
      case 'n':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("sync ");
	  file_flag |= O_SYNC;
	  break;
	}
      case 'o':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      first_flag = 0;
	      }
	  if (verbose == 1)
	    printf("trunc ");
	  file_flag |= O_TRUNC;
	  break;
	}
	//end of the cases of file flags
      case 'a':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      }
	  if (verbose == 1)
	    {
	      printf("--rdonly %s\n", optarg);
	    }

	  fd[fd_index] = open(optarg, file_flag|O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH);
	  if (fd[fd_index] < 0)
	    {
	      fprintf(stderr,"can't open file %s\n", optarg);
	    }
	  fd_index++;
	  file_flag = 0;
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
	      printf("user_time: %f, kernel_time: %f for %s\n", user_time, kernel_time, optarg);
	      first_flag = 1;
	    }
	  break;
	}
      case 'b':
	{
	  if (profile == 1 && first_flag == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0;
	      }
	  if (verbose == 1)
	    {
	      printf("--wronly %s\n", optarg);
	    }
	  file_flag |=O_WRONLY;
	  fd[fd_index] = open(optarg, file_flag, S_IWUSR|S_IWGRP|S_IWOTH);
	  if (fd[fd_index] < 0)
	    {
	      fprintf(stderr,"can't open file %s\n", optarg);
	    }
	  fd_index++;
	  file_flag = 0;
	  if (profile == 1)
	    {
	      getrusage(RUSAGE_SELF, &usage);
	      user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0-user_time;
	      kernel_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0-kernel_time;
	      printf("user_time: %f, kernel_time: %f for %s\n", user_time, kernel_time, optarg);
	      first_flag = 1;
	    }
	  break;
	}
      case 'c':
	{
	  //parse the arguments
	  int index = optind - 1;
	  char* ddash = "--";
	  int len = 1;
	  char** str = (char**)malloc(len*sizeof(char*));
	  while ((index < argc) && (strncmp(ddash, argv[index],2) != 0))
	    {
	      str[len-1] = argv[index];
	      len++;
	      index++;
	      str = (char**)realloc(str,len*sizeof(char*));
	    }
	  if (len < 5)
	    {
	      fprintf(stderr,"invalid syntax for --command\n");
	      break;
	    }
	  //load the parse results into the process_t struct
	  
	  optind = index;
	  char* endptr;
	  long i0, i1, i2;
	  
	  //check for syntax errors
	  i2 = strtol(str[2], &endptr, 10);
	  if (!!*endptr || (fd[i2] < 0))
	    {
	      break;
	    }
	  i1 = strtol(str[1], &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "invalid syntax for --command\n");
	      break;
	    }
	  i0 = strtol(str[0], &endptr, 10);
	  if (!!*endptr)
	    {
	      fprintf(stderr, "invalid syntax for --command\n");
	      break;
	    }
	  if (fd[i0] < 0 || fd[i1] < 0)
	    {
	      fprintf(stderr, "invalid file descriptor\n");
	      exit(1);
	    }

	  str[len-1] = NULL;
	  process_array[process_index].str = str;
	  process_array[process_index].argnum = len-1;
	  process_index++;
	  break;
	  }
	//verbose
      case 'd':
	{
	  verbose = 1;
	  break;
	}
      case '?':
	fprintf(stderr, "unrecognized option %s\n", argv[optind-1]);
	break;
	}
  }

    int total_length = process_index;
  //iterator for the each process
  int loop_times = total_length;
  process_index = 0;
  while (loop_times > 0)
    {
	  int len = process_array[process_index].argnum;
	  char** str = process_array[process_index].str;
	  //print out command for --verbose option
	  if (verbose == 1)
	    {
	      int k = 0;
	      printf("--command");
	      while (k < len)
		{
		  printf(" %s", str[k]);
		  k++;
		}
	      printf("\n");
	    }
	  int pid = fork();
	  int has_error = 0;
	  if (pid == 0)
	    {
	      int i0, i1, i2;
	      i0 = atoi(str[0]);
	      i1 = atoi(str[1]);
	      i2 = atoi(str[2]);
	      dup2(fd[i2],2);
	      dup2(fd[i0],0);
	      dup2(fd[i1],1);
	      int k = 0;
	      
	      //close all irrelevant file descriptors
	      while (k < 20)
		{
		  if (k != i0 && k!= i1 && k != i2 && fd[k] >= 0)
		    close(fd[k]);
		  k++;
		}
	      execvp(str[3], &str[3]);
	    }
	  else
	    {
	      process_array[process_index].pid = pid;
	      process_index++;
	    }
	  loop_times--;
    }
  loop_times = total_length;
  int fd_k = 0;
  while (fd_k < 20)
    {
      close(fd[fd_k]);
      fd_k++;
    }
  if (wait_status == 1)
    {
      if (verbose == 1)
	printf("--wait\n");
      while (loop_times>0)
	{
	  int status;
	  int returnpid = waitpid(-1, &status, 0);
	  int i = 0;
	  char** str = NULL;
	  int len;
	  while (i < process_index)
	    {
	      if (returnpid == process_array[i].pid)
		{
		  str = process_array[i].str;
		  len = process_array[i].argnum;
		  break;
		}
	      i++;
	    }
	  printf("%i",status);
	  int j = 3;
	  while (j < len)
	    {
	      printf(" %s", str[j]);
	      j++;
	    }
	  printf("\n");
	  if (str != NULL)
	    free(str);
	  loop_times--;
	}
    }
}
