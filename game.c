/*
This source file is used to launch the game and calling the fork to different
processes. After the different processes are created the RunDungeon function is
called with the PIDs of the character classes that were made */

#include "dungeon_info.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>


int main()
{
  int fd;//variable used for the file decriptor
  struct Dungeon *shared_dungeon;
  //creates/opens a new or existing POSIX shared memory object, 0666 if the directory of permissions
  fd = shm_open(dungeon_shm_name, O_CREAT|O_RDWR, 0666);

  ftruncate(fd, sizeof(struct Dungeon));//truncate the file to length of the dungeon struct

  //creates a new mapping in the virtual address space
  shared_dungeon = mmap(NULL, sizeof(struct Dungeon),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  pid_t wizard, rogue;

  sem_t *s1;
  sem_t *s2;

  pid_t barbarian = fork();//first child process created

  sem_unlink(dungeon_lever_one);//removes the semaphore
  sem_unlink(dungeon_lever_two);

  s1 = sem_open(dungeon_lever_one, O_CREAT|O_EXCL,0660, 1);//creates a new semaphore
  s2 = sem_open(dungeon_lever_two, O_CREAT|O_EXCL,0660, 1);
  //value of 1 used so that sem_wait can be called later

  if(barbarian == 0)//if 0 its in this current process
  {
    char *args[] = {"./barbarian", NULL};
    execv(args[0], args);//replaces current process image with barbarian
  }
  else if(barbarian != -1)//only create child processes not grand child
  {
    wizard = fork();
    if(wizard == 0)
    {

    char *args[] = {"./wizard", NULL};
    execv(args[0], args);

    }
    else if(wizard != -1)
    {
      rogue = fork();
      if(rogue == 0)
      {

    	    char *args[] = {"./rogue", NULL};
    	    execv(args[0], args);
      }
    }
  }


  sleep(1);//used to fix timing issues in my project

  RunDungeon(wizard, rogue, barbarian);//runs the dungeon

  if(!shared_dungeon->running)//clean up by termianting the processes
  {
    kill(barbarian, SIGKILL);
    kill(wizard, SIGKILL);
    kill(rogue, SIGKILL);
    sem_close(s1);//closes the semaphore
    sem_close(s2);
  }

    return 0;

}
