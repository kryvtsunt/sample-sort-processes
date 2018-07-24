Concept: Sample Sort
Sample sort is a parallel version of quicksort.

It works as follows:

Input:
- An array of N items
- An integer P for number of processes to use.

Result:
- The input array has been sorted.

Sample
Randomly select 3*(P-1) items from the array.
Sort those items.
Take the median of each group of three in the sorted array, producing an array (samples) of (P-1) items.
Add 0 at the start and +inf at the end (or the min and max values of the type being sorted) of the samples array so it has (P+1) items numbered (0 to P).
Partition
Spawn P processes, numbered p in (0 to P-1).
Each process builds a local array of items to be sorted by scanning the full input and taking items between samples[p] and samples[p+1].
Write the number of items (n) taken to a shared array sizes at slot p.
Sort locally
Each process uses quicksort to sort the local array.
Copy local arrays to input.
Each process calculates where to put its result array as follows:
- start = sum(sizes[0 to p - 1]) # that's sum(zero items) = 0 for p = 0
- end = sum(sizes[0 to p]) # that's sum(1 item) for p = 0
Warning: Data race if you don't synchronize here.
Each process copies its sorted array to input[start..end]
Cleanup
Terminate the P subprocesses. Array has been sorted "in place".
Homework Tasks
Task 1: Write a Program
Your program, ssort, should take two arguments:
- An integer specifying the number of processes to sort with
- The file to sort
The input file is structured as follows:
- First 8 bytes: long size - number of floats in the file
- Next 4*size bytes: An array of doubles
Use mmap for I/O on the file to be sorted.
Use mmap to share data between processes.
Use sample sort to sort the input file, in place.
In addition, each subprocess should print out some text, showing that it's actually participating in sample sort:

 {p}: start {start}, count {length of local array}  
Example session:

$ ./tools/gen-input 20 data.dat
$ ./ssort 4 data.dat
0: start 0.0000, count 3
1: start 7.5690, count 5
2: start 27.1280, count 11
3: start 95.5110, count 1
$ ./tools/check-sorted data.dat
$
Task 2: Measure Parallel Speedup
Test your sample sort program in three scenarios, each with a newly generated input file of 50 million floats (or enough to take over 10 seconds for 1 process on your machine):

1 process
4 processes
8 processes
Time these tests using the "/usr/bin/time -p" command. (You're interested in "real" time)

Repeat each test three times, taking the median.

Make a graph (graph.png) showing the measured time for each case.

Task 3: report.txt
Create a file, report.txt, containing the following information:

The operating system, processor model, number of processor cores, and amount of RAM for your test machine.
A table of measured time and parallel speedup for each test (use ASCII art).
Parallel speedup is calculated as (time for 1 process / time for N processes)
Optimal parallel speedup is N.
A paragraph or two discussing what results you got and why you got them.
Look up "Amdahl's Law" and "Gustafson's Law" on Wikipedia. Is sample sort a good parallel sorting algorithm?
