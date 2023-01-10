/*
This is the wizard character class. When it recieves the signal from the dungeon
the qizard reads the Caesar Cypher placed in the barriers spell field and decodes
it using the first character as the key and copies the decoded message in the Wizards
spell field
*/
#include "dungeon_info.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <ctype.h>
#include <semaphore.h>


struct Dungeon * shared_dungeon;
sem_t *sem;

void handler(int a, siginfo_t * s, void * name)
{
	  char first = shared_dungeon->barrier.spell[0];
	  char res[100];//the maximum size is given of 100
	  first %= 26;//the remainder is the shift value
	  for(int i = 1; i < 100 ; i++)
	    {
		  char remainder = 0;

	     if(isupper(shared_dungeon->barrier.spell[i]))//check if it is upper case letter
	     {
	       res[i] = (shared_dungeon->barrier.spell[i]  - first);
	       if( res[i] < 'A')//if it goes past the alphabet letters
	       {
	    	   remainder = 'A' - res[i] - 1;//one off??
	    	   res[i] = 'Z' - remainder;//raps it around
	       }
	     }
	      else if(islower(shared_dungeon->barrier.spell[i]))//check if lowercase letter
	      {
	               res[i] = shared_dungeon->barrier.spell[i] - first;
	    	       if( res[i] < 'a')//to see if it goes past the letters
	    	       {
	    	    	   remainder = 'a' - res[i] - 1;
	    	    	   res[i] = 'z' - remainder;
	    	       }
	      }
	      else if (shared_dungeon->barrier.spell[i] == ' ')//to make sure spaces are just spaces
	      {
	    	  res[i] = shared_dungeon->barrier.spell[i] ;
	      }
	      else
	      {
	    	  res[i] = shared_dungeon->barrier.spell[i] ;//all other symbols are not changed

	      }//-1 becaue the first letter is not stored, just a shift value
	     shared_dungeon->wizard.spell[i-1] = res[i];//puts the decoded message in wizard
	    }
}
//semaphores signal handler to hold the door open
void handler2(int a, siginfo_t * s, void * name)
{
	sem_wait(sem);
	while(shared_dungeon->spoils[3] == '\0')
	{
		usleep(100);
	}
	sem_post(sem);
}

//similar setup as barbarian
int main()
{
	
 int fd;
fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
sem = sem_open(dungeon_lever_one, 1);

shared_dungeon = mmap(NULL, sizeof(struct Dungeon),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

struct sigaction sa;
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_RESTART|SA_SIGINFO;
	sigaction(DUNGEON_SIGNAL, &sa, NULL);

struct sigaction sa2;
	sa2.sa_sigaction = handler2;
	sa2.sa_flags = SA_RESTART|SA_SIGINFO;
	sigaction(SEMAPHORE_SIGNAL, &sa2, NULL);

while(!shared_dungeon->running)
{
	 sleep(1);
}

while(shared_dungeon->running)
{
	 sleep(1);
}

  return 0;
}
