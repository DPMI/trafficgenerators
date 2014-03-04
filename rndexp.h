
#ifndef _RNDEXP_INCL
#define _RNDEXP_INCL


using namespace std;

#include "rnd.h"

class RNDEXP: public RND
{
  
                // Members: ********************************************
	private:
		double Mean;		// theoretical mean
		double Var;		// theoretical variance
		double Factor;		// dummy
	
                 // Methods: ********************************************
	public:
                // Constructor *****************************************
		RNDEXP(double mean) 
		{ Mean = mean; Var = mean * mean; Factor = -mean; }
			
                // margins *********************************************
                double GetMinRndValue() 
                { return log(RND::GetMaxRndValue()) * Factor; }
                double GetMaxRndValue() 
                { return log(RND::GetMinRndValue()) * Factor; }

		// public generators ***********************************
		double Rnd() 
		{ return log(1.0-Rnd01()) * Factor; }
	
		
 
                // friend: output operator *****************************
		friend ostream& operator<<(ostream& outp, RNDEXP& gen);
};

#endif
