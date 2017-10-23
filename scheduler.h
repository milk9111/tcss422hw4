/*
	10/12/2017
	Authors: Connor Lundberg, Carter Odem
	
	In this project we will be making a simple Round Robin scheduling algorithm
	that will take a single ReadyQueue of PCBs and run them through our scheduler.
	It will simulate the "running" of the process by randomly changing the PC value
	of the process as well as incorporating various interrupts to show the effects
	it has on the scheduling simulator.
	
	This file holds the definitions of structs and declarations functions for the 
	scheduler.c file.
*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

//includes
#include "priority_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//defines
#define MAX_PCB_TOTAL 30
#define MAX_PCB_IN_ROUND 5
#define MAX_PC_JUMP 4000
#define MIN_PC_JUMP 3000
#define PC_JUMP_LIMIT 999
#define IS_TIMER 1
#define SWITCH_CALLS 4
#define RESET_COUNT 10


//structs
/*typedef struct created_list_node {
	PCB pcb;
}

typedef struct created_list {
	
}*/

typedef struct scheduler {
	ReadyQueue created;
	ReadyQueue killed;
	ReadyQueue blocked;
	PriorityQueue ready;
	PCB running;
	PCB interrupted;
	int isNew;
} scheduler_s;

typedef scheduler_s * Scheduler;


//declarations
void timer ();

int makePCBList (Scheduler);

unsigned int runProcess (unsigned int, int);

void pseudoISR (Scheduler);

void scheduling (int, Scheduler);

void dispatcher (Scheduler);

void pseudoIRET (Scheduler);

void printSchedulerState (Scheduler);

Scheduler schedulerConstructor ();

void schedulerDeconstructor (Scheduler);

int isPrivileged(PCB pcb);

void terminate(Scheduler theScheduler);

void resetMLFQ(Scheduler theScheduler);

void resetReadyQueue (ReadyQueue queue);

#endif