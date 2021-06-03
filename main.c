#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

void *commentator(void *);
void *moderator(void *);

/**
 * pthread_sleep takes an integer number of seconds to pause the current thread
 * original by Yingwu Zhu
 * updated by Muhammed Nufail Farooqi
 * updated by Fahrican Kosar
 */
int pthread_sleep(double seconds){
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL)){
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

#define MAX_COMMENTATOR_NUM 256

int n; //commentator threads number
int q; //question number
double p; //answer probability
double t;
double b;

//SEMAPHORES
sem_t commentator_sem;

//MUTEXES
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mainMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[MAX_COMMENTATOR_NUM];
pthread_cond_t question_available = PTHREAD_COND_INITIALIZER; //Condition variable that is going to check if question is asked and commentators can answer
pthread_cond_t all_comentators_decided = PTHREAD_COND_INITIALIZER; //Condition to check if moderator can go on after commentators decide to answer
pthread_cond_t answerQueueConditions[MAX_COMMENTATOR_NUM];
pthread_cond_t wait_for_commentator_threds = PTHREAD_COND_INITIALIZER;

int answerQueue[MAX_COMMENTATOR_NUM];

int main(int argc, char *argv[]) {

    for(int i = 1; i < argc; i+=2){ // start from first arg go setting by setting
        if (strcmp(argv[i], "-n") == 0) {
            n = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-q") == 0) {
            q = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-p") == 0) {
            p = atof(argv[i + 1]);
        }
        if (strcmp(argv[i], "-t") == 0) {
            t = atof(argv[i + 1]);
        }
        if (strcmp(argv[i], "-b") == 0) {
            b = atof(argv[i + 1]);
        }
    }
    printf("n: %d, q: %d, p: %f, t: %f, b: %f\n", n, q, p, t, b);

    //initialize semaphores
    sem_init(&commentator_sem, 0, 1);
    pthread_create(&threads[0], NULL, moderator, NULL); //Create 1 moderator thread
    printf("Moderator thread created.\n");
    for(int i = 1; i<(n + 1); i++){
        pthread_create(&threads[i], NULL, commentator, (void *)(long)i); //Create n commentator thread
    }
    printf("Commentator threads created.\n");
}

int currComentator = 0;
int answerQueueCount = 0;
int createdCommentatorCount = 0;
void *commentator(void *args){
    int id = (long) args;
    sem_wait(&commentator_sem);
    createdCommentatorCount++;
    if(createdCommentatorCount == n){
        pthread_cond_signal(&wait_for_commentator_threds);
    }
    sem_post(&commentator_sem);
    
    for(int i = 0; i < q; i++){
        //all are going to wait for a question to be asked
        pthread_mutex_lock(&mutex);
        printf("before wait question ava\n");
        pthread_cond_wait(&question_available, &mutex);
        printf("In commentator loop question available\n");
        //---- decide to answer
        int answer = 0;
        double rand = (double) random() / RAND_MAX;

        if(rand < p){ // this commentator answers
            answer = 1;
        }else{ //dont answer
            answer = 0;
        }
        //----

        sem_wait(&commentator_sem); //wait for other commentator
        if(answer == 1){
            answerQueueCount++;
            printf("commentator #%d generates answer, position in queue: %d\n", id, answerQueueCount);
            answerQueue[currComentator] = id;
        }else{
            answerQueue[currComentator] = 0;
        }
        currComentator++;
        if(currComentator == n){
            pthread_cond_signal(&all_comentators_decided); //moderator was waiting
        }
        sem_post(&commentator_sem);

        if(answer == 1){ //answer when its this comentator turns

        }

    }
    pthread_mutex_unlock(&mutex);
}

void *moderator(void *args){
    
    for(int i = 0; i<q; i++){ //Moderator is going to ask q questions 1 by 1
        pthread_mutex_lock(&mutex);

        currComentator = 0;
        answerQueueCount = 0;

        printf("Moderator asks question %d\n", (i+1));
        pthread_cond_wait(&wait_for_commentator_threds, &mutex);
        printf("before broad\n");
        pthread_cond_broadcast(&question_available); //all commentators is going to decide to answer now.
        printf("after broad\n");
        pthread_cond_wait(&all_comentators_decided, &mutex); //commentators decided to answer now we can let them answer 1 by 1
        printf("after wait moderator\n");

    }
    pthread_mutex_unlock(&mutex);
}

