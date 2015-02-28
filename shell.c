#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 80
#define MAX_JOBS_LIST 100


/* 80 chars per line, per command, should be enough. */

/**
* setup() reads in the next command line, separating it into distinct tokens
* using whitespace as delimiters. setup() sets the args parameter as  
* null - terminated string.
*/

typedef struct history{
	int length;
	int max_length;
	char** historyArr;
}history;

typedef struct backgroundJobs{
	int length;
	pid_t Array[MAX_JOBS_LIST];
}jobs;

history History = {0,10};
jobs bk= {0};

void addJob(pid_t pid){
	if(bk.length == MAX_JOBS_LIST){
		int j =0;
		for(j;j<MAX_JOBS_LIST-1;j++){
			bk.Array[j] = bk.Array[j+1];
		}
		bk.Array[j] = pid;
	}else{
		bk.Array[bk.length] = pid;
		bk.length++;
	}
}

void removeJob(int index){
	
	if(bk.length == MAX_JOBS_LIST){
		bk.Array[MAX_JOBS_LIST] = 0;
		return;
	}

	for(index;index<bk.length;index++){
		bk.Array[index] = bk.Array[index+1];
		bk.length--;
	}
}

void printJobs(){
	int i =0;
	while(i<bk.length){
		printf("%d. %lu\n",i+1,bk.Array[i]);
		i++;
	}
}

void executeJob(char Character){
	int i = Character - 48;
	if(i<=bk.length){
	pid_t PID = bk.Array[i-1];
	
	kill(PID,SIGCONT);
	wait(0);
	removeJob(i-1);
	}
	return;
}

char* removeQuotes(char* string){
	char* tmp;
	int i =0;
	while(*string){
		if((*string)!='\"'){
			*(tmp+i) = *string;
			i++;
		}
		string++;
	}
	*(tmp+i) = '\0';
	return tmp;
}

int getArrayLength(char* array){
	int j =0;
	while(*(array+j) != '\0'){
		j++;
	}
	return j;
}

int isAlphabet(char alphabet){
	if((alphabet>=65 && alphabet<=90) || (alphabet>=97 && alphabet <= 112)){
		return 1;
	}
	return 0;
}

int isHistoryCommand(char* command){
	if(command[0] == 'r' && !isAlphabet(command[1])){
		return 1;
	}else{
		return 0;
	}
}

char* getHistoryCommand(char* command){

	int command_index = 0;
	int command_letter = command[2];
	
	if(!isHistoryCommand(command)){
		return NULL;
	}

	if((command_letter>=97 && command_letter<=122) || (command_letter>=65 && command_letter<=90)){
		int i =History.length-1;
		for(i;i>=0;i--){
			if(History.historyArr[i][0]==command[2]){
				command = History.historyArr[i];
				break;
			}
		}  
		
		if(i==-1){
			printf("No command with the starting letter %c was found\n",command_letter);
			char* invalid_result = "NO_COMMAND";
			return invalid_result;
		}else{
			printf("Command is : %s\n",command);
		}

	}else{
		command_index = History.length - 1;
		command = History.historyArr[command_index];
	}	
			
	return command;
}

void printHistory(int printLength){

	if(printLength > History.length){
		printLength = History.length;
	}	

	int i = History.length - printLength;
	int lastIndex = History.length - 1;

	for(i;i<=lastIndex;i++){
		printf("%d. %s\n",i,History.historyArr[i]);
	}
}

void adjustHistoryLength(){
	int MAX_HISTORY = History.max_length;
	
	char** tmp = (char**)malloc(MAX_HISTORY*2*sizeof(char**));
	int i = 0;

	for(i;i<History.length;i++){
		tmp[i] = History.historyArr[i];
	}

	History.historyArr = tmp;
	History.max_length = MAX_HISTORY*2;
}

void manageHistory(char* command){

	int *length = &History.length;
	int MAX_HISTORY = History.max_length;

	if(*length>=MAX_HISTORY){
		adjustHistoryLength();
	}

	(*length)++;
	History.historyArr[*length-1] = (char*)malloc((getArrayLength(command)-1)*sizeof(char));
	char* tmp_command = History.historyArr[*length -1];
	
	while(*command){
		if(*command == '\n'){
			command++;
			continue;
		}
      	*tmp_command = *command;
      	command++;
      	tmp_command++;
   }

   *tmp_command = '\0';
}


