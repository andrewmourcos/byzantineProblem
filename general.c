#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <cstdlib>

// add any #defines here
#define MAXQUEUELEN 7 // max size of the message queue
#define MAXRECURR 3

// add global variables here

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

lieutenant_t *g_lieutenantList;
uint8_t g_n=0; // total number of generals
uint8_t g_m=0; // number of traitors

uint8_t g_numActiveGenerals = 0;

void printLetter(letter_t letter){
	for(int i=letter.chainIndex-1; i>=0; i--){
		printf("%d:", letter.chain[i]);
	}
	printf("%c\n", letter.decision);
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
	letter.decision = command;
	
	for(int i=0; i<g_n; i++){
		if(i == sender)
			continue;
		
		tmp = osMessageQueuePut(g_lieutenantList[i].messageQueue, &letter, 0,0);
		c_assert(tmp==osOK);
	}
	osDelay(90);
	
	return;
}

bool in_array(uint8_t val, uint8_t arr[], uint8_t len){
	for(int i=0; i<len; i++){
		if (arr[i]==val)
			return true;
	}
	return false;
}

void om_algorithm(uint8_t id, uint8_t m, letter_t letter){
	if(g_lieutenantList[id].reporter){
		
		if(m==0){
			printLetter(letter);
		}
		
		else{
			for(int i=0; i<g_n; i++){
				if(g_lieutenantList[i].commander || i==id || in_array(i, letter.chain, letter.chainIndex))
					continue;
					
				letter.chain[g_m - m + 1] = i;
				letter.chainIndex = g_m - m + 2;
				
				om_algorithm(id, m-1, letter);
			}
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

	// generate next letter and send it
	letter_t received;
	if(!g_lieutenantList[id].commander){
		tmp = osMessageQueueGet(g_lieutenantList[id].messageQueue, &received, NULL, osWaitForever);
		om_algorithm(id, g_m, received);
	}

}
