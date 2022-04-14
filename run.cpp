/* 
  Joe Donoghue
  Aparajita Biswas

  Producer/Consumer Problem with Faulty Threads
  COSC 40203 - Operating Systems
*/


/* INCLUDES */
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
#include <chrono>

/* NAMESPACES */
using namespace std::chrono;
using namespace std;

/* DEFINITIONS */
#define SEM_EMPTY_FNAME "/myempty"
#define SEM_FULL_FNAME "/myfull"
#define SEM_MUTEX_FNAME "/mymutex"

/* GLOBALS */
// cmd arguments
int item_cnt;
int buf_len;
int prod_cnt;
int fprod_cnt;
int cons_cnt;
bool debug;

// stats
int full_buf_cnt = 0;
int empty_buf_cnt = 0;
int non_prime_cnt = 0;
int item_cons_cnt = 0;

// semaphores
sem_t *sem_empty;
sem_t *sem_full;
sem_t *sem_mutex;

// vectors
vector<int> numbers;
vector<string> consumed;
vector<pthread_t> prod_threads;
vector<pthread_t> cons_threads;
vector<pthread_t> faulty_prod_threads;


/* FUNCTIONS */

// return true if prime
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

// print numbers vector helper
void printNumbers() {
  printf("[ ");
  for(int i: numbers) {
    printf("%d ", i);
  }
  printf("] ");
}

// producer algorithm
void *producer(void *id) {
  int randint;
  int tid = (*((int *)id)) + 1; // get thread id
  for(int i = 0; i < item_cnt; i++) { // loop to item count
    while(!isPrime(randint = (rand() % 999999 + 1))); // get a prime
    sem_wait(sem_empty); // wait (empty)
    sem_wait(sem_mutex); // wait (mutex)
    numbers.push_back(randint);
    printf("(PRODUCER %d writes %d/%d %d): (%ld): ", tid, i+1, item_cnt, randint, numbers.size());
    printNumbers();
    if(numbers.size() >= buf_len) {
      printf(" *BUFFER NOW FULL* ");
      full_buf_cnt++;
    }
    printf("\n");
    sem_post(sem_mutex); // signal (mutex)
    sem_post(sem_full); // signal (full)
  }
  return NULL;
}

// create producer threads
void createProducers() {
  for(int i = 0; i < prod_cnt; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, producer, &i);
    prod_threads.push_back(thread_id);
  }
}

// join producer threads
void joinProducers() {
  for(int i = 0; i < prod_threads.size(); i++) {
    pthread_join(prod_threads.at(i), NULL);
    cout << "Joining producer " << i << endl;
  }
}

// faulty producer algorithm
void *faultyProducer(void *id) {
  int randint;
  int tid = (*((int *)id)) + 1; // get thread id
  for(int i = 0; i < item_cnt; i++) {
    while(isPrime(randint = (rand() % 999999 + 1))); // get non-prime
    sem_wait(sem_empty); // wait (empty)
    sem_wait(sem_mutex); // wait (mutex)
    numbers.push_back(randint);
    printf("(PR*D*C*R %d writes %d/%d %d): (%ld): ", tid, i+1, item_cnt, randint, numbers.size());
    printNumbers();
    if(numbers.size() >= buf_len) {
      printf(" *BUFFER NOW FULL* ");
      full_buf_cnt++;
    }
    printf("\n");
    sem_post(sem_mutex); // signal (mutex)
    sem_post(sem_full); // signal (full)
  }
  return NULL;
}

// create faulty consumers
void createFaultyProducers() {
  for(int i = 0; i < fprod_cnt; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, faultyProducer, &i);
    prod_threads.push_back(thread_id);
  }
}


// join faulty consumers
void joinFaultyProducers() {
  for(int i = 0; i < faulty_prod_threads.size(); i++) {
    pthread_join(faulty_prod_threads.at(i), NULL);
    cout << "Joining Faulty Producer " << i << endl;
  }
}


