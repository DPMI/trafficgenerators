// sample.hh
// Patrik Carlsson,ets97pca@student.hk-r.se
// TTS402 Simulering
// # Time-stamp: <98/10/14 15:56:36 ets97pca>
// 
// Version Datum Sig
//------------------------------
// 1.0     980902  Pca
// 1.1     980909  Pca
//

#ifndef _SAMPLE_HH
#define _SAMPLE_HH


class SAMPLE
{
private:
  double Variance;
  double Mean;
  
  double VariableSqrSum;
  double VariableSum;
  
  double nrElem;

  double min;
  double max;

public:
  SAMPLE();
  ~SAMPLE();

  void PutSample(double value);
  double GetSampleMean(void);
  double GetSampleVar(void);
  double GetSampleMax(void);
  double GetSampleMin(void);
  void Reset(void);
};

#endif







