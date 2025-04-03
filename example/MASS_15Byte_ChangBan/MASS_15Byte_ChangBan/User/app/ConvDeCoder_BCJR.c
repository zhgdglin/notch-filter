#include "bsp.h"
//其中有些数学函数直接使用的math.h库里的函数，如果影响计算速度应该更换DSP库中的函数
#define LMAX  			1000.0f
#define LMAX2 			2000.0f
#define myINFINITY 		10000000000.0f
#define UNDERFLOW_LIMIT -1000.0f

//int aa=0,bb=0,cc=0,dd=0;

float Myaddplog(float lp1, float lp2)
{
	float d;
	d = lp1-lp2;
	if(d < 0)
	{
		d = -d;
		lp1 = lp2;
	}
	if(d > 2.772588f)
//	if(d > 7.0f)
	{
		return lp1;
	}
	else
	{
		return lp1-0.25f*d+0.693147f;
//		return lp1-0.002*powf((d-7),3);
	}
}


// add and store two log probabilities
void Myaddplogs(float* lp1, float lp2)
{
	*lp1 = Myaddplog(*lp1, lp2);
}

// MAX-Log-MAP
//void Myaddplogs(double* lp1, double lp2)
//{
//	if(*lp1<lp2)*lp1=lp2;
//}



//RowsOfData = DataLen
void MyConvDecode(int RowsOfData, float* llrin, char* data, float* llrout)
{
	int M, Lg, Lc, Ldt, Ld, Ls, rec, i, j, k, stmax, st2, dbit, * gen, * xlist, * cbits;
	float * alpha, * beta, * sigc0, * sigc1, gam0, gam1, h, sig0, sig1;

    int genr[2] = {115, 93};
	Lg = 2;
	Lc = RowsOfData;
	rec = 1;

	gen = mymalloc(SRAMIN, Lg*sizeof(int));//8byte
	
// 	llrin=mxGetPr(llrin_if);  //#define LMAX  	1000.0
	for (i = 0; i < Lc; i++)
   {	
	   if (llrin[i] > LMAX) 
		   llrin[i] = LMAX;
	   else if (llrin[i] < (-LMAX))
		   llrin[i] = -LMAX;
   }
	M = 0;    //constrain length determination  移位寄存器个数
   
	for (i = 0; i < Lg; i++)
	{
		gen[i] = (int)genr[i];
		gen[i] |= 1; //bitwise OX 0/1 ==>> 1/1
		j = gen[i];
		k = 0; while (j > 0) { j = j >> 1; k++; }
		if (k > M) M = k - 1;            // M=6
	}
   
	Ldt = Lc / Lg;  //interleaving size / # flows of coded bit (=2)  未编码前比特个数
	Ld = Ldt - M;   //M = # shift register     减去补零的比特个数
	Ls = 1 << M;    //Ls=4;                    Ls=64


	k = 1 << (M + 1); //M=2; k=8;    //K=128
	//xlist = (int*)calloc(k, sizeof(int));
	xlist = mymalloc(SRAMIN, k*sizeof(int));//0.5k
	
	for (i = 0; i < k; i++) { xlist[i] = 0; for (j = 0; j <= M; j++) if (i & (1 << j)) xlist[i] ^= 1; }//0-127的二进制表示中有奇数个1则xlist[i]=1反之为0
	//bitwise XOR operator ^  0/1 ==>> 1/0
		
		 //Ls=64  Ldt=256  Lg=2   
		alpha = mymalloc(SRAMDTCM, Ls * Ldt*sizeof(float));//64k
		beta = mymalloc(SRAMIN, Ls * Ldt*sizeof(float));//64k
		sigc0 = mymalloc(SRAMIN, Lg*sizeof(float));//
		sigc1 = mymalloc(SRAMIN, Lg*sizeof(float));//
		cbits = mymalloc(SRAMIN, Lg*sizeof(int));//
//		llrout = mymalloc(SRAMIN, Lc*sizeof(float));//
		
		
		
	//#define myINFINITY 	100000000000.0
	for (i = 0; i < (Ldt * Ls); i++) 
    { 
	alpha[i] = -myINFINITY; 
	beta[i] = -myINFINITY;
	}
	alpha[0] = 0.0;
	beta[Ldt - 1] = 0.0; // termination

	if (M > 0)
	{
		// obtain alphas
		stmax = (1 << M) - 1;
		for (i = 0; i < Ldt - 1; i++)
		{
			for (j = 0; j < Ls; j++)
			{
				st2 = j << 1; gam0 = 0.0; gam1 = 0.0;
				for (k = 0; k < Lg; k++)
				{
					h = (double)0.5 * llrin[i * Lg + k];
					if (xlist[gen[k] & st2] > 0) { gam0 += -h; gam1 += h; }
					else { gam0 += h; gam1 += -h; }
				}
				st2 &= stmax;
				// add log probabilities and store in lp1
				Myaddplogs(&alpha[i + 1 + st2 * Ldt], alpha[i + j * Ldt] + gam0);
				Myaddplogs(&alpha[i + 1 + (st2 + 1) * Ldt], alpha[i + j * Ldt] + gam1);
			}
			h = -myINFINITY;
			for (j = 0; j < Ls; j++) if (alpha[i + 1 + j * Ldt] > h) h = alpha[i + 1 + j * Ldt]; 
			for (j = 0; j < Ls; j++) alpha[i + 1 + j * Ldt] -= h;
		}

		
//		aa=0,bb=0,cc=0,dd=0;
		// obtain betas
		for (i = Ldt - 1; i >= 1; i--)
		{
			for (j = 0; j < Ls; j++)
			{
				st2 = j << 1; gam0 = 0.0; gam1 = 0.0;
				for (k = 0; k < Lg; k++)
				{
					h = (double)0.5 * llrin[i * Lg + k];
					if (xlist[gen[k] & st2] > 0) { gam0 += -h; gam1 += h; }
					else { gam0 += h; gam1 += -h; }
				}
				st2 &= stmax;
				Myaddplogs(&beta[i - 1 + j * Ldt], beta[i + st2 * Ldt] + gam0);
				Myaddplogs(&beta[i - 1 + j * Ldt], beta[i + (st2 + 1) * Ldt] + gam1);
			}
			h = -myINFINITY;
			for (j = 0; j < Ls; j++) if (beta[i - 1 + j * Ldt] > h) h = beta[i - 1 + j * Ldt];
			for (j = 0; j < Ls; j++) beta[i - 1 + j * Ldt] -= h;
		}

		
//		aa=0,bb=0,cc=0,dd=0;
		// obtain outputs
		for (i = 0; i < Ldt; i++)
		{
			sig0 = -myINFINITY; sig1 = -myINFINITY;
			for (j = 0; j < Lg; j++) { sigc0[j] = -myINFINITY; sigc1[j] = -myINFINITY; }
			for (j = 0; j < Ls; j++)
			{
				st2 = j << 1;
				if (rec) dbit = 1 ^ xlist[gen[0] & st2]; else dbit = 1;
				gam0 = 0.0; gam1 = 0.0;
				for (k = 0; k < Lg; k++)
				{
					cbits[k] = xlist[gen[k] & st2];
					h = (double)0.5 * llrin[i * Lg + k];
					if (cbits[k] > 0) { gam0 += -h; gam1 += h; }
					else { gam0 += h; gam1 += -h; }
				}
				st2 &= stmax;
				gam0 += alpha[i + j * Ldt] + beta[i + st2 * Ldt];
				gam1 += alpha[i + j * Ldt] + beta[i + (st2 + 1) * Ldt];
				if (dbit > 0) { Myaddplogs(&sig0, gam0); Myaddplogs(&sig1, gam1); }
				else { Myaddplogs(&sig1, gam0); Myaddplogs(&sig0, gam1); }
				for (k = 0; k < Lg; k++)
					if (cbits[k] > 0) { Myaddplogs(&sigc1[k], gam0); Myaddplogs(&sigc0[k], gam1); }
					else { Myaddplogs(&sigc0[k], gam0); Myaddplogs(&sigc1[k], gam1); }
			}
			if (i < Ld) data[i] = (sig0 > sig1) ? 0 : 1;
			for (k = 0; k < Lg; k++)
			{
				h = sigc0[k] - sigc1[k];
				if (h < (-LMAX2)) llrout[i * Lg + k] = -LMAX2; else if (h > LMAX2) llrout[i * Lg + k] = LMAX2; else llrout[i * Lg + k] = h;
				llrout[i * Lg + k] -= llrin[i * Lg + k];
			}
		}
	}
	else
	{
		// obtain outputs
		for (i = 0; i < Ldt; i++)
		{
			gam0 = 0.0; gam1 = 0.0;
			for (k = 0; k < Lg; k++)
			{
				h = (double)0.5 * llrin[i * Lg + k];
				gam0 += h; gam1 += -h;
			}
			if (i < Ld) data[i] = (gam0 > gam1) ? 0 : 1;
			for (k = 0; k < Lg; k++)
			{
				h = gam0 - gam1;          //#define LMAX2 	2000.0
				if (h < (-LMAX2)) llrout[i * Lg + k] = -LMAX2; else if (h > LMAX2) llrout[i * Lg + k] = LMAX2; else llrout[i * Lg + k] = h;
				llrout[i * Lg + k] -= llrin[i * Lg + k];
			}
		}
	}

	
	myfree(SRAMDTCM, alpha);
	myfree(SRAMIN, beta);
	myfree(SRAMIN, sigc0);
	myfree(SRAMIN, sigc1);
	myfree(SRAMIN, cbits);
	myfree(SRAMIN, xlist);
	myfree(SRAMIN, gen);
}