char* setup(char inputBuffer[], char *args[],int *background)
{
	int length, /* # of characters in the command line */
	i, /* loop index for accessing inputBuffer array */
	start, /* index where beginning of next command parameter is */
	ct; /* index of where to place the next parameter into args[] */

	char* command;
	ct = 0;

	/* read what the user enters on the command line */
	int idx;
	if (fgets(inputBuffer,MAX_LINE, stdin)) {
	    if (1 == sscanf(inputBuffer, "%d", &i)) {
	        printf("%d\n",i);
	    }
	}

	size_t size_string = (size_t)idx;
	strncpy(inputBuffer,(const char*)inputBuffer,size_string);

	if(isHistoryCommand(inputBuffer)){
		const char* tmp_string = getHistoryCommand(inputBuffer);
		strncpy(inputBuffer,tmp_string, MAX_LINE);
		length = strlen(inputBuffer);
		inputBuffer[length] = '\n';
		length+=2;	
	}

	if(!strncmp(inputBuffer,"history",7)==0 && !strncmp(inputBuffer,"NO_COMMAND",10)==0){
		manageHistory(inputBuffer);
	}

	start = - 1;	

	if (length == 0)
		exit(0); /* ^d was entered, end of user command stream */

	if (length < 0){
		perror("error reading the command");
		exit(-1);
		/* terminate with error code of - 1 */
	}

	/* examine every character in the inputBuffer */
	for (i=0;i<length;i++) {
		switch (inputBuffer[i]){
			case ' ':
			case '\t' : /* argument separators */
			
			if(start != -1){
				args[ct] = &inputBuffer[start]; /* set up pointer */
				ct++;
			}

			inputBuffer[i] = '\0'; /* add a null char; make a C string */
			start = -1;
			break;

			case '\n': /* should be the final char examined */
			if (start != -1){
				args[ct] = &inputBuffer[start];
				ct++;
			}

			inputBuffer[i] = '\0';

			args[ct] = NULL; /* no more arguments to this command */
			break;

			default : /* some other character */
			if (start == -1 && inputBuffer[i] != '&') 
				start = i;
			
			if (inputBuffer[i] == '&'){
				*background = 1;
				args[ct] = &inputBuffer[i];
				ct++;
				inputBuffer[i] = '\0'; 
			}
		}
	}

	args[ct] = NULL; /* just in case the input line was >80 */

	return command;
}

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background; /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE+1]; /* command line (of 80) has max of 40 arguments */
	//char* history[MAX_HISTORY+1];
	pid_t PID;
	int* status_ptr;
	int j =0;
	int status_children;
	char* command;
	int jobs_list[MAX_JOBS_LIST];
	History.historyArr = (char**)malloc(History.max_length*sizeof(char *));

	int i =0;
	while (1){
		/* Program terminates normally inside setup */

		background = 0;
		j=0;
		for(j;j<bk.length;j++){
			pid_t result = waitpid(bk.Array[j], &status_children, WNOHANG);
			if (result != 0) {
			  removeJob(j); 
			} 
		}

		printf(" COMMAND -> \n");
		command = setup(inputBuffer,args,&background); /* get next command */

		/*
		Since the user will have to enter the jobs command to check the status of a child process,
		this shell will check the status of all child processes in bk.Array and remove the ones which\
		have already died.
		*/

		if((PID=fork())==0){

			if(strncmp(args[0],"pwd",3)==0){
				char* CWD = (char *)malloc(sizeof(char)*1000);
				printf("%s\n",getcwd(CWD,1000));
				exit(1);
			}			

			if(strncmp(inputBuffer,"exit",4)==0){
				exit(1);
			}

			execvp(inputBuffer,args);
			exit(1);
		}else{	

			if(strncmp(inputBuffer,"history",7)==0){
				printHistory(10);
			}		

			if(strncmp(inputBuffer,"cd",2)==0){
				printf("%s\n",args[1]);
				args[1] = removeQuotes(args[1]);
				chdir(args[1]);
				continue;
			}

			if(strncmp(inputBuffer,"jobs",4)==0){
				printJobs();
			}

			if(strncmp(inputBuffer,"fg",2)==0){
				char Character = inputBuffer[3];
				executeJob(Character);
			}

			if(strncmp(inputBuffer,"exit",4)==0){
				exit(1);
			}

			if(background==0){				
				wait(0);
			}else{
				printf("The PID is: %d",PID);
				addJob(PID);
			}
		}
	}
}
