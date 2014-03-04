// rnd.cc
//*************************
// basic random number generator
// V 1.1
// 981111; 001028
//*************************
// (c) Markus Fiedler, ITM

// #include <values.h>
#define MAXLONG 2147483647
#include <time.h>//added
#include <stdlib.h>//added
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "rnd.h"

using namespace std;

// initialization of static members ************************************
//double RND::SeedValue = 123457.0;
//srand(time(NULL));
int seed = rand() % time(NULL);

  //ofstream myfile;
 // myfile.open ("example.txt");
 // myfile << "Writing this to a file.\n";
 // myfile.close();
//FILE * pFile;
  //pFile = fopen ("RANOMSEED.txt","w");
  
  
    //fprintf(pFile,"Seed: %d\n",seed);
    //fclose (pFile);
  
double RND::SeedValue = (double) seed;	
double RND::RndValue = (double)RND::SeedValue / MAXLONG;
const double RND::Multiplic = 16807.0;
const double RND::MaxLongD = (double)MAXLONG;
const long RND::MaxLong = MAXLONG;

// Seed ****************************************************************
void RND::SetSeedValue(long value)
{

 
  
	if ((value <= 0) || (value == MaxLong))
		RndValue = 123457.0 / MaxLongD;
	else
		RndValue = value / MaxLongD;
}
