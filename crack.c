//Jake Bernstein
//9/27/18
//Lab 3 - Crack - a program for finding the password associated with a targe DES hash
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <crypt.h>
#include <string.h>

/* 4 threads: thread id 0-3
thread 0: 'a' + 0 -> add 4 = 'e' + 0: a, e, i, j, k, l, m, q, u, y
thread 1: 'a' + 1 = 'b' -> add 4 = 'f' + 0: 
thread 2: 'a' + 2 = 'c', 'g', 'k', 'o', 's', 'w' > 'z'

psswd[0] ->a _ _ _
psswd[1] ->b _ _ _
           _ _ _ _
           _ _ _ _

char psswd[numThreads][keysize + 1]
psswd[0] points to the first byte in the first item in the array, can set that as a certain letter to change when the others all reach 'z' without a product

CANNOT BE GLOBAL, MUST BE IN MAIN, AS LONG AS VARIABLES ARE DEFINED
Could also create an excessively large array which would be globally declared, just not very space efficient
 */

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //Set up the mutex
char psswd[100][8];

struct cryptStruct{
  int keyLength;
  char *hash;
  char *salt;
  int threadCount;
  int threadNum;
};

void *decrypt( void* args ){

  //A struct pointer
  struct cryptStruct* ptr = (struct cryptStruct*)args;

  //Initializing values
  int keyLength = ptr->keyLength;
  char *hash = ptr->hash;
  char *salt = ptr->salt;
  int threadCount = ptr->threadCount;
  int threadNum = ptr->threadNum;

  //Variables for iterating through passwords
  int work = 0;
  char guess[keyLength + 1];
  char noResult[keyLength + 1];

  //printf("%d\n", threadNum);

  strncpy(guess, psswd[threadNum], keyLength);
 
  //Populate the guess string - guess is already of length [keyLength]
  //Now we just need to fill those open spaces with 'a'
  int i = 1;
  for(i = 1; i < keyLength; i++) {
    guess[i] = 'a';
    noResult[i] = 'z';
  }
  noResult[0] = 'z';

  //Put the results of Crypt_r in one variable to save time
  char *crypt_r_results;

  //Not sure why this works, but the program breaks without it
  struct crypt_data thing;
  thing.initialized = 0;

  printf("Running...\n");

  //printf("%s\n", guess); //debugging
  //printf("%s\n", noResult); //debugging
  
  while(work < keyLength){
    work = 0;

    //pthread_mutex_lock(&mutex); //Lock mutex to prevent threads from interfering
                                //with each other

    //Here's the important bit: This cycles through all the possibilities
    //To find a password match
    while(work < keyLength){
      if(guess[work] >= 'a' && guess[work] < 'z'){
	
	guess[work]++;
	break;
	
      }
      
      if(guess[work] == 'z'){
	
	guess[work] = 'a';
	work++;
	
      }
      
    }

    //This code copies the result to a variable set up for the purpose
    //It also adds the null value to the end of that variable
    //strncpy(guess, start, keyLength);
    guess[keyLength] = 0;
    crypt_r_results = crypt_r(guess, salt, &thing);

    //printf("%s\n", guess);

    //This should only activate if no match is discovered
    if(strcmp(guess, noResult) == 0){
      printf("Something went wrong. No valid match discovered.\n");
      exit(0);
    }

    //pthread_mutex_unlock(&mutex); //Need to unlock the mutex after the operation

    //printf("Results: %s, %s\n", guess, crypt_r_results);

    //Compare the guess with the original hash input - if it's a match, it will
    //Return 0
    if(strcmp(crypt_r_results, hash) == 0){

	  printf("Positive match: %s\n", guess);
	  exit(0);
    }
  }
}

int main( int argc, char *argv[] ){

  //Error checking: make sure the user sent the proper number of arguments
  if( argc != 4 ){
    printf("Usage: ./crack <threads> <keysize> <target>\n" );
    return -1;
  }

  //Assigning arguments to variables
  int numThreads = atoi(argv[1]);
  int keysize = atoi(argv[2]);
  char *target = argv[3];

  pthread_t threads[numThreads]; //Building a pthread array
  
  //Error checking: Make sure the keysize is not too long
  if( keysize > 8 ){
    printf("Keysize too large!\n");
    return -1;
  }

  char salt[2];

  //Setting up the decryption salt and adding the null terminator
  salt[0] = target[0];
  salt[1] = target[1];
  salt[2] = 0;

  struct cryptStruct cryptValues;
  cryptValues.keyLength = keysize;
  cryptValues.hash = target;
  cryptValues.salt = salt;
  cryptValues.threadCount = numThreads;

  printf("Initializing %d threads\n", numThreads);
  
  //Create the threads using the decrypt function - work is done here
  char start = 'a';
  int i = 0;
  for (i = 0; i < 27; i++){
    psswd[i][0] = start;
    //printf("%s\n", psswd[i]);
    start++;
  }
  
  for (i = 0; i < numThreads; i++){

    //Need to create arithmetic that will allow each thread to start with a different letter
    //in the first byte of each entry in psswd

    cryptValues.threadNum = i;
    
    pthread_create(&threads[i], NULL, decrypt, &cryptValues);
  }

  for (i = 0; i < numThreads; i++){
    pthread_join(threads[i], NULL);
  }

  return 0;
}
