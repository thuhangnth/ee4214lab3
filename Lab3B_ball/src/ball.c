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
 -- File           : BALL.c
 -----------------------------------------------------------------------------
 -- Description    : C code
 -- --------------------------------------------------------------------------
 -- Author         : Manmohan/Shakith

 -----------------------------------------------------------------------------
 */
#include "xmk.h"
#include <xparameters.h>
#include <pthread.h>
#include <errno.h>
#include "xmutex.h"

// declare the Hardware Mutex
#define MUTEX_DEVICE_ID XPAR_MUTEX_0_IF_1_DEVICE_ID
#define MUTEX_NUM 0
XMutex Mutex;

struct sched_param sched_par;
pthread_attr_t attr;
pthread_t tid1;
static int MAX_ITERATION=10;
volatile int taskrunning;

void ball()
{
	// --add your lock and unlock code in this function
	XMutex_Lock(&Mutex,MUTEX_NUM);
	int i=0;
	for (i=0; i<MAX_ITERATION;i++){

		xil_printf(" Microblaze 1 : thread to control ball !! \r\n");
	}
	XMutex_Unlock(&Mutex,MUTEX_NUM);
}


void* thread_func_1 () {
	while(1) {
		ball();
		sleep(200);
	}
}

void main_prog(void);
int main (void) {
	print("-- Entering main() uB1 --\r\n");
	xilkernel_init();
	xmk_add_static_thread(main_prog,0);
	xilkernel_start();
	//Start Xilkernel
	xilkernel_main ();

	//Control does not reach here

}

void main_prog(void) {   // This thread is statically created (as configured in the kernel configuration) and has priority 0 (This is the highest possible)


	print("-- Entering main() uB1 --\r\n");
	XStatus Status;
	int ret;
	// configure the HW Mutex
	XMutex_Config*ConfigPtr;
	ConfigPtr = XMutex_LookupConfig(MUTEX_DEVICE_ID);
	if (ConfigPtr == (XMutex_Config *)NULL){
		xil_printf("B1-- ERROR  init HW mutex...\r\n");
	}

	// initialize the HW Mutex	
	Status = XMutex_CfgInitialize(&Mutex, ConfigPtr, ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS){

		xil_printf ("B1-- ERROR  init HW mutex...\r\n");
	}


	print("--Initialized -- uB1 \r\n");
	ret = pthread_create (&tid1, NULL, (void*)thread_func_1, NULL);


	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_1...\r\n", ret);
	}
	else {
		xil_printf ("Thread 1 launched with ID %d \r\n",tid1);
	}

}

