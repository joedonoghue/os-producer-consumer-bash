#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>


using namespace std;

#define SEM_PRODUCER_FNAME "/myproducer"
#define SEM_CONSUMER_FNAME "/myconsumer"

int item_cnt;
int buf_len;
int prod_cnt;
int fprod_cnt;
int cons_cnt;
bool debug = true;

sem_t *sem_prod;
sem_t *sem_cons;


vector<pthread_t> prod_threads;
vector<pthread_t> cons_threads;
vector<int> numbers;

bool isPrime(int num) {
  if(num <= 1) return false;
    int sq = sqrt(num);
    for(int i = 2; i <= sq; ++i) {
        if(num % i == 0) {
            return false;
        }
    }
    return true;
}

void printNumbers() {
  printf("[ ");
  for(int i: numbers) {
    printf("%d ", i);
  }
  printf("] ");
}

void *producer(void *id) {
  int randint;
  int tid = (*((int *)id)) + 1;
  for(int i = 0; i < item_cnt; i++) {
    while(!isPrime(randint = rand()));
    sem_wait(sem_cons);
    numbers.push_back(randint);
    printf("(PRODUCER %d writes %d/10 %d): (%ld): ", tid, i+1, randint, numbers.size());
    printNumbers();
    if(numbers.size() >= buf_len) {
      printf(" *BUFFER NOW FULL* ");
    }
    printf("\n");
    sem_post(sem_prod);
  }
  return NULL;
}

void createProducers() {
  for(int i = 0; i < prod_cnt; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, producer, &i);
    prod_threads.push_back(thread_id);
  }
}



void joinProducers() {
  for(int i = 0; i < prod_threads.size(); i++) {
    pthread_join(prod_threads.at(i), NULL);
    cout << "Joining producer " << i << endl;
  }
}



void *consumer(void *id) {
  int tid = (*((int *)id))+1;
  while(true) {
    sem_wait(sem_prod);
    if(!numbers.empty()) {
      int num = numbers.back();
      numbers.pop_back();
      printf("(CONSUMER %d reads %d): (%ld): ", tid, num, numbers.size());
      printNumbers();
      if(!isPrime(num)) {
        cout << " *NOT PRIME* ";
      }
      if(numbers.empty()) {
        cout << " *BUFFER NOW EMPTY* ";
      }
      cout << endl;
    }

    sem_post(sem_cons);
  }
  return NULL;
}

void createConsumers() {
  for(int i = 0; i < cons_cnt; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, consumer, &i);
    cons_threads.push_back(thread_id);
  }
}


void cancelConsumers() {
  for(int i = 0; i < cons_threads.size(); i++) {
    pthread_cancel(cons_threads.at(i));
    cout << "Canceling consumer " << i << endl;
  }
}



int getCmdOption(char ** begin, char ** end, const std::string & option)
{
  char ** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    try {
      int ret = stoi(*itr);
      return ret;
    }
    catch (const std::exception&) {
      cout << "Error with option -" << option << endl;

      return -1;
    }
  }
  cout << "Missing -" << option << endl;
  return -1;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}

int main(int argc, char * argv[])
{
  item_cnt = getCmdOption(argv, argv + argc, "-n");
  buf_len = getCmdOption(argv, argv + argc, "-l");
  prod_cnt = getCmdOption(argv, argv + argc, "-p");
  fprod_cnt = getCmdOption(argv, argv + argc, "-f");
  cons_cnt = getCmdOption(argv, argv + argc, "-c");

  if(debug) {
    printf("item_cnt: %d\n", item_cnt);
    printf("buf_len: %d\n", buf_len);
    printf("prof_cnt: %d\n", prod_cnt);
    printf("fprof_cnt: %d\n", fprod_cnt);
    printf("cons_cnt: %d\n", cons_cnt);
  }

  sem_unlink(SEM_PRODUCER_FNAME);
  sem_unlink(SEM_CONSUMER_FNAME);
  sem_prod = sem_open(SEM_PRODUCER_FNAME, O_CREAT, 0660, 0);
  sem_cons = sem_open(SEM_CONSUMER_FNAME, O_CREAT, 0660, 1);

  if (sem_prod == SEM_FAILED) {
    perror("sem_open/producer");
    exit(EXIT_FAILURE);
  }

  if (sem_cons == SEM_FAILED) {
    perror("sem_open/consumer");
    exit(EXIT_FAILURE);
  }

  createProducers();
  createConsumers();
  joinProducers();
  cancelConsumers();

  return 0;
}