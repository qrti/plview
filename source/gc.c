#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static int (*gc)(void) = NULL;
static unsigned short gc_enable = 1;

int gc_run(void) 
{
	if(gc != NULL){
		if(gc() == 0)
			return EXIT_SUCCESS;
		else
			return EXIT_FAILURE;
	} 
	else{
		return EXIT_SUCCESS;
	}
}

void gc_handler(int sig) 
{
	switch(sig){
		case SIGSEGV:
		    printf("error: segmentation fault\n");
		    exit(EXIT_FAILURE);
		    
		case SIGBUS:    
    		printf("error: buserror\n");
    		exit(EXIT_FAILURE);		    
		    
		case SIGILL:
		case SIGABRT:
		case SIGFPE:
		    printf("gc signal SIGFPE not evaluated\n");
		    break;    
		    
		default:
		    ;
	}

	if(((sig==SIGINT || sig==SIGTERM || sig==SIGTSTP) && gc_enable==1) || (!(sig==SIGINT || sig==SIGTERM || sig==SIGTSTP) && gc_enable==0)){
		if(sig == SIGINT)
			printf("received interrupt signal, stopping pilight...\n");
		else if(sig == SIGTERM)
			printf("received terminate signal, stopping pilight...\n");
		else
			printf("received stop signal, stopping pilight...\n");
		
		gc_enable = 0;
		gc_run();
	}
}

void gc_attach(int (*fp)(void)) 
{
	if(gc != NULL)
		printf("error: multiple calls to gc_attach\n");
	
	gc = fp;
}

void gc_catch(void) 
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = gc_handler;
	sigemptyset(&act.sa_mask);
	
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	sigaction(SIGABRT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);

	sigaction(SIGBUS,  &act, NULL);
	sigaction(SIGILL,  &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGFPE,  &act, NULL);
	
}
