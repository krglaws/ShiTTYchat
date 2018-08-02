#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"

int main(int argc, char *argv[]){
	
	if (argc < 3){
		printf("Usage: './omp_monte_carlo <threads> <N>'\nExiting... \n");
		exit(-1);
	}

	int threads = strtol(argv[1], NULL, 10);
	int n = strtol(argv[2], NULL, 10)/threads;

	double glob_in = 0;
	double glob_out = 0;
	double start, stop;

	printf("Starting with %d threads...\n", threads);
	GET_TIME(start);
	#pragma omp parallel num_threads(threads)
	{

		double x, y, dist;
		double loc_in = 0;
		double loc_out = 0;

		srand(omp_get_thread_num());

		for (int i = 0; i < n/omp_get_num_threads(); i++){

			x = (double) rand() / (double) RAND_MAX;
			y = (double) rand() / (double) RAND_MAX;

			dist = sqrt((x*x)+(y*y));

			if (dist < 1) loc_in++;
			else loc_out++;

		}
		
		#pragma omp atomic
		glob_in += loc_in;
		#pragma omp atomic
		glob_out += loc_out;
	}
	GET_TIME(stop);	
	printf("Estimate of PI: %f\n", 4*(glob_in/(glob_out+glob_in)));
	printf("Computed in %fs\n", stop-start);
	return 0;
}
