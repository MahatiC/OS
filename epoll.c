// C program to implement eventfd and epoll
// along with pthreads
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <stdbool.h>
 
#define MAX_EVENTS 5

int done = 1;
int efd;
int gv = 0;
bool finish = false;
int abc = 0;
int i = 0;
int fdnum[2];
struct epoll_event *events;
 
//add eventfd to epoll
void add_event(int fdk){
  struct epoll_event ee;
  fdnum[i] = fdk;
  ee.events = EPOLLET;
  ee.events |= EPOLLIN;
  ee.data.fd = fdk;
  epoll_ctl(efd, EPOLL_CTL_ADD, fdk, &ee);
  i++;
  printf("added eventfd %d epoll_ctl_add\n", fdk);
}

// Thread function
void* foo()
{
 
    if (done == 1) {
 
        // create epoll
        done = 2;
        efd = epoll_create(1024);
        printf("Created epoll\n");
        
    }
    else if (done == 2) {

        // create eventfd and add to epoll
        done = 3;
        for (int j = 0; j < 2; j++){
          int k = eventfd(0, EFD_NONBLOCK);
          add_event(k);
        }
        printf("created eventfd\n");
    }
    else if (done == 3) {
 
        // Let's process the events
        done = 4;
        uint64_t val;
        int numevents = 0;
        printf("Entering epoll wait\n");
        while(!finish){
          int r = epoll_wait(efd, events, MAX_EVENTS, 30000);
          if (r > 0 ){
            numevents = r;
            //read(fd, &val, sizeof (val));
            for(int i = 0; i < numevents; i++)
            {
              struct epoll_event *e = events + i;
              read(e->data.fd, &val, sizeof(val));
              printf("fd in epoll_w %d \n", e->data.fd);
            }
          }
          abc++;
        }
        printf("Exiting epoll wait\n");
        printf("abc val %d \n", abc);
    }
    else if(done == 4){
       done = 5;
       sleep(1);
       printf("creating a dummy thread to sleep\n");
    }
    else if (done == 5) {
        done = 6;
        uint64_t value = 1;
        write(fdnum[0], &value, sizeof (value));
        printf("Writing an eventfd value \n");
    }

    else {
        uint64_t val = 1;
        write(fdnum[1], &val, sizeof (val));
        printf("Writing a second eventfd value \n");
    }

 
    printf("Returning thread\n");
 
    return NULL;
}
 
int main()
{
    //malloc events to access epoll events later
    events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*2);
    memset(events, 0, sizeof(struct epoll_event)*2);

   //create threads
    pthread_t tid[6]; 

    for(int i = 0; i < 6; i++){
      pthread_create(&tid[i], NULL, foo, NULL);
      sleep(1);
    }

    //if(gv == 1)
    finish = true;
 
    // wait for the completion of all threads
    for(int i=0; i < 6; i++){
      pthread_join(tid[i], NULL);
    }

   return 0;
}
