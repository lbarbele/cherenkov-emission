#pragma once
#ifndef __CLASS__CorsikaAtmosphere__
#define __CLASS__CorsikaAtmosphere__ 1

#include <string>
#include <vector>

#include <CorsikaClasses.h>

class CorsikaAtmosphere
{
private:

  std::vector<double> a, b, c, h, d;

  void Initialize(CorsikaFile &);

public:

  CorsikaAtmosphere(CorsikaFile &);
  CorsikaAtmosphere(std::string);

  void Print();

  double Depth(double);
  double Height(double);
  double Density_vs_height(double);
  double Density_vs_depth(double);


};

#endif
