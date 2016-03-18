/*
 -----------------------------------------------------------------------------
 -- Copyright (C) 2005 IMEC                                                  -
 --                                                                          -
 -- Redistribution and use in source and binary forms, with or without       -
 -- modification, are permitted provided that the following conditions       -
 -- are met:                                                                 -
 --                                                                          -
 -- 1. Redistributions of source code must retain the above copyright        -
 --    notice, this list of conditions and the following disclaimer.         -
 --                                                                          -
 -- 2. Redistributions in binary form must reproduce the above               -
 --    copyright notice, this list of conditions and the following           -
 --    disclaimer in the documentation and/or other materials provided       -
 --    with the distribution.                                                -
 --                                                                          -
 -- 3. Neither the name of the author nor the names of contributors          -
 --    may be used to endorse or promote products derived from this          -
 --    software without specific prior written permission.                   -
 --                                                                          -
 -- THIS CODE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''           -
 -- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED        -
 -- TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A          -
 -- PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR       -
 -- CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,             -
 -- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         -
 -- LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF         -
 -- USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND      -
 -- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,       -
 -- OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT       -
 -- OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF       -
 -- SUCH DAMAGE.                                                             -
 --                                                                          -
 -----------------------------------------------------------------------------
 -----------------------------------------------------------------------------
 -- File           : threads_PRIO.c
 -----------------------------------------------------------------------------
 -- Description    : C code
 -- --------------------------------------------------------------------------
 -- Author         : Kristof Loots
 -- Date           : 14/09/2006
 -- Version        : V1.0
 -- Change history :
 -----------------------------------------------------------------------------
 */
#include "xmk.h"
#include <xparameters.h>
#include <pthread.h>
#include <errno.h>
#include "xmutex.h"



// MUTEX ID PARAMETER for HW Mutex
// --add your code here
#define MUTEX_DEVICE_ID XPAR_MUTEX_0_IF_1_DEVICE_ID
#define MUTEX_NUM 0
XMutex Mutex;

struct sched_param sched_par;
pthread_attr_t attr;
pthread_t tid1, tid2, tid3, tid4;

// ---------------------------------------

volatile int taskrunning;

void do_something(int max, int ID) {
	// --add your lock and unlock code in this function
	// int max : Number of outer loops.
	// int ID  : The number of the thread. This is used for debugging.
	//
	int i,j;

	for(i=0; i< max; i++) {
		XMutex_Lock(&Mutex,MUTEX_NUM);
		if (taskrunning != ID) {
			taskrunning = ID;
			xil_printf ("\r\n Column changed to : %d\r\n", ID);

		}
		xil_printf ("Microblaze 0: Column %d, i %d\r\n", ID, i);
		XMutex_Unlock(&Mutex,MUTEX_NUM);
		for(j=0; j< 0xffff; j++) {
			;
		}

	}

}


void* thread_func_1 () {
	while(1) {
		do_something(0x0004, 1);
		sleep(200);
	}
}


void* thread_func_2 () {
	while(1) {
		do_something(0x000f, 2);
		sleep(100);
	}
}


void* thread_func_3 () {
	while(1) {
		do_something(0x000f, 3);
		sleep(10);
	}
}



void main_prog(void);

int main (void) {


	print("-- Entering main() uB0--\r\n");
	xilkernel_init();
	xmk_add_static_thread(main_prog,0);
	xilkernel_start();
	//Start Xilkernel
	xilkernel_main ();

	//Control does not reach here

}

void main_prog(void) {   // This thread is statically created (as configured in the kernel configuration) and has priority 0 (This is the highest possible)

	int ret;

	print("-- Entering main_prog() uB0--\r\n");

	pthread_attr_init (&attr);

	XStatus Status;

	// configure the HW Mutex
	// --add your code here

	// initialize the HW Mutex
	// --add your code here

	XMutex_Config*ConfigPtr;
	ConfigPtr = XMutex_LookupConfig(MUTEX_DEVICE_ID);
	if (ConfigPtr == (XMutex_Config *)NULL){

		("B1-- ERROR  init HW mutex...\r\n");
	}

	Status = XMutex_CfgInitialize(&Mutex, ConfigPtr, ConfigPtr->BaseAddress);


	print("--Initialized -- uB0 \r\n");
	// Priority 1 for thread 1
	sched_par.sched_priority = 1;
	pthread_attr_setschedparam(&attr,&sched_par);
	print("--Priority Set uB0 --\r\n");
	//start thread 1
	ret = pthread_create (&tid1, &attr, (void*)thread_func_1, NULL);
	print("--returned --\r\n");
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_1...\r\n", ret);
	}
	else {
		xil_printf ("Thread 1 launched with ID %d \r\n",tid1);
	}

	// Priority 2 for thread 2
	sched_par.sched_priority = 2;
	pthread_attr_setschedparam(&attr,&sched_par);

	//start thread 2
	ret = pthread_create (&tid2, &attr, (void*)thread_func_2, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_2...\r\n", ret);
	}
	else {
		xil_printf ("Thread 2 launched with ID %d \r\n",tid2);
	}

	// Priority 3 for thread 3
	sched_par.sched_priority = 3;
	pthread_attr_setschedparam(&attr,&sched_par);

	//start thread 3
	ret = pthread_create (&tid3, &attr, (void*)thread_func_3, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_3...\r\n", ret);
	}
	else {
		xil_printf ("Thread 3 launched with ID %d \r\n",tid3);
	}

}

