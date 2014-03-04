
#include <iostream>
#include "rnddet.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// Constructors ********************************************************
/*RNDDET::RNDDET(double val)
{
        Value = val;                            // double:
        LValue = (long)(val + 1e-11);           // rounds off!!!
}
RNDDET::RNDDET(long lval)
{
        Value = (double)lval;                   // integer
        LValue = lval;                          // less critical
}
*/

// output operator *****************************************************
ostream& operator<<(ostream& outp, RNDDET& gen)
{
        outp << "RNDDET (" << gen.Value  
             << "), EX = " << gen.Value << ", VarX = " << 0.0;
	return outp;
}

// main program for a demonstration of generator output ****************
//#ifdef SINGLE
int main()
{       srand(time(NULL));
        double val;
	int no;
        cout << "value? ";
        cin >> val;
        cout << "no.?  ";
        cin >> no;
        RNDDET RD(val);

        cout << RD << ":" << endl;
        for (int i = 0; i < no; i++)
	{
                printf("%6.3lf", RD.Rnd());
                if (i % 10 == 9)
                        cout << endl;
                else
                        cout << "\t";
        }

        return 0;
}
//#endif
