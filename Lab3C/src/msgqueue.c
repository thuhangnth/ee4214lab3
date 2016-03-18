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
 -- File           : msgqueue.c
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
#include <pthread.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/timer.h>


pthread_mutex_t uart_mutex;
pthread_t tid1,tid2,tid3,tid4,tid5,disp;
const int COLUMN_WIDTH = 20;
const int COLUMN_LENGTH = 50;
const int COLOUR = 0x00ffff00;
const int COLUMN = -1;

struct msg {
	int x,y,w,l,c,d;
};

void send (int id, int x, int y, int w, int l, int c, int d) {

	struct msg send_msg;
	int ret;
	int msgid;
	send_msg.x = x;
	send_msg.y = y;
	send_msg.w = w;
	send_msg.l = l;
	send_msg.c = c;
	send_msg.d = d;

	//ENTER YOUR CODE HERE TO SEND DATA TO THREAD DISPLAY
	pthread_mutex_lock (&uart_mutex);
	xil_printf("PRODCON: Producer -- Start !\r\n");
	pthread_mutex_unlock (&uart_mutex);
	msgid = msgget (id, IPC_CREAT ) ;
	if( msgid == -1 ) {
		xil_printf ("PRODCON: Producer -- ERROR while opening Message Queue. Errno: %d\r\n", errno) ;
		pthread_exit (&errno);
	}

	if( msgsnd (msgid, &send_msg, 24, 0) < 0 ) { // blocking send
		xil_printf ("PRODCON: Producer -- msgsnd of message(%d) ran into ERROR. Errno: %d. Halting..\r\n", id, errno);
		pthread_exit(&errno);
	}
	pthread_mutex_lock(&uart_mutex);
	xil_printf("PRODCON: Producer done !\r\n");
	pthread_mutex_unlock(&uart_mutex);
}

void* display () {
	struct msg disp_msg;
	int i,msgid;
	while (1) {
		// wait for message from send
		//ENTER YOUR CODE HERE TO RECEIVE THE DATA FROM THREADS
		for (i=1; i<6; i++) {
			msgid = msgget (i, IPC_CREAT);
			if( msgid == -1 ) {
				xil_printf ("PRODCON: Consumer -- ERROR while opening Message Queue. Errno: %d \r\n", errno);
				pthread_exit(&errno) ;
			}
			if( msgrcv( msgid, &disp_msg, 24, 0,0 ) != 24 ) { // blocking recv
				xil_printf ("PRODCON: Consumer -- msgrcv of message(%d) ran into ERROR. Errno: %d. Halting...\r\n", i, errno);
				//       consret = errno;
				pthread_exit(&errno);
			}
			// print message
			if(disp_msg.d == -1){
				pthread_mutex_lock (&uart_mutex);
				xil_printf ("\rDisplay received : Column (x,y) = (%d,%d), w = %d  l= %d, c=%d  \r\n", disp_msg.x, disp_msg.y,disp_msg.w,disp_msg.l,disp_msg.c);
				pthread_mutex_unlock (&uart_mutex);
			}
			else {
				pthread_mutex_lock(&uart_mutex);
				xil_printf ("\rDisplay received : Ball (x,y) = (%d,%d), d = %d , c=%d  \r\n", disp_msg.x, disp_msg.y,disp_msg.d,disp_msg.c);
				pthread_mutex_unlock(&uart_mutex);
			}
		}

	}
}


void* thread_func_1 () {
	while(1) {
		send(1,10,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR,COLUMN);
	}
}


void* thread_func_2 () {
	while(1) {
		send(2,50,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR,COLUMN);
	}
}



void* thread_func_3 () {
	while(1) {
		send(3,90,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR,COLUMN);
	}
}

void* thread_func_4 ()
{
	while(1)
	{
		send(4,130,20,COLUMN_WIDTH,COLUMN_LENGTH,COLOUR,COLUMN);
	}
}

void* thread_func_5 ()
{
	while(1)
	{
		send(5,200,400,-1,-1,COLOUR,7);
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
	pthread_mutexattr_t attr;

	print("-- Entering main() --\r\n");
	ret = pthread_mutexattr_init(&attr);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) initializing mutex attr...\r\n", ret);
	}
	else {
		xil_printf ("Mutex attr initialized\r\n");
	}

	ret = pthread_mutex_init(&uart_mutex, &attr);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching mutex...\r\n", ret);
	}
	else {
		xil_printf ("Mutex initialized\r\n \r\n");
	}

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

	//start thread 5
	ret = pthread_create (&tid5, NULL, (void*)thread_func_5, NULL);
	if (ret != 0) {
		xil_printf ("-- ERROR (%d) launching thread_func_5..\r\n", ret);
	}
	else {
		xil_printf ("Thread 5 launched with ID %d \r\n",tid5);
	}



}

