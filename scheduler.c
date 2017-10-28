/*
	10/28/2017
	Authors: Connor Lundberg, Jacob Ackerman
	
	In this project we will be making an MLFQ scheduling algorithm
	that will take a PriorityQueue of PCBs and run them through our scheduler.
	It will simulate the "running" of the process by incrementing the running PCB's PCB 
	by one on each loop through. It will also incorporate various interrupts to show 
	the effects it has on the scheduling simulator.
	
	This file holds the defined functions declared in the scheduler.h header file.
*/

#include "scheduler.h"


unsigned int sysstack;
int switchCalls;

PCB privileged[4];
int privilege_counter = 0;
int ran_term_num = 0;
int terminated = 0;
int currQuantumSize;
int quantum_tick = 0; // Use for quantum length tracking
int io_timer = 0;
time_t t;


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
	This creates the list of new PCBs for the current loop through. It simulates
	the creation of each PCB, the changing of state to new, enqueueing into the
	list of created PCBs, and moving each of those PCBs into the ready queue.
*/
int makePCBList (Scheduler theScheduler) {
	int newPCBCount = rand() % MAX_PCB_IN_ROUND;
	//int newPCBCount = 3;
	
	int lottery = rand();
	for (int i = 0; i < newPCBCount; i++) {
		PCB newPCB = PCB_create();
		newPCB->state = STATE_NEW;
		q_enqueue(theScheduler->created, newPCB);
		
		// creates privileged pcb (from HW3)
		/*if (privilege_counter < 4 && lottery % 7 == 0) {
			privileged[privilege_counter] = newPCB;
			// printf("PRIVILEGE PID: %d\n", privileged[privilege_counter]->pid);
			privilege_counter++;
		}*/
	}
	printf("Making New PCBs: \r\n");
	if (newPCBCount) {
		while (!q_is_empty(theScheduler->created)) {
			PCB nextPCB = q_dequeue(theScheduler->created);
			nextPCB->state = STATE_READY;
			toStringPCB(nextPCB, 0);
			printf("\r\n");
			pq_enqueue(theScheduler->ready, nextPCB);
		}
		printf("\r\n");

		if (theScheduler->isNew) {
			printf("Dequeueing PCB ");
			toStringPCB(pq_peek(theScheduler->ready), 0);
			printf("\r\n\r\n");
			theScheduler->running = pq_dequeue(theScheduler->ready);
			theScheduler->running->state = STATE_RUNNING;
			theScheduler->isNew = 0;
			currQuantumSize = theScheduler->ready->queues[0]->quantum_size;
		}
	}
	
	return newPCBCount;
}


/*
	Creates a random number between 3000 and 4000 and adds it to the current PC.
	It then returns that new PC value.
*/
unsigned int runProcess (unsigned int pc, int quantumSize) {
	//quantumSize is the difference in time slice length between
	//priority levels.
	unsigned int jump;
	if (quantumSize != 0) {
		jump = rand() % quantumSize;
	}
	
	pc += jump;
	return pc;
}



void terminate(Scheduler theScheduler) {
	//ran_term_num = rand() % RANDOM_VALUE;
	/*	
	if (theScheduler->running != NULL && ran_term_num <= MAX_VALUE_PRIVILEGED && isPrivileged(theScheduler->running) == 0) {
		theScheduler->running->state = STATE_HALT;
	}
	*/
	if(theScheduler->running != NULL && theScheduler->running->terminate > 0 && theScheduler->running->terminate == theScheduler->running->term_count)
	{
		theScheduler->running->state = STATE_HALT;
		scheduling(-1, theScheduler);	
	}
	
}


