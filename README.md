# Lift Simulator

## Introduction
This application simulates three lifts serving requests from an input file. There are two different executables, lift_sim_A and lift_sim_B. The first executable uses POSIX threads to run this simulation while the second uses Processes, POSIX Semaphores and POSIX shared memory.

There are 20 floors that can be served and there can be anywhere from 50 to 100 requests. The requests themselves are placed into a buffer (size is user defined) during the simulation and there is user-defined delay between each lift request (to simulate the time it takes for a lift to complete a request).

## Compiling Program
This program comes with a Makefile which simplifies compiling each file. To compile all the files, run the command:
```sh
$ make
```
To remove all the executable and .o files, run:
```sh
$ make clean
```

## Using Program
### Input File
To initially start the program, an input file must be generated which contains 50-100 lift requests in the correct format. To simplify things, an executable called 'gen_sim_input' is compiled along with the rest of the executables. This program generates a random input file in the correct format.

To run this program, run the command:
```sh
$ ./gen_sim_input
```
The console should print a line like this:
```sh
Num requests = 50
```
50 will be replaced by the number of requests generated in the input file (sim_input).
### Running Simulation Using Threads
To simulate the lifts using threads, the executable called 'lift_sim_A' must be used. To run the program, use the command:
```sh
$ ./lift_sim_A ${buffer_size} ${sleep_time}
```
Replace `${buffer_size}` with the size of the buffer that stores the lift requests during simulation (any integer more than or equal to 1). Also, replace `${sleep_time}` with the time that the lifts will take to serve a request (any integer more than or equal to 0).
### Running Simulation Using Processes
To simulate the lifts using processes, the executable called 'lift_sim_B' must be used. To run the program, use the command:
```sh
$ ./lift_sim_B ${buffer_size} ${sleep_time}
```
Replace `${buffer_size}` with the size of the buffer that stores the lift requests during simulation (any integer more than or equal to 1). Also, replace `${sleep_time}` with the time that the lifts will take to serve a request (any integer more than or equal to 0).
### Output File
All of the actions of the lift simulator for both executables are stored in a file called 'sim_output'.

The structure of the 'sim_output' are as follows (Numbers replaced as necessary):
* Lift Request added to buffer
```sh
--------------------------------------------
New Lift Request From Floor 10 to Floor 2
Request No: 21
--------------------------------------------
```
* Lift serves request from buffer
```sh
Lift-1 Operation
Previous position: Floor 8
Request: Floor 2 to Floor 9
Detail operations:
 Go from Floor 8 to Floor 2
 Go from Floor 2 to Floor 9
#movement for this request: 13
#request: 5
Total #movement: 70
Current position: Floor 9
```
* Result when simulator ends
```sh
--------------------------------------------
Total number of requests: 100
Total number of movements: 5520
--------------------------------------------
```
