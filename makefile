CFLAGS = -Wall -pedantic -ansi -Werror

all : lift_sim_A

lift_sim_A : lift_sim_A.o fileIO.o
	gcc lift_sim_A.o fileIO.o -lpthread -o lift_sim_A

lift_sim_A.o : lift_sim_A.c lift_sim_A.h structs.h fileIO.h
	gcc -c lift_sim_A.c $(CFLAGS)

fileIO.o : fileIO.c fileIO.h structs.h
	gcc -c fileIO.c $(CFLAGS)

clean :
	rm -f lift_sim_A lift_sim_A.o fileIO.o
