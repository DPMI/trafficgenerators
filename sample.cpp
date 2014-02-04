// sample.cc
// Patrik Carlsson,ets97pca@student.hk-r.se
// TTS402 Simulering
// # Time-stamp: <2004-09-17 14:35:12 pca>
//
// Version Datum Sig
//------------------------------
// 1.0     980902  Pca
// 1.1     980909  Pca
//

#include <iostream>
#include "sample.h"

using namespace std;


SAMPLE::SAMPLE()
{
  Variance=0;
  Mean=0;
  VariableSqrSum=0;
  VariableSum=0;
  nrElem=0;
  max=0;
  min=1999999999;
}

SAMPLE::~SAMPLE()
{
  //Exit
}

void SAMPLE::PutSample(double value)
{
  nrElem++; 
  VariableSum+=value; 
  VariableSqrSum+=value*value;
  if(value>max){
    max=value;
  }
  if(value<min){
    min=value;
  }
#ifdef REPLICATION_LOGG
  cout << "SAMPLE: " << nrElem << " Value: " << value << endl;
#endif
}
// INLINE declared in sample.hh

//Calculate the MEAN value.
double SAMPLE::GetSampleMean(void)
{
  //  cout << "Variable sum = " << VariableSum << "nrElem = " << nrElem << endl;
  Mean=VariableSum/nrElem;
  return Mean;
}

//Calculate the VARIANCE value.
double SAMPLE::GetSampleVar(void)
{
  Mean=VariableSum/nrElem;
  if(nrElem==(double)(1))
    return(-1);

  Variance=(VariableSqrSum-2*Mean*VariableSum+Mean*Mean*nrElem);
  Variance=Variance/(nrElem-1);
  return Variance;
}

//Reset counters and similar stuff.
void SAMPLE::Reset(void)
{
#ifdef REPLICATION_LOGG
  cout << "!!!!!!!!!!RESETS SAMPLE TOOL!!!!!!!!!!" << endl;
#endif
  nrElem=0;
  Mean=0;
  Variance=0;
  VariableSqrSum=0;
  VariableSum=0;
  return;
}

double SAMPLE::GetSampleMax(void)
{
  return max;
}

double SAMPLE::GetSampleMin(void)
{
  return min;
}







