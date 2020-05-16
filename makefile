CFLAGS = -Wall -pedantic -ansi -Werror

all : lift_sim_A lift_sim_B gen_sim_input

lift_sim_A : lift_sim_A.o fileIO.o
	gcc lift_sim_A.o fileIO.o -lpthread -o lift_sim_A

lift_sim_B : lift_sim_B.o fileIO.o
	gcc lift_sim_B.o fileIO.o -lrt -lpthread -pthread -o lift_sim_B

gen_sim_input : gen_sim_input.o
	gcc gem_sem_input.o -o gen_sim_input

lift_sim_A.o : lift_sim_A.c lift_sim_A.h structs.h fileIO.h
	gcc -c lift_sim_A.c $(CFLAGS)

lift_sim_B.o : lift_sim_B.c lift_sim_B.h structs.h fileIO.h
	gcc -c lift_sim_B.c -ggdb $(CFLAGS)

fileIO.o : fileIO.c fileIO.h structs.h
	gcc -c fileIO.c $(CFLAGS)

gen_sim_input.o : gen_sim_input.c
	gcc -c gen_sim_input.c $(CFLAGS)

clean :
	rm -f lift_sim_A lift_sim_A.o lift_sim_B lift_sim_B.o fileIO.o gen_sim_input gen_sem_input.o
