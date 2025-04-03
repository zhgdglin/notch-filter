#include "bsp.h"
//其中有些数学函数直接使用的math.h库里的函数，如果影响计算速度应该更换DSP库中的函数
//c=conv_encoder(b,gen,REC);

//RowsOfData = DataLen



//data:输入InfoSequence
//RowOfData：InfoSequence长度
//code：输出卷积码
void ConvEncode(char* data,  int RowOfData, char* code)
{
	int M, Lg, Ld, Ldt, rec, i, j, k, st, stmax, * gen, * xlist; 
	//int* code;
    int genr[2] = {115, 93};
    int REC = 1;
	int ColumnOfGenr = 2;
		
	Lg = ColumnOfGenr;
	Ld = RowOfData;
	rec = REC > 0;

	gen = mymalloc(SRAMEX, Lg*sizeof(int));
	
	M = 0; //memory length determination
	for (i = 0; i < Lg; i++) //Lg=2;
	{
		gen[i] = (int)genr[i];
		gen[i] |= 1; //gen[i]=gen[i] (bitwise) or (1)
		j = gen[i];
		k = 0; while (j > 0) { j = j >> 1; k++; }
		if (k > M) M = k - 1; //memory length
	}
	Ldt = Ld + M; // data length including termination
	
	k = 1 << (M + 1); //k=8;
	//xlist = (int*)calloc(k, sizeof(int));        //bitwise AND
    xlist = mymalloc(SRAMEX, k*sizeof(int));
	
	for (i = 0; i < k; i++) { xlist[i] = 0; for (j = 0; j <= M; j++) if (i & (1 << j)) xlist[i] ^= 1; }
	//bitwise XOR operator ^  0/1 ==>> 1/0  
// encode data bits
	st = 0;
	stmax = (1 << (M + 1)) - 1; //8-1
	if (rec) for (i = 0; i < Ld; i++) //recursive case
	{
		code[i * Lg] = data[i]; //systematic bit sequence 0, 2, 4, 6, ...
		k = xlist[gen[0] & (st << 1)];
		k ^= (int)data[i]; //bitwise XOR
		st = ((st << 1) + k) & stmax; //bitwise AND
		for (j = 1; j < Lg; j++) code[i * Lg + j] = xlist[gen[j] & st]; //coded sequence 1, 3, 5, 7, ...
	}
	else for (i = 0; i < Ld; i++) //none-recursive case
	{
		st = ((st << 1) + ((int)data[i])) & stmax;
		for (j = 0; j < Lg; j++) code[i * Lg + j] = xlist[gen[j] & st];
	}

	// encode termination bits
	if (rec) for (i = Ld; i < Ldt; i++)
	{
		code[i * Lg] = xlist[gen[0] & (st << 1)];
		st = (st << 1) & stmax;
		for (j = 1; j < Lg; j++) code[i * Lg + j] = xlist[gen[j] & st];
	}
	else for (i = Ld; i < Ldt; i++)
	{
		st = (st << 1) & stmax;
		for (j = 0; j < Lg; j++) code[i * Lg + j] = xlist[gen[j] & st];
	}

//	free(xlist);
//	free(gen);
	myfree(SRAMEX, xlist);
	myfree(SRAMEX, gen);
	
}
//
//
//测试函数
//int main()
//{
//	int i;
//	char Data[10] =  { 0, 1, 0, 1, 0, 1, 1, 0, 0, 1 };
//	int gen[2] = { 115,93 };
//	int REC = 1;
//	char* ConvCode;
//	ConvCode = (char*)calloc(2*(10+6), sizeof(char));
//	ConvEncode(&Data[0], &gen[0], REC, 2, 10, ConvCode);
//	for (i = 0; i < 32; i++)
//	{
//		printf("ConvCode[%d]=%d\n",i+1, ConvCode[i]);

//	}
//	return 0;
//	free(ConvCode);
//}
