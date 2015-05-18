#include <stdio.h>
#include "mpi.h"
#define MAX_RAND 10
#define LOG_FILE "log.txt"
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
	fp = fopen(LOG_FILE, "a");
	start = MPI_Wtime();
		
	/* Split communicator into base station and other nodes */
	MPI_Comm_split(MPI_COMM_WORLD, rank==0, 0, &new_comm);

	if(rank==0){
		fprintf(fp, "\nTOTAL EVENTS:\t%d\n\n", base_station(MPI_COMM_WORLD));
		fprintf(fp,"\nSIMULATION TIME:\t%f\n\n", MPI_Wtime()-start);
	}else
		slave_node(MPI_COMM_WORLD, new_comm);
	MPI_Finalize();
	return 0;
}

/* The base_station() function implements the the WSN base station
 * The WSN base station receives updates from all the other nodes, exluding one node
 * in each adjacency group, and detects any events that have occured
 * return	number of events detected
 */
int base_station(old_comm)
{
	int i, size, total_events=0, num_matches, msg=0;
	MPI_Status status;
	MPI_Comm_size(old_comm, &size);
	FILE *fp;
	fp = fopen(LOG_FILE, "a");
	
	for(i=0; i<size-1; i++){
		/* Receive updates from 3/4 nodes in each adjacency */
		if(i%4!=0){
			MPI_Recv(&num_matches, 1, MPI_INT, i+1, 0, old_comm, &status);
			msg++;
			fprintf(fp, "%d:\t Sent %d num_matches to base station\n", i,num_matches);
			/* Detect event */
			if(num_matches==4){
				printf("Event detected from node %d\n", i);
				total_events++;
			}
			
		}
	}
	fprintf(fp, "%d messages sent to the base station\n", msg);
	return total_events;
}

/* The slave_node() function implements all the communications between
 * the 64 nodes in the 8x8 grid. The nodes are taken four at a time,
 * each group of four adjacent nodes sends a random number to the next
 * node and receives one from the previous node. The number sent and
 * the number received are compared and num_matches is incremented i
 * they are equal. The first 3 nodes send num_matches to the next node,
 * the last 3 nodes send num_matches to the base station.
 */
int slave_node(old_comm, comm)
{
	int rank, s, r, num_matches=0;
	MPI_Status status;
	MPI_Comm_rank(comm, &rank);
	FILE *fp;
	fp = fopen(LOG_FILE, "a");
	/* Generate random number */
	srandom(time(NULL)+rank);
	s = (random()+rank)%MAX_RAND;
	
	/* Separate into groups of four adjacent nodes */
	switch(rank % 4){
		case 0:
			/* Send random number to the next adjacent node */
			MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent '%d' to node %d\n", rank, s, rank+1);
			MPI_Recv(&r, 1, MPI_INT, rank+3, 0, comm, &status);
			if(s == r)
				num_matches++;
			/* Send the number of matches to the next node */
			MPI_Send(&num_matches, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d matches to node %d\n", rank, num_matches, rank+1);
			break;
		case 1:
		case 2:
			/* Receive number from the previous node */
			MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
			/* Send random number to the next adjacent node*/
			MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent '%d' to node %d\n", rank, s, rank+1);
			/* Receive the number of matches from the previous node */
			MPI_Recv(&num_matches, 2, MPI_INT, rank-1, 0, comm, &status);
			if(s == r)
				num_matches++;
			/* Send the number of matches to the base station */
			MPI_Send(&num_matches, 1, MPI_INT, 0, 0, old_comm);
			/* Send the number of matches to the next node */
			MPI_Send(&num_matches, 1, MPI_INT, rank+1, 0, comm);
			fprintf(fp, "%d:\t Sent %d num_matches to node %d\n", rank, num_matches, rank+1);
			break;
		case 3:
			/* Receive number from the previous node */
			MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
			/* Send random number to the next adjacent node */
			MPI_Send(&s, 1, MPI_INT, rank-3, 0, comm);
			fprintf(fp, "%d:\t Sent '%d' to node %d\n", rank, s, rank-3);
			/* Receive the number of matches from the previous node */
			MPI_Recv(&num_matches, 2, MPI_INT, rank-1, 0, comm, &status);
			if(s == r)
				num_matches++;
			/* Send the number of matches to the base station */
			MPI_Send(&num_matches, 1, MPI_INT, 0, 0, old_comm);
			break;
	}
	return 0;
}