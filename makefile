CFLAGS = -Wall -pedantic -ansi -Werror

all : lift_sim_A lift_sim_B

lift_sim_A : lift_sim_A.o fileIO.o
	gcc lift_sim_A.o fileIO.o -lpthread -o lift_sim_A

lift_sim_B : lift_sim_B.o fileIO.o
	gcc lift_sim_B.o fileIO.o -lrt -lpthread -o lift_sim_B

lift_sim_A.o : lift_sim_A.c lift_sim_A.h structs.h fileIO.h
	gcc -c lift_sim_A.c $(CFLAGS)

lift_sim_B.o : lift_sim_B.c lift_sim_B.h structs.h fileIO.h
	gcc -c lift_sim_B.c -ggdb $(CFLAGS)

fileIO.o : fileIO.c fileIO.h structs.h
	gcc -c fileIO.c $(CFLAGS)

clean :
	rm -f lift_sim_A lift_sim_A.o lift_sim_B lift_sim_B.o fileIO.o
