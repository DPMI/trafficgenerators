
#ifndef _RNDUNID_INCL
#define _RNDUNID_INCL

#include "rnd.h"
using namespace std;

class RNDUNID: public RND
{
		// Members: ********************************************
	private:
		double MinValue;	// smallest value
		double MaxValue;	// largest value
		double DiffValue;	// difference + 1
		double Mean;		// theoretical mean/expectation
		double Var;		// theoretical variance
	
		// Methods: ********************************************
	public:
		// Constructor *****************************************
		RNDUNID(double minval, double maxval)
{
        MinValue = floor(minval + 0.5);
        MaxValue = floor(maxval + 0.5);
        DiffValue = MaxValue - MinValue + 1.0;
	Mean = 0.5 * (MaxValue + MinValue);
	Var = (DiffValue * DiffValue - 1.0) / 12.0;
}
			
		// margins *********************************************
		double GetMinRndValue() 
		{ return floor(DiffValue * RND::GetMinRndValue() + 1e-11) 
	                 + MinValue; }
		double GetMaxRndValue() 
		{ return floor(DiffValue * RND::GetMaxRndValue() + 1e-11) 
	                 + MinValue; }

		// public generators ***********************************
		virtual inline double Rnd() 
		{ return floor(DiffValue * Rnd01() + 1e-11) + MinValue; }
		virtual inline long RndL()
		{ return (long)(DiffValue * Rnd01() + MinValue + 1e-11); }
//*********************printseed..
		
		// friend: output operator *****************************
		friend ostream& operator<<(ostream& outp, RNDUNID& gen);
};

#endif
