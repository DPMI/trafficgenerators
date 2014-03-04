
#ifndef _RNDUNIF_INCL
#define _RNDUNIF_INCL

#include "rnd.h"

using namespace std;

class RNDUNIF: public RND
{
		// Members: ********************************************
	//private:
		double MinValue;	// lower bound
		double MaxValue;	// upper bound
		double DiffValue;	// difference of bounds
		double Mean;		// theoretical mean/expectation
		double Var;		// theoretical variance
	
		// Methods: ********************************************
	public:
		// Constructor *****************************************
		RNDUNIF(double minval, double maxval)
                   // RNDUNIF::RNDUNIF(double minval, double maxval)
{
        MinValue = minval;
        MaxValue = maxval;
        DiffValue = MaxValue - MinValue;
	Mean = 0.5 * (MinValue + MaxValue);
	Var = DiffValue * DiffValue / 12.0;
}
			
                // margins *********************************************
                double GetMinRndValue() 
                { return DiffValue * RND::GetMinRndValue() + MinValue; }
                double GetMaxRndValue() 
                { return DiffValue * RND::GetMaxRndValue() + MinValue; }

		// public generators **********************************
		virtual inline double Rnd() 
		{ return DiffValue * Rnd01() + MinValue; }

		

                // friend: output operator *****************************
		friend ostream& operator<<(ostream& outp, RNDUNIF& gen);
};

#endif