// consumer thread algorithm
void *consumer(void *id) {
  int tid = (*((int *)id))+1; // thread id
  while(true) {
    sem_wait(sem_full); // wait (full)
    sem_wait(sem_mutex); // wait (mutex)
    int num = numbers.back();
    numbers.pop_back();
    sem_post(sem_mutex); // signal (post)
    sem_post(sem_empty); // signal (empty)
    item_cons_cnt++;
    consumed.push_back("Thread " + std::to_string(tid) + ": " + std::to_string(num));
    printf("(CONSUMER %d reads %d): (%ld): ", tid, num, numbers.size());
    printNumbers();
    if(!isPrime(num)) {
      cout << " *NOT PRIME* ";
      non_prime_cnt++;
    }
    if(numbers.empty()) {
      cout << " *BUFFER NOW EMPTY* ";
      empty_buf_cnt++;
    }
    cout << endl;

  }
  return NULL;
}

// create consumers
void createConsumers() {
  for(int i = 0; i < cons_cnt; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, consumer, &i);
    cons_threads.push_back(thread_id);
  }
}

// cancel consumers (cancel because they are infinite loops)
void cancelConsumers() {
  for(int i = 0; i < cons_threads.size(); i++) {
    pthread_cancel(cons_threads.at(i));
    cout << "Canceling consumer " << i << endl;
  }
}

// print stats
void printStats() {
  printf("PRODUCER / CONSUMER SIMULATION COMPLETE\n");
  printf("=======================================\n");
  printf("Number of Items Per Producer Thread: %d\n", item_cnt);
  printf("Size of Buffer: %d\n", buf_len);
  printf("Number of Producer Threads: %d\n", prod_cnt);
  printf("Number of Faulty Producer Threads: %d\n", prod_cnt);
  printf("Number of Consumer Threads: %d\n", cons_cnt);
  printf("\n");
  printf("Number of Times Buffer Became Full %d\n", full_buf_cnt);
  printf("Number of Times Buffer Became Empty %d\n", empty_buf_cnt);
  printf("\n");
  printf("Number of Non-prime Detected %d\n", non_prime_cnt);
  printf("Total Number of Items Consumed: %d\n", item_cons_cnt);
  for(string thread : consumed) {
    printf("  %s\n", thread.c_str());
  }
  printf("\n");
}



// get command line option
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

      return -2;
    }
  }
  cout << "Missing -" << option << endl;
  return -1;
}

// check if command line option exists
bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}


// main function
int main(int argc, char * argv[])
{
  // get cmd options
  item_cnt = getCmdOption(argv, argv + argc, "-n");
  buf_len = getCmdOption(argv, argv + argc, "-l");
  prod_cnt = getCmdOption(argv, argv + argc, "-p");
  fprod_cnt = getCmdOption(argv, argv + argc, "-f");
  cons_cnt = getCmdOption(argv, argv + argc, "-c");
  debug = cmdOptionExists(argv, argv + argc, "-d");

  // handle semaphores
  sem_unlink(SEM_EMPTY_FNAME);
  sem_unlink(SEM_FULL_FNAME);
  sem_unlink(SEM_MUTEX_FNAME);

  sem_empty = sem_open(SEM_EMPTY_FNAME, O_CREAT, 0660, buf_len);
  sem_full = sem_open(SEM_FULL_FNAME, O_CREAT, 0660, 0);
  sem_mutex = sem_open(SEM_MUTEX_FNAME, O_CREAT, 0660, 1);

  if (sem_empty == SEM_FAILED) {
    perror("sem_open/empty");
    exit(EXIT_FAILURE);
  }

  if (sem_full == SEM_FAILED) {
    perror("sem_open/full");
    exit(EXIT_FAILURE);
  }

  if (sem_mutex == SEM_FAILED) {
    perror("sem_open/mutex");
    exit(EXIT_FAILURE);
  }

  // start producer consumer algorithms
  auto start = high_resolution_clock::now(); // start timer
  createProducers();
  createFaultyProducers();
  createConsumers();
  joinProducers();
  joinFaultyProducers();
  cancelConsumers();
  auto stop = high_resolution_clock::now(); // end timer
  auto duration = duration_cast<microseconds>(stop - start); // get duration

  // print stats if debug
  if(debug) {
    printStats();
    cout << "Total Simulation Time: " << duration.count() << " microseconds" <<  endl;
  }
  return 0;
}