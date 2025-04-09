#ifndef _CONVCODE_H
#define _CONVCODE_H

//#include "bsp.h"

void ConvEncode(char* data,  int RowOfData, char* code);
double addplog(double lp1, double lp2);
void addplogs(double* lp1, double lp2);
void ConvDecode(int RowsOfData, float* llrin, char* data, float* llrext);
void ConvDecode_SRAMIN(int RowsOfData, float* llrin, char* data, float* llrext);

void MyConvDecode(int RowsOfData, float* llrin, char* data, float* llrext);
void Myaddplogs(float* lp1, float lp2);
float Myaddplog(float lp1, float lp2);	
#endif

