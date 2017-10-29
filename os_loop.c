/*
	10/28/2017
	Authors: Connor Lundberg, Jacob Ackerman
	
	In this project we will be making an MLFQ scheduling algorithm
	that will take a PriorityQueue of PCBs and run them through our scheduler.
	It will simulate the "running" of the process by incrementing the running PCB's PCB 
	by one on each loop through. It will also incorporate various interrupts to show 
	the effects it has on the scheduling simulator.
	
	This file holds the defined functions declared in the os_loop.h header file. This 
	is being used to reduce the size of scheduler.c for readability and management.
*/

#include "os_loop.h"


unsigned int sysstack;
int switchCalls;
int ran_term_num = 0;
int terminated = 0;
int currQuantumSize;
int quantum_tick = 0; // Use for quantum length tracking
int io_timer = 0;
time_t t;

//This was the HW3 loop. This is here for reference when making the new loop.
/*void timer () {
	unsigned int pc = 0;
	int totalProcesses = 0, iterationCount = 1;
	Scheduler thisScheduler = schedulerConstructor();
	for (;;) {
		if (totalProcesses >= MAX_PCB_TOTAL) {
			printf("Reached max PCBs, ending Scheduler.\r\n");
			break;
		}
		printf("Iteration: %d\r\n", iterationCount);
		if (!(iterationCount % RESET_COUNT)) {
			printf("\r\nRESETTING MLFQ\r\n");
			resetMLFQ(thisScheduler);
		}
		totalProcesses += makePCBList(thisScheduler);		
		
		if (totalProcesses > 1) {
			pc++; //= runProcess(pc, currQuantumSize);
			sysstack = pc;
			terminate(thisScheduler); 
			if(quantum_tick >= currQuantumSize)
			{
				pseudoISR(thisScheduler);
				quantum_tick = 0;
			}
			pc = thisScheduler->running->context->pc;
		}
		
		printSchedulerState(thisScheduler);
		iterationCount++;
		quantum_tick++;
		
		
	}
	schedulerDeconstructor(thisScheduler);
}*/


/*
	This function is our main loop. It creates a Scheduler object and follows the
	steps a normal MLFQ Priority Scheduler would to "run" for a certain length of time,
	check for all interrupt types, then call the ISR, scheduler,
	dispatcher, and eventually an IRET to return to the top of the loop and start
	with the new process.
*/
void osLoop () {
	int totalProcesses = 0, iterationCount = 1;
	Scheduler thisScheduler = schedulerConstructor();
	totalProcesses += makePCBList(thisScheduler);
	printSchedulerState(thisScheduler);
	for(;;) {
		if (thisScheduler->running) { // In case the first makePCBList makes 0 PCBs
			thisScheduler->running->context->pc++;
			
			if (timerInterrupt(iterationCount) == 1) {
				pseudoISR(thisScheduler, IS_TIMER);
				printf("Completed Timer Interrupt\n");
				printSchedulerState(thisScheduler);
				iterationCount++;
			}
			
			if (ioTrap(thisScheduler->running) == 1) {
				printf("Iteration: %d\r\n", iterationCount);
				printf("Initiating I/O Trap\r\n");
				printf("PC when I/O Trap is Reached: %d\r\n", thisScheduler->running->context->pc);
				pseudoISR(thisScheduler, IS_IO_TRAP);
				printf("Completed I/O Trap\n");
				printSchedulerState(thisScheduler);
				iterationCount++;
			}
			
			if (ioInterrupt(thisScheduler->blocked) == 1) {
				printf("Iteration: %d\r\n", iterationCount);
				printf("Initiating I/O Interrupt\n");
				pseudoISR(thisScheduler, IS_IO_INTERRUPT);
				printf("Completed I/O Interrupt\n");
				printSchedulerState(thisScheduler);
				iterationCount++;
			}
			
			if (thisScheduler->running->context->pc == thisScheduler->running->max_pc) {
				thisScheduler->running->context->pc = 0;
				thisScheduler->running->term_count++;	//if terminate value is > 0
			}
		} else {
			iterationCount++;
		}
	
		// if running PCB's terminate == running PCB's term_count, then terminate (for real).
		terminate(thisScheduler);
		
		if (!(iterationCount % RESET_COUNT)) {
			printf("\r\nRESETTING MLFQ\r\n");
			printf("iterationCount: %d\n", iterationCount);
			resetMLFQ(thisScheduler);
			totalProcesses += makePCBList (thisScheduler);
			printSchedulerState(thisScheduler);
			iterationCount = 1;
		}
		
		if (totalProcesses >= MAX_PCB_TOTAL) {
			printf("Reached max PCBs, ending Scheduler.\r\n");
			break;
		}
	}
}


/*
	Checks if the global quantum tick is greater than or equal to
	the current quantum size for the running PCB. If so, then reset
	the quantum tick to 0 and return 1 so the pseudoISR can occur.
	If not, increase quantum tick by 1.
*/
int timerInterrupt(int iterationCount)
{
	if (quantum_tick >= currQuantumSize)
	{
		printf("Iteration: %d\r\n", iterationCount);
		printf("Initiating Timer Interrupt\n");
		printf("Current quantum tick: %d\r\n", quantum_tick);
		quantum_tick = 0;
		return 1;
	}
	else
	{
		quantum_tick++;
		return 0;
	}
}


/*
	Checks if the current PCB's PC is one of the premade io_traps for this
	PCB. If so, then return 1 so the pseudoISR can occur. If not, return 0.
*/
int ioTrap(PCB current)
{
	unsigned int the_pc = current->context->pc;
	int c;
	for (c = 0; c < TRAP_COUNT; c++)
	{
		if(the_pc == current->io_1_traps[c])
		{
			return 1;
		}
	}
	
	for (c = 0; c < TRAP_COUNT; c++)
	{
		if(the_pc == current->io_2_traps[c])
		{
			return 1;
		}
	}
	return 0;
}


int ioInterrupt(ReadyQueue the_blocked)
{
	//printf("here2\n");
	//printf("currQuantumSize: %d, quantum_tick: %d\n", currQuantumSize, quantum_tick);
	if (the_blocked->first_node != NULL && q_peek(the_blocked) != NULL)
	{
		//printf("here3\n");
		PCB nextup = q_peek(the_blocked);
		if (io_timer >= nextup->blocked_timer)
		{
			io_timer = 0;
			return 1;
		}
		else
		{
			io_timer++;
		}
	}
	
	return 0;
}


void main () {
	setvbuf(stdout, NULL, _IONBF, 0);
	srand((unsigned) time(&t));
	sysstack = 0;
	switchCalls = 0;
	currQuantumSize = 0;
	osLoop();
}