
#include <iostream>
#include "rndunif.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

// Constructor *********************************************************
/*RNDUNIF::RNDUNIF(double minval, double maxval)
{
        MinValue = minval;
        MaxValue = maxval;
        DiffValue = MaxValue - MinValue;
	Mean = 0.5 * (MinValue + MaxValue);
	Var = DiffValue * DiffValue / 12.0;
}
*/
// output operator *****************************************************
ostream& operator<<(ostream& outp, RNDUNIF& gen)
{
	outp << "RNDUNIF ]" << gen.MinValue << ", " << gen.MaxValue 
	     << "[, EX = " << gen.Mean << ", VarX = " << gen.Var;
	return outp;
}

// main program for a demonstration of generator output ****************
//#ifdef SINGLE
int main()
{   srand(time(NULL));
	double min, max;
	int no;
	cout << "min.? ";
	cin >> min;
	cout << "max.? ";
	cin >> max;
        cout << "no.?  ";
        cin >> no;
        RNDUNIF RU(min, max);

	cout << RU << ":" << endl;
        for (int i = 0; i < no; i++)
	{
                printf("%6.3lf", RU.Rnd());
                if (i % 10 == 9)
                        cout << endl;
                else
                        cout << "\t";
        }

        return 0;
}
//#endif
