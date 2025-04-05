// TO-DO:
// add support for > (redicert stdout to a file), < (redirect stdin to a file), >> (append to a file)
// add support for pipes

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_TOKENS 50
#define BUFFER_SIZE 256
#define NUMBER_ELEMENTS(a) (sizeof(a)/sizeof(a[0])) //doesnt work for dinamic arrays

typedef struct BuiltIn BuiltIn;
typedef void (*command_handler)(char **args);

struct BuiltIn {
  const char* name;
  command_handler handler;
  const char *description;
};

void handle_help();

void handle_exit()
{
  exit(0);
}

void handle_cd(char **args)
{
  if(args[1] == NULL)
  {
    printf("cd: missing argument\n");
  }
  else if(chdir(args[1]) != 0)
  {
    perror("cd failed");
  }
  chdir(args[1]);
}

void handle_echo(char **args)
{
  printf("%s\n", args[1]);
}

void handle_clear()
{
  printf("\e[1;1H\e[2J"); 
}

void handle_where()
{
  char cwd[PATH_MAX];
  printf("%s\n", getcwd(cwd, sizeof(cwd)));
};

void handle_mktext(char **args)
{
  if(args[1] != NULL)
  {
    FILE *file;
    char *filename = args[1];

    if(args[2] != NULL)
    {
      char *cd_args[] = {"cd", args[2], NULL};
      handle_cd(cd_args);
    }
    
    size_t len = strlen(args[1]);
    if(strcmp(args[1] + len - 4, ".txt") == 0)
    {
      filename = strcat(args[1], ".txt");
    }

    file = fopen(filename, "w");
    fclose(file);
  }
  else {
    printf("Please provide the name of the file.\n");
  }
}

BuiltIn built_in[] = {
  {"exit", handle_exit, "Exits the shell"},
  {"cd", handle_cd, "Change directory. Args: The path to the location"},
  {"echo", handle_echo, "Display message"},
  {"clear", handle_clear, "Clear the screen"},
  {"help", handle_help, "Show this help message"},
  {"whereami", handle_where, "Shows current directory"},
  {"mktext", handle_mktext, "Creates/Makes an empty text file. Args: 1: Name of the file; [2]: Desired path."}
};

void handle_help()
{
  printf("Available commands:\n");
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    printf("%s - %s \n", built_in[i].name, built_in[i].description);
  }
  printf("\nThe help command only displayes the build-in commands but the shell supports external commands too.\n");
}


// Tokenization of the input
char** tokenization(char *buffer)
{
  char** tokens = malloc((MAX_TOKENS + 1) * sizeof(char*));
  if(!tokens)
  {
    perror("malloc failed for tokens\n");
    exit(EXIT_FAILURE);
  }
  
  int tokenCount = 0;
  char *token = strtok(buffer, " \t");

  while(token != NULL && tokenCount < MAX_TOKENS){
    tokens[tokenCount++] = token;
    token = strtok(NULL, " \t");
  }
  tokens[tokenCount] = NULL;

  return tokens;
}

void running_command(char** tokens)
{
  int is_built = 0;
  int status; 

  if(tokens[0] == NULL) return;

  //handleing built in commands
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    if(strcmp(built_in[i].name, tokens[0]) == 0)
    {
      built_in[i].handler(tokens);
      is_built = 1;
      return;
    }
  }
  
  //handle external commands
  if(is_built == 0)
  {
    pid_t pid = fork();
    if(pid == -1)
    {
      perror("fork failed\n");
    }
    else if(pid == 0)
    {
      signal(SIGINT, SIG_DFL);
      if(setpgid(0, 0) == -1) //setting the child proccess PID to 0
      {
        perror("child setpgid error");
        exit(EXIT_FAILURE);
      }
      if(execvp(tokens[0], tokens) == -1)//executing the command
      {
        perror("");
        printf("%s command doesn't exist\n", tokens[0]);
        exit(EXIT_FAILURE);
      }
    }
    else {
      if(setpgid(pid, pid) == -1)
      {
        perror("parent setpgid error");
      }
      if(tcsetpgrp(STDIN_FILENO, pid) == -1)
      {
        perror("tcsetpgrp error");
      }

      waitpid(pid, &status, 0);  //wait for the child proccess to finish
      
      if(tcsetpgrp(STDIN_FILENO, getpgid(0)) == -1)
      {
        perror("tcsetpgrp restore");
      }
      if(WIFSIGNALED(status))
      {
        printf("\n");
      }
    }

  }
}

int shell_loop()
{
  char cwd[PATH_MAX];

  while(1)
  {
    if (getcwd(cwd, sizeof(cwd)) != NULL) //changes the directory to the current one
    {      
      printf("%s : ", cwd);
      char *input = readline("shell>");
      if(!input) break;
      add_history(input);

      char **tokens = tokenization(input);

      running_command(tokens);
      free(tokens);

    } 
    else {
      perror("getcwd() error!");
      return 1;
    }
  }
  return 0;
}

int main()
{
  setpgid(0, 0); 
  tcsetpgrp(STDIN_FILENO, getpgid(0));
  signal(SIGINT, SIG_IGN); //the parent proccess (the shell) ignores CTRL+C signal
  handle_clear();
  printf("Use the help command to list all commands.\n");
  printf("\n");
  chdir(getenv("HOME")); //changes the current directory to home

  shell_loop();

  return 0;
}
