#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define NUM_PHILO 5

// function declarations 
int philo(int num);
void grab_forks(int left_fork_id);
void put_away_forks(int left_fork_id);

int forks;
int onset_dinner;

// Left fork and Right fork id distribution: 
// Left fork: According to philosopher's number
// Right fork: According to (num+1)%5

int main() {
  int i, status;
  pid_t philosophers[NUM_PHILO];
  printf("DINNER STARTS NOW!\n");

  for (i = 0; i < NUM_PHILO; i++) { //Allocating the forks only for parent process
    semctl(forks, i, SETVAL, 1);
  }
  onset_dinner = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); // The semflg here is IPC_CREAT | IPC EXCL. We use IPC_EXCL + IPC_CREAT to make sure
  // no existing semaphore set is opened for access

  semctl(onset_dinner, 0, SETVAL, 5); // prevent child processes from beginning to eat

  // give rise to child processes
  // Where child process = philosopher
  for (i = 0; i < NUM_PHILO; i++) {
    int pid = fork();
    if (pid == 0) {
      int ret = philo(i);
      exit(ret);
    } else {
      philosophers[i] = pid;
    }
  }
  for (i = 0; i < NUM_PHILO; i++) { // Wait for child processes to finish
    waitpid(philosophers[i], & status, 0);
  }
  // Properly removing semaphores after usage
  semctl(forks, 0, IPC_RMID, 0);
  semctl(onset_dinner, 0, IPC_RMID, 0);

  return 0;
}

struct sembuf semOPS; // Struct responsible for semaphore opeartions 

// The following actions are done by each philosopher 
int philo(int num) {
  int i, meals;
  semOPS.sem_flg = 0;

  // Dinner begins
  semOPS.sem_op = -1;
  semOPS.sem_num = 0;
  semop(onset_dinner, & semOPS, 1);
  printf("PHILOSOPHER %d ARRIVED TO DINNER\n", num);

  // Waiting for everyone to be ready
  semOPS.sem_op = 0;
  semOPS.sem_num = num;
  semop(onset_dinner, & semOPS, 1);

  while (meals < 1) { // 1 is the maximum number of meals
    // To avoid deadlocks, we will change the order of asking for forks
    int wait_time = rand() % 300000;

    printf("Philosopher %d is thinking\n", num);
    usleep(wait_time);
    printf("Philosopher %d finished thinking\n", num);
    
    grab_forks(num);

    printf("Philosopher %d is eating\n", num);
    usleep(wait_time);
    printf("Philosopher %d finished eating \n", num);

    put_away_forks(num);
    meals++;
  }
  printf("PHILOSOPHER %d LEFT DINNER\n", num);
  exit(num);
}

void grab_forks(int left_fork_id) {
  int right_fork_id = (left_fork_id + 1) % NUM_PHILO;

  if (left_fork_id == NUM_PHILO - 1) { // To avoid deadlock, ask for the right fork if we are the last philosopher
    // Getting the first fork
    semOPS.sem_op = -1;
    semOPS.sem_flg = right_fork_id;
    semop(forks, & semOPS, 1);

    // Getting the second fork
    semOPS.sem_op = -1;
    semOPS.sem_num = left_fork_id;
    semop(forks, & semOPS, 1);
  } else { // Other philosophers
    semOPS.sem_op = -1;
    semOPS.sem_flg = left_fork_id;
    semop(forks, & semOPS, 1);

    semOPS.sem_op = -1;
    semOPS.sem_num = right_fork_id;
    semop(forks, & semOPS, 1);
  }
}
void put_away_forks(int left_fork_id) {
  int right_fork_id = (left_fork_id + 1) % NUM_PHILO;

  // Leave first fork
  semOPS.sem_op = +1;
  semOPS.sem_num = left_fork_id;
  semop(forks, & semOPS, 1);

  // Leave second fork
  semOPS.sem_op = +1;
  semOPS.sem_num = right_fork_id;
  semop(forks, & semOPS, 1);
}
