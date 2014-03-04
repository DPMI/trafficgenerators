
#ifndef _RNDDET_INCL
#define _RNDDET_INCL

#include "rnd.h"
using namespace std;

class RNDDET: public RND        // just because of compatibility
{
		// Members: ********************************************
	private:
                double Value;   // fixed value

                long LValue;
	
		// Methods: ********************************************
	public:
		// Constructor *****************************************
                RNDDET(double val)     // for doubles
{
        Value = val;                            // double:
        LValue = (long)(val + 1e-11);           // rounds off!!!
}
                RNDDET(int lval)     // for longs
{
       // Value = (double)lval;
          Value = lval;                   // integer
        LValue = lval;                          // less critical
}
			
                // margins *********************************************
                double GetMinRndValue() 
                { return Value; }
                double GetMaxRndValue() 
                { return Value; }

		// public generators **********************************
		virtual inline double Rnd() { return Value; }
                virtual inline long RndL() { return LValue; }
		

                // friend: output operator *****************************
                friend ostream& operator<<(ostream& outp, RNDDET& gen);
};

#endif
