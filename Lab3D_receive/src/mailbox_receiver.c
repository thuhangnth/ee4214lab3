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
 -- File           : MAILBOX_PLAYERS.c
 -----------------------------------------------------------------------------
 -- Description    : C code
 -- --------------------------------------------------------------------------
 -- Author         :Shakith/Manmohan
 -- Date           : 
 -- Change history : 
 -----------------------------------------------------------------------------
 */
#include "xmk.h"
#include <xparameters.h>
#include "xmbox.h"
#include "xstatus.h"
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>
#include "xmutex.h"


// mailbox declaration
#define MY_CPU_ID XPAR_CPU_ID
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
static XMbox Mbox;	/* Instance of the Mailbox driver */

#define mutex_DEVICE_ID XPAR_mutex_0_IF_1_DEVICE_ID
#define mutex_NUM 0

//Xmutex mutex;
pthread_mutex_t mutex;
pthread_mutex_t m_attr;

pthread_t tid1,tid2,tid3,tid4,disp;
const int COLUMN_WIDTH = 20;
const int COLUMN_LENGTH = 50;
const int COLOUR = 0x00ffff00;

typedef struct {

	int dir,colour,x,y;
} ball_msg;

typedef struct {
	int x,y,w,l,c;
}msg;

static void MailboxExample_Receive(XMbox *MboxInstancePtr, ball_msg *ball_pointer);

void send (int id, int x, int y, int w, int l, int c) {

	msg send_msg;
	int msgid;
	send_msg.x = x;
	send_msg.y = y;
	send_msg.w = w;
	send_msg.l = l;
	send_msg.c = c;

	//ENTER YOUR CODE HERE TO SEND DATA TO THREAD DISPLAY
	pthread_mutex_lock(&mutex);

	xil_printf("PRODCON: Producer -- Start !\r\n");

	msgid = msgget (id, IPC_CREAT ) ;
	if( msgid == -1 ) {
		xil_printf ("PRODCON: Producer -- ERROR while opening Message Queue. Errno: %d\r\n", errno) ;
		pthread_exit (&errno);
	}

	if( msgsnd (msgid, &send_msg, 20, 0) < 0 ) { // blocking send
		xil_printf ("PRODCON: Producer -- msgsnd of message(%d) ran into ERROR. Errno: %d. Halting..\r\n", id, errno);
		pthread_exit(&errno);
	}

	xil_printf("PRODCON: Producer done !\r\n");
	pthread_mutex_unlock(&mutex);

}

void* thread_func_1 () {
	while(1) {
		send(1,10,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR);
		sleep(100);
	}
}


void* thread_func_2 () {
	while(1) {
		send(2,50,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR);
		sleep(100);
	}
}



void* thread_func_3 () {
	while(1) {
		send(3,90,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR);
		sleep(100);
	}
}

void* thread_func_4 ()
{
	while(1)
	{
		send(4,130,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR);
		sleep(100);
	}
}

void* display () {
	msg disp_msg;
	int i,msgid;
	ball_msg ball;
	while (1) {
		// wait for message from send
		//ENTER YOUR CODE HERE TO RECEIVE THE DATA FROM THREADS
		//pthread_mutex_lock(&mutex);

		for (i=1; i<5; i++) {
			msgid = msgget (i, IPC_CREAT);
			if( msgid == -1 ) {
				xil_printf ("PRODCON: Consumer -- ERROR while opening Message Queue. Errno: %d \r\n", errno);
				pthread_exit(&errno) ;
			}
			if( msgrcv( msgid, &disp_msg, 20, 0,0 ) != 20 ) { // blocking recv
				xil_printf ("PRODCON: Consumer -- msgrcv of message(%d) ran into ERROR. Errno: %d. Halting...\r\n", i, errno);
				//       consret = errno;
				pthread_exit(&errno);
			}
			// print message


			xil_printf ("\rDisplay received : Column (x,y) = (%d,%d), w = %d  l= %d, c=%d  \r\n", disp_msg.x, disp_msg.y,disp_msg.w,disp_msg.l,disp_msg.c);


		}
		//pthread_mutex_unlock(&mutex);
		MailboxExample_Receive(&Mbox, &ball);
		pthread_mutex_lock(&mutex);
		print("-- Sucessfully receive IN RECEIVER --\r\n");
		pthread_mutex_unlock(&mutex);
	}
	sleep(100);
}

int main_prog(void);
int main (void) {
	print("-- Entering main() uB0 RECEIVER--\r\n");
	xilkernel_init();
	xmk_add_static_thread(main_prog,0);
	xilkernel_start();
	//Start Xilkernel
	xilkernel_main ();

	//Control does not reach here

}


int main_prog(void) {   // This thread is statically created (as configured in the kernel configuration) and has priority 0 (This is the highest possible)

	int ret;
	XStatus mutexStatus;

	print("-- Entering main_prog() uB0 RECEIVER--\r\n");

	//Xmutex_Config*mutex_ConfigPtr;
	XMbox_Config *ConfigPtr;
	int Status;

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * component.
	 */
	//mutex_ConfigPtr = Xmutex_LookupConfig(mutex_DEVICE_ID);
	//if (mutex_ConfigPtr == (Xmutex_Config *)NULL){

//		print("B1-- ERROR  init HW mutex...\r\n");
	//}

	// initialize the HW mutex
	//mutexStatus = Xmutex_CfgInitialize(&mutex, mutex_ConfigPtr, mutex_ConfigPtr->BaseAddress);

	ConfigPtr = XMbox_LookupConfig(MBOX_DEVICE_ID );

	if (ConfigPtr == (XMbox_Config *)NULL) {
		print("-- Error configuring Mbox uB0 receiver--\r\n");
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization.
	 */
	Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		print("-- Error initializing Mbox uB0 receiver--\r\n");
		return XST_FAILURE;
	}
	pthread_mutexattr_init(&m_attr);
	pthread_mutex_init(&mutex, &m_attr);

	ret = pthread_create (&disp, NULL, (void*)display, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching display...\r\n", ret);
	}
	else {
		xil_printf ("Display launched with ID %d \r\n",disp);
	}


	//start thread 1
	ret = pthread_create (&tid1, NULL, (void*)thread_func_1, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_1...\r\n", ret);
	}
	else {
		xil_printf ("Thread 1 launched with ID %d \r\n",tid1);
	}

	//start thread 2
	ret = pthread_create (&tid2, NULL, (void*)thread_func_2, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_2...\r\n", ret);
	}
	else {
		xil_printf ("Thread 2 launched with ID %d \r\n",tid2);
	}

	//start thread 3
	ret = pthread_create (&tid3, NULL, (void*)thread_func_3, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_3...\r\n", ret);
	}
	else {
		xil_printf ("Thread 3 launched with ID %d \r\n",tid3);
	}

	//start thread 4
	ret = pthread_create (&tid4, NULL, (void*)thread_func_4, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_4...\r\n", ret);
	}
	else {

		xil_printf ("Thread 4 launched with ID %d \r\n",tid4);

	}

}

static void MailboxExample_Receive(XMbox *MboxInstancePtr, ball_msg *ball_pointer)
{
	XMbox_ReadBlocking(MboxInstancePtr,	ball_pointer,16);
	pthread_mutex_lock(&mutex);
	xil_printf("direction : %d \r\n", ball_pointer->dir);
	xil_printf("colour : %d \r\n", ball_pointer->colour);
	xil_printf("x : %d \r\n", ball_pointer->x);
	xil_printf("y : %d \r\n", ball_pointer->y);
	pthread_mutex_unlock(&mutex);
}

