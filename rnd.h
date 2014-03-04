
#ifndef _RND_INCL
#define _RND_INCL

class RND
{
		// Members: ******************************************** 
	protected:
		static double RndValue;		// last "random" value
		static double SeedValue;	// gives first "random" value
		static const double Multiplic;	// constant for generator
		static const double MaxLongD;	// dto.
		static const long MaxLong;	// dto.

		// Methods: ********************************************

		// internal ]0, 1[ uniform generator *******************
		double Rnd01()
		{
		 	
			double TmpValue 
			       = (double)(long)(RndValue * MaxLongD + 0.5) 
				 * Multiplic / MaxLongD;
			RndValue = TmpValue - (double)(long)TmpValue;
			return RndValue;
		}

	public:	
		// Con-/Destructor *************************************
		RND() {}
		virtual ~RND() {}

		// Seed ************************************************
		void SetSeedValue(long value);
		long GetSeedValue() { return (long)(RndValue * MaxLongD + 0.5); }
		
		// margins *********************************************
		double GetMinRndValue() { return 1.0 / MaxLongD; }
		double GetMaxRndValue() { return 1.0 - 1.0 / MaxLongD; }

		// public generators ***********************************
		virtual inline double Rnd() { return Rnd01(); }
		virtual inline long RndL() { return -1; }
//*********************printseed..
		void printseed(){ printf("Seed: %f\n",SeedValue); }
		
		// default if generator makes no sense!
};

#endif
