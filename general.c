#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <cstdlib>

// add any #defines here
#define MAXQUEUELEN 1 // max size of the message queue
#define MAXRECURR 3

// add any types here
typedef struct {
	uint8_t chainIndex; // Last position in chain array
	uint8_t chain[MAXRECURR]; // ex: [0, 3, 4, NaN] == "4:3:0"
	char decision; // 'A' or 'R'
} letter_t;

typedef struct {
	bool loyal;
	bool reporter;
	bool commander;
	osMessageQueueId_t messageQueue;
} lieutenant_t;

// add global variables here
lieutenant_t *g_lieutenantList;
uint8_t g_n=0; // total number of generals
uint8_t g_m=0; // number of traitors

uint8_t g_generalsVisited = 0;
uint8_t g_generalsActive = 0;

osMutexId_t mut_printer;
osMutexId_t mut_counters;

// Helper function to print letters sent b/w generals
void printLetter(letter_t letter){
	printf(" ");
	for(int i=letter.chainIndex-1; i>=0; i--){
		printf("%d:", letter.chain[i]);
	}
	printf("%c", letter.decision);
}

/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	// create semaphores/mutexes
	mut_printer = osMutexNew(NULL);
	mut_counters = osMutexNew(NULL);
	
	// create list of generals + malloc error check
	g_n = nGeneral;
	g_m = 0;
	g_lieutenantList = malloc(g_n*sizeof(lieutenant_t));
	if(!c_assert(g_lieutenantList))
		return false;
	
	// for each general
	for(int i=0; i<nGeneral; i++){
		g_m += !loyal[i]; // count number of traitors
		g_lieutenantList[i].loyal = loyal[i]; // copy loyalty flag
		g_lieutenantList[i].messageQueue = osMessageQueueNew(MAXQUEUELEN, sizeof(letter_t), NULL); // allocate a queue
		g_lieutenantList[i].commander=false;
		g_lieutenantList[i].reporter = false;
		
		// queue error check
		if (!c_assert(g_lieutenantList[i].messageQueue))
			return false;
	}
	g_lieutenantList[reporter].reporter=true;

	// too many traitors check
	if(!c_assert(g_n > 3*g_m))
		return false;
		
	return true;
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
	g_generalsActive = 0;
	g_generalsVisited = 0;
	
	// free message queues
	osStatus_t tmp;
	for(int i=0; i<g_n; i++){
		tmp = osMessageQueueDelete(g_lieutenantList[i].messageQueue);
		c_assert(tmp==osOK);
		g_lieutenantList[i].messageQueue = NULL;
	}
	// free lieutenant list
	free(g_lieutenantList);
	g_lieutenantList = NULL;

	// free semaphores/mutexes/etc
	tmp = osMutexDelete(mut_printer);
	c_assert(tmp==osOK);
	tmp = osMutexDelete(mut_counters);
	c_assert(tmp==osOK);
}


/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t sender) {
	osStatus_t tmp;

	g_lieutenantList[sender].commander=true;
	
	letter_t letter;
	letter.chainIndex = 1;
	letter.chain[0] = sender;
	
	for(int i=0; i<g_n; i++){
		if(i == sender)
			continue;
	
		if( (!g_lieutenantList[sender].loyal) && (i%2==0) )
			letter.decision = 'R';
		else if( (!g_lieutenantList[sender].loyal) && (i%2!=0) )
			letter.decision = 'A';
		else
			letter.decision = command;
		
		tmp = osMessageQueuePut(g_lieutenantList[i].messageQueue, &letter, 0,0);
		c_assert(tmp==osOK);
	}
	
	// keep lending the processor to the general threads until they're all done
	while(!(g_generalsActive==0 && g_generalsVisited==g_n)){
		osDelay(10);
	}
	printf("\n");
	
}

bool in_array(uint8_t val, uint8_t arr[], uint8_t len){
	for(int i=0; i<len; i++){
		if (arr[i]==val)
			return true;
	}
	return false;
}

void om_algorithm(uint8_t id, uint8_t m, letter_t letter){
		
	if(m==0 && g_lieutenantList[id].reporter){
		// Protecting printing rights
		osMutexAcquire(mut_printer, osWaitForever);{
			printLetter(letter);
		} osMutexRelease(mut_printer);
	}
	
	else if(m!=0){
		for(int i=0; i<g_n; i++){
			if(g_lieutenantList[i].commander || i==id || in_array(i, letter.chain, letter.chainIndex))
					continue;
				
				// treacherous lieutenants will say R if *they* are even, A otherwise
				if(!g_lieutenantList[id].loyal && id%2==0)
					letter.decision = 'R';
				else if(!g_lieutenantList[id].loyal && id%2!=0)
					letter.decision = 'A';
			
				letter.chain[g_m - m + 1] = id;
				letter.chainIndex = g_m - m + 2;
				om_algorithm(i, m-1, letter);
		}
	}	
}

/** Generals are created before each test and deleted after each
  * test.  The function should wait for a value from broadcast()
  * and then use the OM algorithm to solve the Byzantine General's
  * Problem.  The general designated as reporter in setup()
  * should output the messages received in OM(0).
  * idPtr: pointer to general's id number which is in [0,n-1]
  */
void general(void *idPtr) {
	uint8_t id = *(uint8_t *)idPtr;
	osStatus_t tmp;

	osMutexAcquire(mut_counters, osWaitForever);{
		g_generalsActive ++;
		g_generalsVisited ++;
	}osMutexRelease(mut_counters);
		
		// generate next letter and send it
	letter_t received;
	if(!g_lieutenantList[id].commander){
		tmp = osMessageQueueGet(g_lieutenantList[id].messageQueue, &received, NULL, osWaitForever);
		om_algorithm(id, g_m, received);
	}
	
	osMutexAcquire(mut_counters, osWaitForever);{
		g_generalsActive--;
	}osMutexRelease(mut_counters);
}
