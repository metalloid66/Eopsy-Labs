#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define NUM_PHILO 5

int foodTotal = 15;

int statePhilo[NUM_PHILO];

// mutex setup and thread identifiers 
pthread_mutex_t m;
pthread_cond_t ready;
pthread_t philoThread[NUM_PHILO];

// function declarations 
void *philo(void *num);
void grab_forks(int, int, int);
void put_away_forks(int, int, int);
void tryEat(int);

// Left fork and Right fork distribution: 
// Left fork: According to (i + 5 - 1 ) % 5
// Right fork: According to (i + 1 )% 5

int main(int argc, char **argv) {

  pthread_mutex_init(&m, NULL);
  pthread_cond_init(&ready, NULL);
  
  int i;
  // creating and initializing threads by calling philo()
  for (i = 0; i < NUM_PHILO; i++) {
    pthread_create(&philoThread[i], NULL, philo, (void *) i);
  }
  // waiting for thread exit 
  for (i = 0; i < NUM_PHILO; i++) {
    pthread_join(philoThread[i], NULL);
  }
  pthread_exit(NULL);
  return 0;
}

void *philo(void *num) {

  int i, philo_id, left_fork, right_fork, f;
  philo_id = (int) num;

  statePhilo[philo_id] = THINKING;
  printf("Philosopher %d is thinking\n", philo_id);
  sleep(1);
  
  right_fork = (philo_id + 1) % NUM_PHILO;
  left_fork = (philo_id + (NUM_PHILO - 1)) % NUM_PHILO;

  while (foodTotal > 0) {
    grab_forks(philo_id, left_fork, right_fork);
    sleep(1);
  }
  return (NULL);
}

void grab_forks(int phil, int left_fork, int right_fork) {
  pthread_mutex_lock(&m);
  statePhilo[phil] = HUNGRY; // initial state is HUNGRY
  tryEat(phil);
  if (statePhilo[phil] == EATING) //
  {
    pthread_cond_wait(&ready,&m);
    printf("Philosopher %d is eating\n", phil);
    foodTotal--;
    if (foodTotal == 0) {
      printf("DINNER IS OVER!\n");
      pthread_mutex_destroy(&m);
      pthread_cond_destroy(&ready);
      exit(1);
    }
    put_away_forks(phil, left_fork, right_fork);
    sleep(1);
  }
  pthread_mutex_unlock(&m);
}

void tryEat(int phil) { // checking if philosphers on sides are not eating, so a philospher can eat
  if ((statePhilo[(phil + NUM_PHILO - 1) % NUM_PHILO] != EATING) && (statePhilo[phil] == HUNGRY)) {
    statePhilo[phil] = EATING;
    pthread_cond_signal(&ready);
  }
}
void put_away_forks(int phil, int c1, int c2) {
  statePhilo[phil] = THINKING;
  tryEat((phil + NUM_PHILO - 1) % NUM_PHILO);
  tryEat((phil + 1) % NUM_PHILO);
}
