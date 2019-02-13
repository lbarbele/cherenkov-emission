#pragma once
#ifndef __CLASS__CorsikaFile__
#define __CLASS__CorsikaFile__ 1

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <CorsikaClasses.h>

class CorsikaFile
{
  friend class CorsikaShower;

private:
  std::ifstream stream;

  int nBlockSize;
  int nWordSize;
  int nSubBlocks;
  int nSubWords;
  int nWordsPerParticle;
  int nParticlesPerBlock;

  int iCurSub;

  bool kThin;
  bool kSkip;
  bool kGood;
  bool kDone;

  std::vector<float> vHeader;
  std::vector<float> vEnd;

  std::string sFileName;

  std::vector<float> NextSubBlock();
  void RewindSubBlock();
  void Reset();

public:
  CorsikaFile(std::string);
  ~CorsikaFile();

  CorsikaShower NextShower();

  int NShow(){return this->vHeader[92];}

  bool Good(){return this->kGood;}
  bool Done(){return this->kDone;}

  void DumpRUNE(){for (int i=0; i<this->vEnd.size(); i++) std::cout << this->vEnd[i] << std::endl;};
  void DumpRUNH(){for (int i=0; i<this->vHeader.size(); i++) std::cout << this->vHeader[i] << std::endl;};

  std::vector<float> GetHeader(){return this->vHeader;}
  std::vector<float> GetEnd(){return this->vEnd;}
  float GetHeader(int n){return this->vHeader[n];}
  float GetEnd(int n){return this->vEnd[n];}

};

#endif
