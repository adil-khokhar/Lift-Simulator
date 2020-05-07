#ifndef F_H
#define F_H

void openFiles();
int* readNextValue();
void writeBuffer(buffer* buffer, int requestNo);
void writeLift(lifts* lift);
void writeResult(int movement, int requests);
void closeFiles();

#endif
