#include <cmsis_os2.h>
#include "general.h"

// add any #includes here
#include <cstdlib>

// add any #defines here

// add global variables here
char *g_reporterMat;
// Note to self:
// 	arr[i*M+j] == arr[i][j]

void printMat(uint8_t nGeneral){
	for(int i=0; i < nGeneral; i++){
		for(int j=0; j < nGeneral; j++){
			g_reporterMat[i*(nGeneral-1)+j]='R';
			printf("%c\t", g_reporterMat[i*(nGeneral-1)+j]);
		}
		printf("\n");
	}
}


/** Record parameters and set up any OS and other resources
  * needed by your general() and broadcast() functions.
  * nGeneral: number of generals
  * loyal: array representing loyalty of corresponding generals
  * reporter: general that will generate output
  * return true if setup successful and n > 3*m, false otherwise
  */
bool setup(uint8_t nGeneral, bool loyal[], uint8_t reporter) {
	// Create report matrix
	g_reporterMat = malloc((nGeneral-1)*(nGeneral-1)*sizeof(char));
	printMat(nGeneral);
	// Count number of traitors and check n>3*m
	uint8_t m=0;
	for(int i=0; i < nGeneral; i++){
		if(!loyal[i])
			m++;
	}
	
	// Errors: too many traitors, failed malloc (NULL)
	if(!c_assert(nGeneral > 3*m) || !c_assert(g_reporterMat))
		return false;
	
	return true;
}


/** Delete any OS resources created by setup() and free any memory
  * dynamically allocated by setup().
  */
void cleanup(void) {
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
