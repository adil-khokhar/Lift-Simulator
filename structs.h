#ifndef S_H
#define S_H

typedef struct {
    int source;
    int destination;
} buffer;

typedef struct {
    int source;
    int destination;
    int prevRequest;
    int movement;
    int totalMovement;
    int totalRequests;
    int finished;
    char name[6];
} lifts;

#endif
