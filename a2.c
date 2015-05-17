#include <stdio.h>
#include "mpi.h"
#define MAX_RAND 5
int main( argc, argv )
int argc;
char **argv;
{
	int rank, size;
	double start;
	MPI_Status status;
	MPI_Comm new_comm;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	FILE *fp;
	fp = fopen("log.txt", "a");
	start = MPI_Wtime();
		
	/* Split communicator into base station and other nodes */
	MPI_Comm_split(MPI_COMM_WORLD, rank==0, 0, &new_comm);

	if(rank==0){
		fprintf(fp, "\nTOTAL EVENTS:\t%d\n\n", master_node(MPI_COMM_WORLD));
		fprintf(fp,"\nSIMULATION TIME:\t%f\n\n", MPI_Wtime()-start);
	}else
		slave_node(MPI_COMM_WORLD, new_comm);
	MPI_Finalize();
	return 0;
}
/*
 * The master_node() function implements the the WSN base station
 * The WSN base station receives updates from all the other nodes, exluding one node
 * in each adjacency group, and detects any events that have occured
 * return	number of events detected
 */
int master_node(old_comm)
{
	int i, size, total_events=0, event, msg=0;
	MPI_Status status;
	MPI_Comm_size(old_comm, &size);
	FILE *fp;
	fp = fopen("log.txt", "a");
	
	for(i=0; i<size-1; i++){
		/* Receive updates from 3/4 nodes in each adjacency */
		if(i%4!=0){
			MPI_Recv(&event, 1, MPI_INT, i+1, 0, old_comm, &status);
			msg++;
			fprintf(fp, "%d:\t Sent %d to base station\n", i,event);
			/* Detect events */
			if(event==4){
				printf("Event detected\n");
				total_events++;
			}
			
		}
	}
	fprintf(fp, "%d messages sent to the base station\n", msg);
	return total_events;
}

int slave_node(old_comm, comm)
{
	int rank, size, s, r, event=0;
	MPI_Status status;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	FILE *fp;
	fp = fopen("log.txt", "a");
	/* Generate random number */
	srandom(time(NULL)+rank);
	s = (random()+rank)%MAX_RAND;
	/* Separate groups of four */
	switch(rank % 4){
		case 0:
			/* Send random number to the next adjacent node*/
			MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d to process %d\n", rank, s, rank+1);
			MPI_Recv(&r, 1, MPI_INT, rank+3, 0, comm, &status);
			event++;
			if(s == r) {
				event++;
			}
			MPI_Send(&event, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d matches to process %d\n", rank, event, rank+1);
			break;
		case 1:
		case 2:
			MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
			MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d to process %d\n", rank, s, rank+1);
			MPI_Recv(&event, 2, MPI_INT, rank-1, 0, comm, &status);
			if(s == r) {
				event++;
			}
			MPI_Send(&event, 1, MPI_INT, 0, 0, old_comm);
			MPI_Send(&event, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d matches to process %d\n", rank, event, rank+1);
			break;
		case 3:
			MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
			MPI_Send(&s, 1, MPI_INT, rank-3, 0, comm);
			fprintf(fp, "%d:\t Sent %d to process %d\n", rank, s, rank-3);
			MPI_Recv(&event, 2, MPI_INT, rank-1, 0, comm, &status);
			if(s == r) {
				event++;
			}
			MPI_Send(&event, 1, MPI_INT, 0, 0, old_comm);
			break;
	}
	return 0;
}