#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <cstdlib>

// add any #defines here

// add global variables here
typedef struct {
	bool loyal;
	bool commander;
} lieutenant_t;

lieutenant_t *g_lieutenantList;
uint8_t g_m = 0; // Number of traitors (or recursions needed)


/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	g_lieutenantList = malloc(nGeneral*sizeof(lieutenant_t));
	
	// Count number of traitors and check n>3*m
	uint8_t m=0;
	for(int i=0; i < nGeneral; i++){
		m+=!loyal[i];
		g_lieutenantList[i].loyal=loyal[i];
	}
	
	// Errors: too many traitors, failed malloc
	if(!c_assert(nGeneral > 3*m) || !c_assert(g_lieutenantList))
		return false;
	
	return true;
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
	free(g_lieutenantList);
	g_lieutenantList = NULL;
}


/** This function performs the initial broadcast to n-1 generals.
  * It should wait for the generals to finish before returning.
  * Note that the general sending the command does not participate
  * in the OM algorithm.
  * command: either 'A' or 'R'
  * sender: general sending the command to other n-1 generals
  */
void broadcast(char command, uint8_t sender) {
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
}
