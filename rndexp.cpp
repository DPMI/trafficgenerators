
#include <iostream>
#include "rndexp.h"
#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <time.h>

// output operator *****************************************************
ostream& operator<<(ostream& outp, RNDEXP& gen)
{
        outp << "RNDEXP, EX = " << gen.Mean << ", VarX = " << gen.Var;
        return outp;
}

// main program for a demonstration of generator output ****************

//#ifdef SINGLE
int main()
{ srand(time(NULL));
	double mean;
        double p;
	int no;
	cout << "mean? ";
	cin >> mean;
        cout << "no.?  ";
        cin >> no;
        RNDEXP RM(mean);

	cout << RM << endl;
	
	cout << endl;
        for (int i = 0; i < no; i++)
        {
                printf("%le", RM.Rnd());
                 
                cout <<setprecision(5)<<RM.Rnd();
                if (i % 5 == 4)
                        cout << endl;
                else
                        cout << "\t";
        }
     cout << "-> minimal random value: " << RM.GetMinRndValue() << endl;
	cout << "-> maximal random value: " << RM.GetMaxRndValue() << endl;
 
        return 0;
}
//#endif
