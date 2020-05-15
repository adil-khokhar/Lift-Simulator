#ifndef F_H
#define F_H

void openFiles();
int* readNextValue();
void writeBuffer(int source, int destination, int requestNo);
void writeLift(lifts* lift);
void writeResult(int movement, int requests);
void closeFiles();

#endif
