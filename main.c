#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
  exit(1);
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

void handle_where(char **args)
{
  char cwd[PATH_MAX];
  printf("%s\n", getcwd(cwd, sizeof(cwd)));
};

void handle_mktext(char **args)
{
  FILE *file;
  const char *filename = strcat(args[1], ".txt");
  file = fopen(filename, "w");

  fclose(file);
}

BuiltIn built_in[] = {
  {"exit", handle_exit, "Exits the shell"},
  {"cd", handle_cd, "Change directory. Args: The path to the location"},
  {"echo", handle_echo, "Display message"},
  {"clear", handle_clear, "Clear the screen"},
  {"help", handle_help, "Show this help message"},
  {"whereami", handle_where, "Shows current directory"},
  {"mktext", handle_mktext, "Creates/Makes an empty text file. Args: Name of the file"}
};

void handle_help()
{
  printf("Available commands:\n");
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    printf("%s - %s \n", built_in[i].name, built_in[i].description);
  }

  printf("\nnThe help command only displayes the build-in commands but the shell supports external commands too.\n");
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
  if(tokens[0] == NULL) return;

  //handleing built in commands
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    if(strcmp(built_in[i].name, tokens[0]) == 0)
    {
      built_in[i].handler(tokens);
      return;
    }
  }

  //handeling external commands
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
