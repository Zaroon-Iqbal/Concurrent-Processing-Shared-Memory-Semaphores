/*
This is the barbarian class. When the barbarian recieves a specific signal from the
dungeon, the barbarian copies the integer in the enemys health field into the attack field.
If the barbarian receives the semaphore signal it does a sem_wait to hold the door open for the
rogue class.
*/

#include "dungeon_info.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <semaphore.h>
//made global variables so they can be accessed in multiple functions
struct Dungeon * shared_dungeon;
sem_t *sem;

void handler(int a, siginfo_t * s, void * name)//specific param needed for when using sigaction
{
	  shared_dungeon->barbarian.attack = shared_dungeon->enemy.health;
}

//semaphores signal handler
void handler2(int a, siginfo_t * s, void * name)
{
	sem_wait(sem);//decrements the sem and waits

	while(shared_dungeon->spoils[3] == '\0')//waits until the rogue does its task
	{
		usleep(100);//used to help with timing issues
	}
	sem_post(sem);//releases


}
int main(int argc, char * argv[])
{

	  int fd;
	  fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
	  sem = sem_open(dungeon_lever_one, 1);//has a initial value of 1 so sem_wait can be done

	  shared_dungeon = mmap(NULL, sizeof(struct Dungeon),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	 struct sigaction sa;
		 sa.sa_sigaction = handler;
		 sa.sa_flags = SA_RESTART|SA_SIGINFO;//need SA_RESTART for when using sa_sigaction
		 sigaction(DUNGEON_SIGNAL, &sa, NULL);

	 struct sigaction sa2;
		 sa2.sa_sigaction = handler2;
		 sa2.sa_flags = SA_RESTART|SA_SIGINFO;
		 sigaction(SEMAPHORE_SIGNAL, &sa2, NULL);

  while(!shared_dungeon->running)//done to help w timing issues, waits till dungeon running
  {
	  sleep(1);
  }

  while(shared_dungeon->running)//to make sure the process doesn't end early
  {
	  sleep(1);
  }

  return 0;
}
