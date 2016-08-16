#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char **tokenize(char *line, int &token_size)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  token_size = tokenNo;
  return tokens;
}


void cd(char **tokens,int token_size){
  if(token_size > 2){
    printf("Arguments more than expected. cd Takes only one arrgument\n");
  }
  else{
    int status = chdir(tokens[1]);
    if(status == -1){
      // 
      printf("Error. No such file or directory");
    }
  }
}





void  main(void)
{
     char  line[MAX_INPUT_SIZE];            
     char  **tokens;              
     int i;
     int token_size;

     while (1) {           
       
       printf("Hello>");     
       bzero(line, MAX_INPUT_SIZE);
       gets(line);           
       printf("Got command %s\n", line);
       line[strlen(line)] = '\n'; //terminate with new line
       tokens = tokenize(line,token_size);
   
        if(strcmp(tokens[0],"cd") == 0){
          cd(tokens,token_size);
        }

        
        


        // pid_t parent = getpid();
        // pid_t pid = fork();

        // if (pid == -1)
        // {
        //   printf("Error,failed to fork\n");
        //     // error, failed to fork()
        // } 
        // else if (pid > 0)
        // {
        //     int status;
        //     waitpid(pid, &status, 0);
        // }
        // else 
        // {
        //     // we are the child
        //       execve("a.out",NULL,NULL);
        //     _exit(EXIT_FAILURE);   // exec never returns
        // }


       // Freeing the allocated memory	
       for(i=0;tokens[i]!=NULL;i++){
	 free(tokens[i]);
       }
       free(tokens);
     }
     

}

                
