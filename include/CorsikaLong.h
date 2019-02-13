#pragma once
#ifndef __CLASS__CorsikaLong__
#define __CLASS__CorsikaLong__ 1

#include <fstream>
#include <string>
#include <vector>
#include <map>

class CorsikaLong
{
private:

  std::ifstream stream;

  std::vector<int> vID;

  std::map<int,std::map<int,std::map<int,std::vector<double>>>> mProf;
  std::map<int,std::vector<double>> mGH;

  std::vector<std::string> vColPart;
  std::vector<std::string> vColDep;

  int nShow;

  bool kGood;
  bool kSlant;

public:
  CorsikaLong(std::string);

  int GetID(int);

  int NShow(){return this->nShow;}

  bool Slant(){return this->kSlant;}
  bool Good(){return this->kGood;}

  double GetXmax(int);
  double GetXmaxByNumber(int n){return this->GetXmax(this->GetID(n));}

  std::vector<double> GetFit(int n);
  std::vector<double> GetFitByNumber(int n){return this->GetFit(this->GetID(n));}

  std::vector<double> GetProfile(int,int);
  std::vector<double> GetProfileByNumber(int n, int ipart){return this->GetProfile(this->GetID(n),ipart);}

  std::vector<double> GetDepositProfile(int,int);
  std::vector<double> GetDepositProfileByNumber(int n, int ipart){return this->GetDepositProfile(this->GetID(n),ipart);}

  std::string GetColumnName(int i, int j);

  void Print(int);
  void PrintByNumber(int n){this->Print(this->GetID(n));}


};


#endif
