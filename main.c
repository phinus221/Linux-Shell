#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

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


// Tokenization of the inputf (chdir("/tmp") != 0)
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
  int is_external = 0;
  int status;

  if(tokens[0] == NULL) return;

  //handleing built in commands
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    if(strcmp(built_in[i].name, tokens[0]) == 0)
    {
      is_built = 1;
      built_in[i].handler(tokens);
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
      int i = 0;

      while(tokens[i] != NULL)
      {
        if(execvp(tokens[0], &tokens[i]) == -1)
        {
          perror("\n");
          is_external = 0;
        }

        i++;
      }
    }
    else {
      waitpid(pid, &status, 0);  
      is_external = 1;
    }

  }

  if(is_built == 0 && is_external == 0)
  {
    printf("%s command doesn't exist.\n", tokens[0]);
  }
}

int shell_loop()
{
  char buffer[BUFFER_SIZE]; 
  char cwd[PATH_MAX];

  while(1)
  {
    if (getcwd(cwd, sizeof(cwd)) != NULL) 
    {
      printf("shell : %s>", cwd);
      fflush(stdout);

      if(!fgets(buffer, BUFFER_SIZE, stdin))
      {
        printf("\n");
        break;
      }

      //change how the input is read, cant use arrows to move the cursor. consider changing fgets() with read()

      buffer[strcspn(buffer, "\n")] = 0;

      char **tokens = tokenization(buffer);

      running_command(tokens);
      free(tokens);

    } else {
      perror("getcwd() error!");
      return 1;
    }
  }
  return 0;
}

int main()
{
  handle_clear();
  printf("Use the help command to list all commands.\n");
  printf("\n");
  chdir(getenv("HOME"));

  shell_loop();

  return 0;
}
