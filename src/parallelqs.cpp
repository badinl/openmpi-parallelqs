#include<iostream>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include<cmath>
#include<cstring>
#include<chrono>
#include<mpi.h>
#define INFO 0
#define ADVANCED_MERGE 1

#include"listout.h"
#include"quicksort.h"
#include"merge.h"

bool checkIndices(int indices[], int size) {
	for(int i=0; i<size; i++)
		if(indices[i] != -1) return true;
	return false;
}

int main(int argc, char *argv[]) {
	int size, sub_size;

	int whoami, np;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Status *status;

	uint64_t start, sortstart, sortend, mergestart, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(comm, &np);
	MPI_Comm_rank(comm, &whoami);

/*
	if((np & (np - 1)) != 0) {
		if(whoami == 0) std::cerr << "Please run this with np = power of 2" << std::endl;
		MPI_Abort(comm,1);
		return 1;
	}
*/
	if(argc < 2) {
		if(whoami == 0) std::cerr << "Please provide input file path" << std::endl;
		MPI_Abort(comm,2);
		return 2;
	}

	int * input;
	int * sub_input;

	std::ifstream file;
	if(whoami == 0) {
		//open input file
		std::string path = argv[1];
		file.open(path);
		if(!file) {
			std::cerr << "Could not open input file " << path << std::endl;
			return 2;
		}

    	if(INFO && whoami == 0) std::cerr << "Reading size" << std::endl;
		//load size
		file >> size;
	}

	MPI_Barrier(comm);
	MPI_Bcast(&size,1,MPI_INT,0,comm);
	input = (int*)malloc(sizeof(int) * size);

    if(INFO && whoami == 0) std::cerr << "Reading input" << std::endl;
	if(whoami == 0) {
		for(int i=0; i<size; i++) file >> input[i];
		file.close();
	}

	sub_size = size / np;
	sub_input = (int*)malloc(sizeof(int) * sub_size);

	MPI_Barrier(comm);

	start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	//scatter
    if(INFO && whoami == 0) std::cerr << "Scattering" << std::endl;
	MPI_Scatter(input, sub_size, MPI_INT,
				sub_input, sub_size, MPI_INT,
				0,comm);

	sortstart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	//qs
    if(INFO && whoami == 0) std::cerr << "Sorting locally" << std::endl;
	srand(time(0));
	quicksort(sub_input,sub_size,0,sub_size);
    if(INFO) std::cerr << "[" << whoami << "]" << "Sorted" << std::endl;

	MPI_Barrier(comm);

	sortend = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if(whoami == 0) std::cerr << "Time sorting: " << sortend - sortstart << std::endl;

	//gather
    if(INFO && whoami == 0) std::cerr << "Gathering" << std::endl;
	MPI_Gather (sub_input, sub_size, MPI_INT,
				input, sub_size, MPI_INT,
				0,comm);

	mergestart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	int * output;

	if(INFO && whoami == 0) std::cerr << "Merging" << std::endl;

	free(sub_input);

	MPI_Barrier(comm);

	//parallel d&c merge
	if(whoami % 2 == 0 && ADVANCED_MERGE == 2) {
		if(whoami == 0) {
			for(int i = 2; i < np; i += 2) {
				//send pair i,i+1 to each i : i%2 == 0 (except yourself)
				MPI_Send(input + i*sub_size, 2*sub_size, MPI_INT, i, 0, comm);
				if(INFO) std::cerr << "[" << whoami << "]" << "Sent to " << i << " with offset " << i*sub_size << std::endl;
			}
			output = input;
		}
		else if(whoami % 2 == 0) {
			output = (int *) malloc(sizeof(int) * 2 * sub_size);
			MPI_Recv(output, 2*sub_size, MPI_INT, 0, 0, comm, status);
			if(INFO) std::cerr << "[" << whoami << "]" << "Received" << std::endl;
			//receive pair i,i+1 from 0
		}

		free(input);

		//MPI_Barrier(comm);
		for(int round = 1; round <= log2(np); round++) {
			//round 1: 0, 2, 4, 6, 8...
			//round 2: 0, 4, 8, 12
			//round 3: 0, 8

			if(INFO && whoami == 0) std::cerr << "Round " << round << " of " << log2(np) << std::endl;


			if(whoami % (int)pow(2,round) == 0) {
				//binary merge two instances of size 2^(round-1) each to produce an instance of size pow(2,round)
				if(INFO) std::cerr << "[" << whoami << "]" << "Locally merging - data ptr " << output << " - seq_size " << sub_size*pow(2,round-1) << " - seq_n " << 2 << std::endl;
				int * out = merge(output, sub_size*pow(2,round-1), 2);
				if(INFO) std::cerr << "[" << whoami << "]" << "Merged" << std::endl;

				if(whoami % (int)pow(2,round+1) == 0) { //am i the receiver?
					//make room
					if(INFO) std::cerr << "[" << whoami << "]" << "Making room (" << sizeof(int) << " x " << sub_size * pow(2,round) << ")" << std::endl;
					int * next = (int*) malloc(sizeof(int) * sub_size * pow(2,round));
					//copy this processor's answer
					if(INFO) std::cerr << "[" << whoami << "]" << "Copying my solution" << std::endl;
					memcpy(next, out, sizeof(int) * sub_size * pow(2,round-1));
					//place output ptr after it to receive the other half
					output = next + sub_size * (int)pow(2,round-1);
					//receive from whoami + pow(2,round) instance of size pow(2,round)
					if(INFO) std::cerr << "[" << whoami << "]" << "Receiving other solution" << std::endl;
					MPI_Recv(output, sub_size*pow(2,round),MPI_INT,whoami + pow(2,round),0,comm,status);
					//reset output to the beginning, ready for next round
					output = next;
					//if(INFO) std::cerr << "[" << whoami << "]" << "Freeing next" << std::endl;
					//free(next);
					if(INFO) std::cerr << "[" << whoami << "]" << "Freeing out" << std::endl;
					free(out);
				} else { //am i the sender?
					//send to whoami - pow(2,round) instance of size pow(2,round)
					if(INFO) std::cerr << "[" << whoami << "]" << "Sending my solution" << std::endl;
					MPI_Send(out, sub_size*pow(2,round),MPI_INT,whoami - pow(2,round),0,comm);
					if(INFO) std::cerr << "[" << whoami << "]" << "Freeing out" << std::endl;
					free(out);
				}
			}
		}
	}

	//d&c merge
	if(whoami == 0 && ADVANCED_MERGE == 1)
		output = merge(input, sub_size, np);

	//merge
	if(whoami == 0 && ADVANCED_MERGE == 0) {
		output = (int*)malloc(sizeof(int) * size);

		//initialise indices
		int indices[np];
		for(int i=0; i<np; i++) indices[i] = 0;

		//while we haven't exhausted all indices
		int next = 0; //next place in the output
		while(checkIndices(indices,np)){
			//find min among unexpired (!= -1) subinputs
			int min = -1;
			for(int i=0; i<np; i++) {
				if(indices[i] != -1) {
					if(min == -1) min = i; //select first min as smallest nonexpired index
					else if(input[i*sub_size + indices[i]] <= input[min*sub_size + indices[min]])
						min = i;
				}
			}

			//place it and update
			output[next++] = input[min*sub_size + (indices[min]++)];

			//is the index out of bounds? set -1 (this subinput has expired)
			if(indices[min] == sub_size) indices[min] = -1;
		}
	}


	if(INFO && whoami == 0) std::cerr<< "Done!" << std::endl;

	if(whoami == 0) {
		end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		std::cerr << "Time merging: " << end - mergestart << std::endl;
		std::cerr << "Time overall: " << end - start << std::endl;
		std::cerr << "Writing output." << std::endl;
		listout(output, size);
	}

	MPI_Finalize();
	return 0;
}