/*
	This acts as an Interrupt Service Routine, but only for the Timer interrupt.
	It handles changing the running PCB state to Interrupted, moving the running
	PCB to interrupted, saving the PC to the SysStack and calling the scheduler.
*/
void pseudoISR (Scheduler theScheduler, int interruptType) {
	if (theScheduler->running->state != STATE_HALT) {
		theScheduler->running->state = STATE_INT;
		theScheduler->interrupted = theScheduler->running;
		theScheduler->running->context->pc = sysstack;
	}
	scheduling(interruptType, theScheduler);
	pseudoIRET(theScheduler);
}


/*
	Prints the state of the Scheduler. Mostly this consists of the MLFQ, the next
	highest priority PCB in line, the one that will be run on next iteration, and
	the current list of "privileged PCBs" that will not be terminated.
*/
void printSchedulerState (Scheduler theScheduler) {
	printf("MLFQ State\r\n");
	toStringPriorityQueue(theScheduler->ready);
	printf("\r\n");
	
	int index = 0;
	// PRIVILIGED PID
	while(privileged[index] != NULL && index < MAX_PRIVILEGE) {
		printf("PCB PID %d, PRIORITY %d, PC %d\n", 
		privileged[index]->pid, privileged[index]->priority, 
		privileged[index]->context->pc);
		index++;
	}
	printf("blocked size: %d\r\n", theScheduler->blocked->size);
	printf("killed size: %d\r\n", theScheduler->killed->size);
	printf("\r\n");
	
	if (pq_peek(theScheduler->ready)) {
		printf("Going to be running ");
		toStringPCB(theScheduler->running, 0);
		printf("\r\n");
		printf("Next highest priority PCB ");
		toStringPCB(pq_peek(theScheduler->ready), 0);
		printf("\r\n\r\n\r\n");
	} else {
		printf("Going to be running next ");
		if (theScheduler->running) {
			toStringPCB(theScheduler->running, 0);
		} else {
			printf("\r\n");
		}

		printf("Next highest priority PCB contents: The MLFQ is empty!\r\n");
		printf("\r\n\r\n\r\n");
	}
}


/*
	Used to move every value in the MLFQ back to the highest priority
	ReadyQueue after a predetermined time. It does this by taking the first value
	of each ReadyQueue (after the 0 *highest priority* queue) and setting it to
	be the new last value of the 0 queue.
*/
void resetMLFQ (Scheduler theScheduler) {
	for (int i = 1; i < NUM_PRIORITIES; i++) {
		ReadyQueue curr = theScheduler->ready->queues[i];
		if (!q_is_empty(curr)) {
			if (!q_is_empty(theScheduler->ready->queues[0])) {
				theScheduler->ready->queues[0]->last_node->next = curr->first_node;
				theScheduler->ready->queues[0]->last_node = curr->last_node;
				theScheduler->ready->queues[0]->size += curr->size;
			} else {
				theScheduler->ready->queues[0]->first_node = curr->first_node;
				theScheduler->ready->queues[0]->last_node = curr->last_node;
				theScheduler->ready->queues[0]->size = curr->size;
			}
			resetReadyQueue(curr);
		}
	}
}


void resetReadyQueue (ReadyQueue queue) {
	ReadyQueueNode ptr = queue->first_node;
	while (ptr) {
		ptr->pcb->priority = 0;
		ptr = ptr->next;
	}
	queue->first_node = NULL;
	queue->last_node = NULL;
	queue->size = 0;
}

