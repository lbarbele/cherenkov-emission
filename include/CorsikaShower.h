#pragma once
#ifndef __CLASS__CorsikaShower__
#define __CLASS__CorsikaShower__ 1

#include <vector>
#include <cmath>

#include <CorsikaClasses.h>

class CorsikaShower
{
  friend class CorsikaFile;

private:

  CorsikaFile * filePtr;

  int iSubParticle;

  bool kGood;
  bool kDone;

  std::vector<float> vHeader;
  std::vector<float> vEnd;
  std::vector<float> vCurSub;

  CorsikaShower(CorsikaFile&, bool good = true);

public:

  std::vector<float> NextParticle();

  bool Done(){return this->kDone;}
  bool Good(){return this->kGood;}

  int Number(){return int(this->vHeader[1]);}
  int ID(){return this->Number();}
  float Theta(){return this->vHeader[10];}
  float Phi(){return this->vHeader[11];}
  float Energy(){return this->vHeader[3];}
  float Primary(){return this->vHeader[2];}
  float ObsLvl(int i = 0){return this->vHeader[47+i];}
  float LEmod(){return this->vHeader[74];}
  float HEmod(){return this->vHeader[75];}

  std::vector<float> GetHeader(){return this->vHeader;}
  std::vector<float> GetEnd(){return this->vEnd;}
  float GetHeader(int n){return this->vHeader[n];}
  float GetEnd(int n){return this->vEnd[n];}


};

#endif
