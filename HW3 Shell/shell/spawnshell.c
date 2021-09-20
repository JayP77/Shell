/* $begin shellmain */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <fcntl.h>


#define MAXARGS 128
#define MAXLINE 8192 /* Max text line length */

typedef enum { IS_SIMPLE, IS_PIPE, IS_INPUT_REDIR, IS_OUTPUT_REDIR, IS_INPUT_OUTPUT_REDIR, IS_SEQ } Mode; /* simple command, |, >, <, ; */
typedef struct { 
    char *argv[MAXARGS]; /* Argument list */
    int argc; /* Number of args */
    int bg; /* Background job? */
    Mode mode; /* Handle special cases | > < ; */
} parsed_args; 

extern char **environ; /* Defined by libc */

/* Function prototypes */
void eval(char *cmdline);
parsed_args parseline(char *buf);
int builtin_command(char **argv, pid_t pid, int status);
void signal_handler(int sig);
int exec_cmd(parsed_args pa, posix_spawn_file_actions_t *actions, pid_t *pid, int *status);
int find_index(char** argv, char* target); 

void unix_error(char *msg) /* Unix-style error */
{
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(EXIT_FAILURE);
}

int main() {
  char cmdline[MAXLINE]; /* Command line */
  /* TODO: register signal handlers */
  signal(SIGINT, signal_handler);
  signal(SIGTSTP, signal_handler);

  while (1) {
    char *result;
    /* Read */
    printf("CS361 > "); /* TODO: correct the prompt */
    result = fgets(cmdline, MAXLINE, stdin);
    if (result == NULL && ferror(stdin)) {
      fprintf(stderr, "fatal fgets error\n");
      exit(EXIT_FAILURE);
    }

    if (feof(stdin)) exit(EXIT_SUCCESS);

    /* Evaluate */
    eval(cmdline);
  }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) {
  char buf[MAXLINE];   /* Holds modified command line */
  pid_t pid;  
  pid_t pid2;  
  //int pid1, pid2;         /* Process id */
  int status;          /* Process status */
  posix_spawn_file_actions_t actions, actions2; /* used in performing spawn operations */
  posix_spawn_file_actions_init(&actions); 
  posix_spawn_file_actions_init(&actions2);
  //parsed_line.argv2
  int pipe_fds[2]; 
  strcpy(buf, cmdline);
  parsed_args parsed_line = parseline(buf);
 
  int i = 0;
  int j;
  //int j;
  //int temp = 0;
// char *argv2[MAXLINE];
  pipe(pipe_fds);
  if (parsed_line.argv[0] == NULL) return; /* Ignore empty lines */

  /* Not a bultin command */
  if (!builtin_command(parsed_line.argv, pid, status)) {
    switch (parsed_line.mode) {
      case IS_SIMPLE: /* cmd argv1 argv2 ... */
        if (!exec_cmd(parsed_line, &actions, &pid, &status)) return;
        break;
      case IS_PIPE: /* cmd | cmd */
        // TODO: handle pipe 

        
        j = find_index(parsed_line.argv, "|");
          
        
        posix_spawn_file_actions_adddup2(&actions, pipe_fds[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions,pipe_fds[0]);
      
        posix_spawn_file_actions_adddup2(&actions2, pipe_fds[0], STDIN_FILENO);
        posix_spawn_file_actions_addclose(&actions2,pipe_fds[1]);
        //char** argv2 = parsed_line.argv + (i+1);
       // char **argv2 = parsed_line.argv;

        //argv2 =  argv2 + i + 1;
      // printf("%s", argv2[0]);
      // printf("\n");
      //  printf("%s", argv2[1]);
      // printf("\n");
      //char* sp = " ";

//       char* argv3 = strcat(argv2[0]," ");
//       argv3 = strcat(argv3,argv2[2]);
// printf("%s" ,"argv3");
// printf("%s",argv3);
        
        // j = i;
        // temp = j;
        parsed_line.argv[j] = NULL;
        
        //printf("%d",parsed_line.argc);
        // while(j != 0){
        // if(parsed_line.argc != 3){
        //   while(temp != i){
        //       //if(temp+1 != i){
        //         argv2[0] = strcat(argv2[0]," ");
        //         argv2[0] = strcat(argv2[0],parsed_line.argv[temp]);
        //         temp++;
        //       //}
        //       // else {
        //       //   *argv2[0] = strcat(argv2[0],);
        //       // }
        //   }

        //   temp = i;
        //   while(temp != parsed_line.argc){
        //      argv2[1] = strcat(argv2[0]," ");
        //         argv2[1] = strcat(argv2[0],parsed_line.argv[temp]);
        //         temp++;
        //   }
        // }
        // printf("%s", argv2[0]);
        // printf("%s", argv2[1]);

        if(0 != posix_spawnp(&pid,  parsed_line.argv[0], &actions,NULL, parsed_line.argv, environ)){
          perror("spawn failed");
          exit(1);
        }
        // j = j-1;

        // while(j != parsed_line.argc){
        // }
          if(0 != posix_spawnp(&pid2,  parsed_line.argv[j+1], &actions2,NULL, &parsed_line.argv[j+1], environ)){
          perror("spawn failed");
          exit(1);
        }
        // j = j + 1;
        // }
        
         //ls -ltr | grep spawn
         //ls | grep spawn
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        waitpid(pid, &status, 0);
        waitpid(pid2, &status, 0);
        //printf("pipe not yet implemented :(\n");
        break;
      case IS_OUTPUT_REDIR: /* cmd > cmd */

            i = find_index(parsed_line.argv, ">");
            posix_spawn_file_actions_addopen(&actions,STDOUT_FILENO, parsed_line.argv[i+1],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            parsed_line.argv[i] = '\0';
            posix_spawnp(&pid, parsed_line.argv[0],&actions,NULL, parsed_line.argv,environ);
            if (!parsed_line.bg){
              if (waitpid(pid, &status, 0) < 0) unix_error("waitfg: waitpid error");
              //return 1;
            }

          
       
        //printf("output redirection not yet implemented :(\n");
        break;
      case IS_INPUT_REDIR: /* cmd < cmd */
        // TODO: handle input redirection 
        //posix_spawn_file_actions_adddup2(&actions,pipe_fds[0],STDIN_FILENO);
        //for(int i = 0; parsed_line.argv[i] != NULL; i++){
       
        posix_spawn_file_actions_addopen(&actions,STDIN_FILENO, parsed_line.argv[2],O_RDONLY , 0);
        posix_spawnp(&pid, parsed_line.argv[0],&actions,NULL, parsed_line.argv,environ);
        if (!parsed_line.bg){
            if (waitpid(pid, &status, 0) < 0) unix_error("waitfg: waitpid error");
        //return 1;
        }
        //}
        //printf("input redirection not yet implemented :(\n");
        break;
      case IS_INPUT_OUTPUT_REDIR: /* cmd < cmd > file */
        // TODO: handle input output redirection 
         posix_spawn_file_actions_addopen(&actions,STDIN_FILENO, parsed_line.argv[2],O_RDONLY , 0);
        posix_spawnp(&pid, parsed_line.argv[0],&actions,NULL, parsed_line.argv,environ);

         i = find_index(parsed_line.argv, ">");
            posix_spawn_file_actions_addopen(&actions,STDOUT_FILENO, parsed_line.argv[i+1],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            parsed_line.argv[i] = '\0';
            posix_spawnp(&pid, parsed_line.argv[0],&actions,NULL, parsed_line.argv,environ);
            if (!parsed_line.bg){
              if (waitpid(pid, &status, 0) < 0) unix_error("waitfg: waitpid error");
            }
       
       //waitpid(pid, &status, 0);

        printf("input output redirection not yet implemented :(\n");
        break;
      case IS_SEQ: /* cmd ; cmd */
        // TODO: handle sequential 
    
         j = find_index(parsed_line.argv, ";");
          
        
        posix_spawn_file_actions_adddup2(&actions, pipe_fds[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions,pipe_fds[0]);
      
        posix_spawn_file_actions_adddup2(&actions2, pipe_fds[0], STDIN_FILENO);
        posix_spawn_file_actions_addclose(&actions2,pipe_fds[1]);

        
        parsed_line.argv[j] = NULL;

        if(0 != posix_spawnp(&pid,  parsed_line.argv[0], &actions,NULL, parsed_line.argv, environ)){
          perror("spawn failed");
          exit(1);
        }
          if(0 != posix_spawnp(&pid,  parsed_line.argv[j+1], &actions2,NULL, &parsed_line.argv[j+1], environ)){
          perror("spawn failed");
          exit(1);
        }
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        waitpid(pid, &status, 0);
        printf("sequential commands not yet implemented :(\n");
        break;
    }
    if (parsed_line.bg)
      printf("%d %s", pid, cmdline);
    
  }
  posix_spawn_file_actions_destroy(&actions);
  return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv, pid_t pid, int status) {
  if (!strcmp(argv[0], "exit")) /* exit command */
    exit(EXIT_SUCCESS);
  if (!strcmp(argv[0], "&")) /* Ignore singleton & */
    return 1;
  if (!strcmp(argv[0], "?")) {
    // TODO: implement special command "?"
   // printf("\npid:%d status:%d\n");
    printf("\npid:");
    printf("%d", pid);
    printf(" status:");
    printf("%d",status);
    printf("\n");
  } 
  return 0; /* Not a builtin command */
}
/* $end eval */

/* Run commands using posix_spawnp 
   Hint: use this as an example for running commands with posix_spawnp */
int exec_cmd(parsed_args pa, posix_spawn_file_actions_t *actions, pid_t *pid, int *status) {
  posix_spawnp(pid, pa.argv[0],actions,NULL, pa.argv,environ);
  //printf("simple command not yet implemented :(\n");
  // Lab 5 TODO: use posix_spawnp to execute commands 
 // exec_cmd(argv[1]);
   
  // Lab 5 TODO: when posix_spawnp is ready, uncomment me
  if (!pa.bg)
    if (waitpid(*pid, status, 0) < 0) unix_error("waitfg: waitpid error");
  return 1;
}
/* $end exec_cmd */

/* signal handler */
void signal_handler(int sig) {
  // TODO: handle SIGINT and SIGTSTP signals
  char out[] = "caught sigint\n";
  char out2[] = "caught sigtstp\n";
  if(sig == SIGINT) {
    write(STDOUT_FILENO,out,sizeof(out));
  }
  else if(sig == SIGTSTP) {
     write(STDOUT_FILENO,out2,sizeof(out2));
  }
}

/* finds index of the matching target in the argumets */
int find_index(char** argv, char* target) {
  for (int i=0; argv[i] != NULL; i++)
    if (!strcmp(argv[i], target))
      return i;
  return 0;
}

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
parsed_args parseline(char *buf) {
  char *delim; /* Points to first space delimiter */
  parsed_args pa;

  buf[strlen(buf) - 1] = ' ';   /* Replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* Ignore leading spaces */
    buf++;

  /* Build the argv list */
  pa.argc = 0;
  while ((delim = strchr(buf, ' '))) {
    pa.argv[pa.argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')){ /* Ignore spaces */
      buf++;
    }
  }
  pa.argv[pa.argc] = NULL;

  if (pa.argc == 0){ /* Ignore blank line */
    return pa;
  }

  /* Should the job run in the background? */
  if ((pa.bg = (*pa.argv[pa.argc - 1] == '&')) != 0) pa.argv[--pa.argc] = NULL;

  /* Detect simple pipes 
     Hint: use this as an example for more complex pipes */
  pa.mode = IS_SIMPLE;
  if (find_index(pa.argv, "|"))
    pa.mode = IS_PIPE;
  else if(find_index(pa.argv, ";")) 
    pa.mode = IS_SEQ; 
  else {
    if(find_index(pa.argv, "<")) 
      pa.mode = IS_INPUT_REDIR;
    if(find_index(pa.argv, ">")){
      if (pa.mode == IS_INPUT_REDIR)
        pa.mode = IS_INPUT_OUTPUT_REDIR;
      else
        pa.mode = IS_OUTPUT_REDIR; 
    }
  }

  return pa;
}
/* $end parseline */
