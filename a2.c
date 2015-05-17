#include <stdio.h>
#include "mpi.h"
#define MAX_RAND 10
int main( argc, argv )
int argc;
char **argv;
{
	int i, rank, size;
	MPI_Status status;
	MPI_Comm new_comm;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	/* Split communicator into base station and other nodes */
	MPI_Comm_split(MPI_COMM_WORLD, rank==0, 0, &new_comm);

	if(rank==0)
		master_node(MPI_COMM_WORLD, new_comm);
	else
		slave_node(MPI_COMM_WORLD, new_comm);
	
	MPI_Finalize();
	return 0;
}

int master_node(old_comm, comm)
{
	int i, size, event[2]={0,0}, flag=0;
	MPI_Request request;
	MPI_Status status;
	MPI_Comm_size(old_comm, &size);
	FILE *fp;
	fp = fopen("log.txt", "a");

	for(i=0; i<size-1; i++){
		if(i%4!=0){
			fprintf(fp,"Receiving from %d\n",i);
			printf("Receiving from %d\n",i);
			MPI_Recv(&event, 2, MPI_INT, i+1, 0, old_comm, &status);
			printf("Received [%d,%d] from %d\n",event[0],event[1],i);
			if(event[1]==4)
				printf("\n***************Event occurred***************\n\n");
		}
	}
	return 0;
}

int slave_node(old_comm, comm)
{
	int rank, size, s, r, event[2]={0,0};
	MPI_Status status;
	MPI_Request request;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	FILE *fp;
	fp = fopen("log.txt", "a");
	if(rank % 4 == 0){
		srandom(time(NULL)+rank);
		s = (random()+rank)%MAX_RAND;
		if(s<0.6*MAX_RAND)
			s=0;
		else
			s=1;
		MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
		MPI_Recv(&r, 1, MPI_INT, rank+3, 0, comm, &status);
		event[0]++;
		if(s == r) {
			event[1]++;
			printf("%d: match with %d [%d, %d]\n", rank, rank+3, event[0],event[1]);
			printf("###match###\n");
			
		}
		MPI_Send(&event, 2, MPI_INT, rank+1, 0, comm);
	
	} if(rank % 4 == 1){
		s = (random()+rank)%MAX_RAND;
		if(s<0.6*MAX_RAND)
			s=0;
		else
			s=1;
		MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
		MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
		MPI_Recv(&event, 2, MPI_INT, rank-1, 0, comm, &status);
		event[0]++;
		if(s == r) {
			event[1]++;
			printf("%d: match with %d [%d, %d]\n", rank, rank-1, event[0],event[1]);
			printf("###match###\n");
			
		}
		MPI_Send(&event, 2, MPI_INT, 0, 0, old_comm);
		printf("%d: Sent [%d,%d]to base station\n", rank,event[0],event[1]);
		MPI_Send(&event, 2, MPI_INT, rank+1, 0, comm);
	
	} if(rank % 4 == 2){
		s = (random()+rank)%MAX_RAND;
		if(s<0.6*MAX_RAND)
			s=0;
		else
			s=1;
		MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
		MPI_Send(&s, 1, MPI_INT, rank+1, 0, comm);
		MPI_Recv(&event, 2, MPI_INT, rank-1, 0, comm, &status);
		event[0]++;
		if(s == r) {
			event[1]++;
			printf("%d: match with %d [%d, %d]\n", rank, rank-1, event[0],event[1]);
			printf("###match###\n");
			
		}
		MPI_Send(&event, 2, MPI_INT, 0, 0, old_comm);
		printf("%d: Sent [%d,%d]to base station\n", rank,event[0],event[1]);
		MPI_Send(&event, 2, MPI_INT, rank+1, 0, comm);
	
	} if(rank % 4 == 3){
		s = (random()+rank)%MAX_RAND;
		if(s<0.6*MAX_RAND)
			s=0;
		else
			s=1;
		MPI_Recv(&r, 1, MPI_INT, rank-1, 0, comm, &status);
		MPI_Send(&s, 1, MPI_INT, rank-3, 0, comm);
		MPI_Recv(&event, 2, MPI_INT, rank-1, 0, comm, &status);
		event[0]++;
		if(s == r) {
			event[1]++;
			printf("%d: match with %d [%d, %d]\n", rank, rank-1, event[0],event[1]);
			printf("###match###\n");
			
		}
		MPI_Send(&event, 2, MPI_INT, 0, 0, old_comm);
		printf("%d: Sent [%d,%d]to base station\n", rank,event[0],event[1]);
	}
	fprintf(fp,"Process %d sent %d and got %d\n", rank, s, r);
	return 0;
}