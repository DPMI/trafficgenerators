
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "rndunid.h"

// Constructor *********************************************************
/*RNDUNID::RNDUNID(double minval, double maxval)
{
        MinValue = floor(minval + 0.5);
        MaxValue = floor(maxval + 0.5);
        DiffValue = MaxValue - MinValue + 1.0;
	Mean = 0.5 * (MaxValue + MinValue);
	Var = (DiffValue * DiffValue - 1.0) / 12.0;
}
*/
// output operator *****************************************************
ostream& operator<<(ostream& outp, RNDUNID& gen)
{
	outp << "RNDUNID {" << gen.MinValue << ".." << gen.MaxValue 
	     << "}, EX = " << gen.Mean << ", VarX = " << gen.Var;
	return outp;
}

// main program for a demonstration of generator output ****************
//#ifdef SINGLE
int main()
{  srand(time(NULL));
	double min, max;
	int no;
	cout << "min.? ";
	cin >> min;
	cout << "max.? ";
	cin >> max;
	cout << "no.?  ";
	cin >> no;
        RNDUNID RDU(min, max);
	RDU.printseed();
	cout << RDU << ":" << endl;
        for (int i = 0; i < no; i++)
	{
                cout << RDU.RndL();
		if (i % 10 == 9)
			cout << endl;
		else
			cout << "\t";
	}
 
        return 0;
}
//#endif

