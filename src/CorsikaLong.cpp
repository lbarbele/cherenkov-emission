#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <CorsikaLong.h>

CorsikaLong::CorsikaLong(std::string s)
: stream(s)
, nShow(0)
, kGood(true)
, kSlant(false)
, vColPart({"depth","gammas","positrons","electrons","mu_p","mu_m","hadrons","charged","nuclei","cherenkov"})
, vColDep({"depth","gamma","em_ioniz","em_cut","mu_ioniz","mu_cut","hadron_ioniz","hadron_cut","netrino","sum"})
{
  //
  // Check if file is open
  //
  if (!this->stream.is_open())
  {
    std::cerr << "Could not open longitudinal file " << s << "." << std::endl;
    this->kGood = false;
    return;
  }



  //
  // An useful lambda
  //
  auto discard = [this](int n){std::string u; for(int i=0; i<n; i++) this->stream >> u;};



  //
  // Check if it is a longitudinal corsika file
  //
  std::string buf;
  this->stream >> buf;
  if (buf != "LONGITUDINAL")
  {
    std::cerr << "The file " << s << " is not a valid CORSIKA long file." << std::endl;
    this->kGood = false;
    return;
  }



  // Check if profiles are given in vertical steps
  discard(3);
  this->stream >> buf;
  if (buf != "VERTICAL") this->kSlant = true;
  this->stream.seekg(0);



  //
  // Parse file
  //
  // while(std::getline(this->stream,buf))
  while (this->stream >> buf)
  {
    // Check if we have a new shower
    if (buf != "LONGITUDINAL") continue;

    int iEvent;
    int iSteps;

    // Get number of steps
    discard(2);
    this->stream >> iSteps;

    // Get event number
    discard(7);
    this->stream >> iEvent;

    // Discard column names
    discard(10);

    // Alocate profiles
    for (int itype = 0; itype < 2; itype++)
      for (int idepth = 0; idepth < iSteps; idepth++)
        for (int ipart = 0; ipart < 10; ipart++)
          this->mProf[iEvent][itype][ipart] = std::vector<double>(iSteps);

    // Get profiles
    for (int itype = 0; itype < 2; itype++)
    {
      for (int idepth = 0; idepth < iSteps; idepth++)
        for (int ipart = 0; ipart < 10; ipart++)
          this->stream >> this->mProf[iEvent][itype][ipart][idepth];

      // Discard the following couple of lines before the energy deposit profiles
      if (itype == 0) discard(29);
    }

    // Discard useless text before the GH fit
    discard(19);

    // Get the Gaisser-Hillas fit of charged particle profile
    this->mGH[iEvent] = std::vector<double>(8);
    for (int ipar = 0; ipar < 8; ipar++)
    {
      if (ipar == 6) discard(2);
      else if (ipar == 7) discard(5);
      this->stream >> this->mGH[iEvent][ipar];
    }

    // Save the event ID
    this->vID.push_back(iEvent);
  }

  // Get number of showers
  this->nShow = this->vID.size();

}



void CorsikaLong::Print(int n)
{
  if (std::find(this->vID.begin(),this->vID.end(),n) == this->vID.end())
  {
    std::cerr << "CorsikaLong::Print(): couldn't find profiles for the ID " << n << "." << std::endl;
    return;
  }

  std::cout << "Shower ID: " << n << std::endl << std::endl;


  int iSteps = this->mProf[n][0][0].size();

  for (int itype = 0; itype < 2; itype++)
  {
    if (itype == 0) std::cout << "Particle profiles" << std::endl;
    else std::cout << std::endl << "Energy deposit profiles" << std::endl;

    for (int idepth = 0; idepth < iSteps; idepth++)
    {
      for (int ipart = 0; ipart < 10; ipart++) std::cout << std::setw(15) << this->mProf[n][itype][ipart][idepth];
      std::cout << std::endl;
    }
  }

  std::cout << std::endl << "Gaisser-Hillas fit parameters" << std::endl;
  for (int ipar = 0; ipar < 8; ipar++) std::cout << std::setw(15) << this->mGH[n][ipar];
  std::cout << std::endl;
}



int CorsikaLong::GetID(int n)
{
  if (n < 0 || n >= this->vID.size())
  {
    std::cerr << "CorsikaLong::GetID(): no ID available for the " << n << "-th shower." << std::endl;
    return -1;
  }

  return this->vID[n];
}



double CorsikaLong::GetXmax(int n)
{
  if (std::find(this->vID.begin(),this->vID.end(),n) == this->vID.end())
  {
    std::cerr << "CorsikaLong::GetXmax(): no data available for shower with ID " << n << "." << std::endl;
    return -1;
  }

  return this->mGH[n][2];
}



std::vector<double> CorsikaLong::GetProfile(int n, int ipart)
{
  if (std::find(this->vID.begin(),this->vID.end(),n) == this->vID.end())
  {
    std::cerr << "CorsikaLong::GetProfile(): no data available for shower with ID " << n << "." << std::endl;
    return std::vector<double>(0);
  }

  if (ipart < 0 || ipart >= 10)
  {
    std::cerr << "CorsikaLong::GetProfile(): particle type should be between 0 and 9. Value given is " << ipart << "." << std::endl;
    return std::vector<double>(0);
  }

  return this->mProf[n][0][ipart];
}



std::vector<double> CorsikaLong::GetDepositProfile(int n, int ipart)
{
  if (std::find(this->vID.begin(),this->vID.end(),n) == this->vID.end())
  {
    std::cerr << "CorsikaLong::GetDepositProfile(): no data available for shower with ID " << n << "." << std::endl;
    return std::vector<double>(0);
  }

  if (ipart < 0 || ipart >= 10)
  {
    std::cerr << "CorsikaLong::GetDepositProfile(): particle type should be between 0 and 9. Value given is " << ipart << "." << std::endl;
    return std::vector<double>(0);
  }

  return this->mProf[n][1][ipart];
}



std::vector<double> CorsikaLong::GetFit(int n)
{
  if (std::find(this->vID.begin(),this->vID.end(),n) == this->vID.end())
  {
    std::cerr << "CorsikaLong::GetFit(): no data available for shower with ID " << n << "." << std::endl;
    return std::vector<double>(0);
  }

  return this->mGH[n];
}



std::string CorsikaLong::GetColumnName(int i, int j)
{
  if (j < 0 || j >= 10) return std::string("");

  if (i == 0) return this->vColPart[j];
  else if (i == 1) return this->vColDep[j];
  else return std::string("");
}
