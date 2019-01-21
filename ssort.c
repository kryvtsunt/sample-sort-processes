// I have used the starter code provided by Prof.Tuck

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "float_vec.h"
#include "barrier.h"
#include "utils.h"
#include "float.h"


// float comparator 
int floatcomp(const void *n1, const void *n2)
{
	float f1 = *(float*)n1;
	float f2 = *(float*)n2;
	if (f1 < f2) return -1;
	if (f1 > f2) return 1;
	return 0;
}



// qsort given array of floats
	void
qsort_floats(floats* xs)
{
	qsort(xs->data,xs->size,sizeof(float), floatcomp);
}

// make a sample 
	floats*
sample(float* data, long size, int P)
{
	int jj = 0;
	int num = 3*(P - 1);
	// make an array of random items
	floats* array = make_floats(num);
	// randomly select (p-1) items
	while(jj < num) {
		int ind = rand() % size;
		floats_push(array, data[ind]);
		jj++;
	}  
	// sort items
	qsort_floats(array);
	// number of medians is p+1
	int num2 = P + 1;
	// make an array of medians
	floats* array2 = make_floats(num2);
	floats_push(array2, 0);
	jj = 1;
	while (jj < num){
		floats_push(array2, array->data[jj]);
		jj = jj+3;
	}
	floats_push(array2, FLT_MAX);
	//free the array
	free_floats(array);
	return array2;
}

// function to sort subarray and insert it values to correct places
	void
sort_worker(int pnum, float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{    
	int ii;
	int jj;
	// create subarray
	floats* xs = make_floats(10);
	// add elements to the subarray
	float n1 = samps->data[pnum];
	float n2 = samps->data[pnum + 1];
	for (ii = 0; ii < size; ii++) {
		float n = data[ii];
		if (n >= n1 && n < n2) {
			floats_push(xs, n);
		}
	}
	// print the info about the subarray
	printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);

	sizes[pnum] = xs->size;
	// sort the the subarray
	qsort_floats(xs);
 	// wait after array of sizes is filled in and sorted
	barrier_wait(bb);
	int pl = 0;
	for (ii =0; ii < pnum; ii++){
		pl+=sizes[ii];
	}
	ii = pl;
	jj = 0;
	// insert items back to data
	while (ii < pl + xs->size ){
		data[ii] = xs->data[jj];
		ii++;
		jj++;
	}
	// free subarray
	free_floats(xs);
}


// spawn P children to make parallel sorting
	void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{  
	int ii;
	// array of P kids
	pid_t kids[P];   
	for (ii = 0; ii < P; ii++) {
		if ((kids[ii] = fork())) {
			//parent process
			//do nothing
		}
		else {
			//child process
			sort_worker(ii, data, size, P, samps, sizes, bb);
			exit(0);
		}
	}
	// wait all children to finish execution
	for (int ii = 0; ii < P; ii++) {
		int rv = waitpid(kids[ii], 0, 0);
		check_rv(rv);
	}
}

// sample sort 
	void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
{
	// create a sample (array of medians) 
	floats* samps = sample(data, size, P);
	// run sort function on a given sample
	run_sort_workers(data, size, P, samps, sizes, bb);
	// free samples array
	free_floats(samps);
}

	int
main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("Usage:\n");
		printf("\t%s P data.dat\n", argv[0]);
		return 1;
	}

	const int P = atoi(argv[1]);
	const char* fname = argv[2];
	seed_rng();
	int rv;
	struct stat st;
	rv = stat(fname, &st);
	check_rv(rv);
	// size of the file should ot be less than 8
	const int fsize = st.st_size;
	if (fsize < 8) {
		printf("File too small.\n");
		return 1;
	}
	// open file
	int fd = open(fname, O_RDWR);
	check_rv(fd);
	// mmap the file
	void* file = mmap(0, fsize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	// count is first 8 bytes of the file
	long* count = file;
	// data starts after these 8 bytes
	float* data = file + 8;  
	// size of the sizes depends on the number of process
	long sizes_bytes = P * sizeof(long);
	// map array of sizes
	long* sizes = mmap(0, sizes_bytes, PROT_READ| PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	// create barrier
	barrier* bb = make_barrier(P);
	// run sample sort 
	sample_sort(data, *count, P, sizes, bb);
	// free barrier
	free_barrier(bb);
	// unmap array of sizes
	munmap(sizes, sizes_bytes);
	// unmap file
	munmap(file, fsize);
	// close file descriptor
	close(fd);
	return 0;
}

