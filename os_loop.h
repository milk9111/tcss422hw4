/*
	10/28/2017
	Authors: Connor Lundberg, Jacob Ackerman
	
	In this project we will be making an MLFQ scheduling algorithm
	that will take a PriorityQueue of PCBs and run them through our scheduler.
	It will simulate the "running" of the process by incrementing the running PCB's PCB 
	by one on each loop through. It will also incorporate various interrupts to show 
	the effects it has on the scheduling simulator.
	
	This file holds the function declarations to be used in the os_loop.c file. This 
	is being used to reduce the size of scheduler.c for readability and management.
*/

#ifndef OS_LOOP_H
#define OS_LOOP_H


//includes
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//declarations
void osLoop ();

int timerInterrupt (int);

int ioTrap (PCB);

int ioInterrupt (ReadyQueue);

#endif