#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_TOKENS 50
#define BUFFER_SIZE 256
#define NUMBER_ELEMENTS(a) (sizeof(a)/sizeof(a[0])) //doesnt work for dinamic arrays

// Built-in commands
const char *built_in[] = {"exit", "cd", "echo", "clear"};

// Tokenization of the input
char** tokenization(char *buffer)
{
  char** tokens = malloc((MAX_TOKENS + 1) * sizeof(char*));
  if(!tokens)
  {
    perror("malloc failed for tokens");
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
  //verifing if the command requested is build-in or external
  // this can *maybe* be improved with verifing the string itself in the built-in global variable
  int found_index = -1;
  for(size_t i = 0; i < NUMBER_ELEMENTS(built_in); i++)
  {
    if(strcmp(built_in[i], tokens[0]) == 0)
    {
      found_index = i;
      break;
    }
  }

  if(found_index != -1)
  {
    switch(found_index)
    {
      case 0: //exit
        exit(0);

      case 1: //cd
        chdir(tokens[1]);
        break;

      case 2: // echo 
        printf("%s", tokens[1]);
        break;

      case 3: // clear
        printf("\e[1;1H\e[2J"); //clears the screen using regex, using system("clear") would be counter intuitive for this project
        break;

      default:
        break;
      
    }
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
  printf("Use the help command to list all commands.\n");
  chdir(getenv("HOME"));

  shell_loop();

  return 0;
}