/*
	If the interrupt that occurs was a Timer interrupt, it will simply set the 
	interrupted PCBs state to Ready and enqueue it into the Ready queue. It then
	calls the dispatcher to get the next PCB in the queue.
*/
void scheduling (int interrupt_code, Scheduler theScheduler) {
	if (interrupt_code == IS_TIMER && theScheduler->running->state != STATE_HALT) {
		theScheduler->interrupted->state = STATE_READY;
		if (theScheduler->interrupted->priority < (NUM_PRIORITIES - 1)) {
			theScheduler->interrupted->priority++;
		} else {
			theScheduler->interrupted->priority = 0;
		}
		pq_enqueue(theScheduler->ready, theScheduler->interrupted);
		
		int index = isPrivileged(theScheduler->running);
		
		if (index != 0) {
			privileged[index] = theScheduler->running;
		}
	}
	else if (interrupt_code == IS_IO_TRAP && theScheduler->running->state != STATE_HALT)
	{
		// Do I/O trap handling
		int timer = currQuantumSize * (rand()%3+2) + rand()%500;
		theScheduler->running->blocked_timer = timer;
		theScheduler->running->state = STATE_WAIT;
		q_enqueue(theScheduler->blocked, theScheduler->running);
		
		// schedule a new process
	}
	else if (interrupt_code == IS_IO_INTERRUPT && theScheduler->running->state != STATE_HALT)
	{
		// Do I/O interrupt handling
		pq_enqueue(theScheduler->ready, q_dequeue(theScheduler->blocked));
		theScheduler->running = theScheduler->interrupted;
		theScheduler->running->state = STATE_RUNNING;
		sysstack = theScheduler->running->context->pc;
	}
	
	if (theScheduler->running->state == STATE_HALT) {
		q_enqueue(theScheduler->killed, theScheduler->running);
		theScheduler->running = NULL;
		
		terminated++;
	}
	
	// I/O interrupt does not require putting a new process
	// into the running state, so we ignore this.
	if(interrupt_code != IS_IO_INTERRUPT)
	{
		theScheduler->running = pq_peek(theScheduler->ready);
	}
	
	if (terminated >= TOTAL_TERMINATED) {
		while(!q_is_empty(theScheduler->killed)) {
			PCB_destroy(q_dequeue(theScheduler->killed));
		}
	}

	// I/O interrupt does not require putting a new process
	// into the running state, so we ignore this.
	if(interrupt_code != IS_IO_INTERRUPT) 
	{
		dispatcher(theScheduler);
	}
}


/*
	This simply gets the next ready PCB from the Ready queue and moves it into the
	running state of the Scheduler.
*/
void dispatcher (Scheduler theScheduler) {
	if (pq_peek(theScheduler->ready) != NULL && pq_peek(theScheduler->ready)->state != STATE_HALT) {
		currQuantumSize = getNextQuantumSize(theScheduler->ready);
		theScheduler->running = pq_dequeue(theScheduler->ready);
		theScheduler->running->state = STATE_RUNNING;
	}
}


/*
	This simply sets the running PCB's PC to the value in the SysStack;
*/
void pseudoIRET (Scheduler theScheduler) {
	theScheduler->running->context->pc = sysstack;
}


/*
	This will construct the Scheduler, along with its numerous ReadyQueues and
	important PCBs.
*/
Scheduler schedulerConstructor () {
	Scheduler newScheduler = (Scheduler) malloc (sizeof(scheduler_s));
	newScheduler->created = q_create();
	newScheduler->killed = q_create();
	newScheduler->blocked = q_create();
	newScheduler->ready = pq_create();
	newScheduler->running = NULL;
	newScheduler->interrupted = NULL;
	newScheduler->isNew = 1;
	
	return newScheduler;
}


/*
	This will do the opposite of the constructor with the exception of 
	the interrupted PCB which checks for equivalancy of it and the running
	PCB to see if they are pointing to the same freed process (so the program
	doesn't crash).
*/
void schedulerDeconstructor (Scheduler theScheduler) {
	q_destroy(theScheduler->created);
	q_destroy(theScheduler->killed);
	q_destroy(theScheduler->blocked);
	pq_destroy(theScheduler->ready);
	PCB_destroy(theScheduler->running);
	if (theScheduler->interrupted == theScheduler->running) {
		PCB_destroy(theScheduler->interrupted);
	}
	free (theScheduler);
}

int isPrivileged(PCB pcb) {
	if (pcb != NULL) {
		for (int i = 0; i < 4; i++) {
			if (privileged[i] == pcb) {
				return i;
			}	
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
