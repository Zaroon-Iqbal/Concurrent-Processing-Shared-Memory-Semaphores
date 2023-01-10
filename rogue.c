/*
This is the rogue character class. When the rogue recieves a DUNGEON_SIGNAL it will
attempt to guess a float value to pick a lock. A binary search algorithm was used to
find the correct value
*/
#include "dungeon_info.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <semaphore.h>


struct Dungeon * shared_dungeon;
sem_t *sem;

void handler(int a, siginfo_t * s, void * name)
{
float bottom = 0;
float top = MAX_PICK_ANGLE;
		while(shared_dungeon->trap.direction != '-')//symbol used to indicate success
		{
			shared_dungeon->rogue.pick = (top + bottom) /2;//random vlaue between top and bottom
			if(shared_dungeon->trap.direction == 'u')//needs to be incremented
			{
				bottom = shared_dungeon->rogue.pick + 1;
				shared_dungeon->trap.direction = 't';//indicate a change was made
			}
			else if(shared_dungeon->trap.direction == 'd')//needs to be decremented
			{
					top = shared_dungeon->rogue.pick - 1;
					shared_dungeon->trap.direction = 't';//indicate a change was made
			}
		}
}

void handler2(int a, siginfo_t * s, void * name)
{
	int num = 0;
	while(shared_dungeon->treasure[num] == '\0')
	{
		usleep(100);//helps with the timing issues
		if(shared_dungeon->treasure[num] != '\0')
		{
			if(num <= 3)//make sure doesnt go past range
			{
				shared_dungeon->spoils[num] = shared_dungeon->treasure[num];//gets the treasure
						num++;
			}
		}
	}
}
//similar setup to others
int main(int argc, char * argv[])
{

  int fd;
  fd = shm_open(dungeon_shm_name, O_RDWR, 0666);

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
