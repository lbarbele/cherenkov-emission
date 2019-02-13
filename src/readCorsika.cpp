#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <valarray>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TNtupleD.h>
#include <TSystem.h>

#include <CorsikaFile.h>
#include <CorsikaShower.h>
#include <CorsikaLong.h>
#include <CorsikaAtmosphere.h>

int main(int argc, char ** argv)
{
  // Fixed parameters!!
  const double maxRadius = 200.;

  //
  // Input parameters
  //

  // Check number of parameters
  if (argc != 4 && argc != 5)
  {
    std::cerr << "Syntax error! Usage: ./readCorsika inputDir/ outputDir/ runNumber [maxShowers:optional]" << std::endl;
    return 1;
  }

  // Get parameters
  std::string sInpDir = argv[1];
  std::string sOutDir = argv[2];
  int runNumber = std::stoi(argv[3]);
  int maxShowers = 0;
  if (argc == 5) maxShowers = std::stoi(argv[4]);

  // Check if direcory names end with '/'
  if (sInpDir[sInpDir.size()-1] != '/') sInpDir += "/";
  if (sOutDir[sOutDir.size()-1] != '/') sOutDir += "/";

  // Build a run number string with 6 digits
  std::string sRunNumber = std::to_string(runNumber);
  while (sRunNumber.size() < 6) sRunNumber = "0" + sRunNumber;

  // Build strings with file names
  auto sInpFil = sInpDir + "CER" + sRunNumber;
  auto sInpLng = sInpDir + "DAT" + sRunNumber + ".long";
  auto sOutFil = sOutDir + "cherenkov_" + sRunNumber + ".root";

  // Creathe the output folder, if necessary
  gSystem->mkdir(sOutDir.c_str());



  //
  // Object declaration
  //

  // Corsika related stuff: the CERXXXXXX file, the .long file and the atmospheric profile object
  CorsikaFile       cfile(sInpFil);
  CorsikaLong       clong(sInpLng);
  CorsikaAtmosphere catm(cfile);

  // Check input files
  if (!cfile.Good())
  {
    std::cerr << "Some error happened when trying to open the file with cherenkov photons! Will exit." << std::endl;
    std::cerr << "File is: " << sInpFil << std::endl;
    return 1;
  }
  else if (!clong.Good())
  {
    std::cerr << "Some error happened when trying to open the file with longitudinal profiles! Will exit." << std::endl;
    std::cerr << "File is: " << sInpLng << std::endl;
    return 1;
  }

  // Output related stuff: the root file, the event tree and the average histograms
  TFile froot(sOutFil.c_str(),"recreate");
  froot.mkdir("Average");

  // Check output file
  if (froot.IsZombie())
  {
    std::cerr << "Could not open output root file! Will exit." << std::endl;
    std::cerr << "File is: " << sOutFil << std::endl;
    return 1;
  }

  // Header tree (tuple)
  TNtupleD theader("Header","Header","ID:Energy:Primary:Theta:Phi:ObsLvl:LEmod:HEmod:Fit0:Fit1:Fit2:Fit3:Fit4:Fit5:FitChi2ndof:FitDev");
  theader.Write();

  // Histograms with cherenkov emission information
  std::vector<TH1D> hThetaAverage(20,TH1D("","",1000*18,0.,10.*18.));
  std::vector<TH1D> hDistAverage(20,TH1D("","",1000,0.,1000.));

  // Histogram with average photons at ground
  TH2D hGroundAverage("","",2*maxRadius,-maxRadius,maxRadius,2*maxRadius,-maxRadius,maxRadius);

  // Histogram of average density vs. r
  TH1D hDensityAverage("","",maxRadius,0,maxRadius);
  TH1D hDensitySigma("","",maxRadius,0,maxRadius);

  // The average profiles
  std::vector<std::valarray<double>> vAvgProfPart(9,std::valarray<double>(0));
  std::vector<std::valarray<double>> vAvgProfDep (9,std::valarray<double>(0));
  std::vector<double> vDepthPart(0);
  std::vector<double> vDepthDep(0);

  // The shower counter
  int nShowers = 0;

  // The cherenkov bunch counter
  double nBunches = 0.;



  //
  // Initial message
  //
  std::cout << std::endl;
  std::cout << "\e[1mreadCorsika\e[0m: starting analysis of run " << sRunNumber << "." << std::endl;
  std::cout << std::endl;
  std::cout << "+ Cherenkov file " << sInpFil << ": " << (cfile.Good() ? "Ok" : "Fail") << std::endl;
  std::cout << "+ Longitudinal file " << sInpLng << ": " << (clong.Good() ? "Ok" : "Fail") << std::endl;
  std::cout << "+ Number of showers: " << cfile.NShow() << std::endl;
  std::cout << "+ Date of run start: " << cfile.StartDate()%100 << "/" << cfile.StartDate()%10000/100 << "/" << cfile.StartDate()/10000 << " (dd/mm/yy)" << std::endl;
  std::cout << "+ CORSIKA version:   " << cfile.Version() << std::endl;
  std::cout << std::endl;
  std::cout << "Starting loop over showers...";
  std::cout << std::setw(10) << "Energy";
  std::cout << std::setw(10) << "Theta";
  std::cout << std::setw(10) << "Phi";
  std::cout << std::setw(10) << "Xmax";
  std::cout << std::setw(10) << "ID";
  std::cout << std::endl;



  //
  // Loop over showers
  //
  while(!cfile.Done())
  {
    //
    // Get next shower and check
    //
    auto shower = cfile.NextShower();
    if (!shower.Good()) continue;

    // increment shower counter
    nShowers++;


    //
    // Get overview of the shower and add to the header tree
    //

    // Put Xmax of the current shower in a variable, since it is used later
    float xmax = clong.GetXmax(shower.ID());

    // Build the vector that will go to the header tree
    std::vector<double> vHeader;
    vHeader.push_back(shower.ID());
    vHeader.push_back(shower.Energy());
    vHeader.push_back(shower.Primary());
    vHeader.push_back(shower.Theta());
    vHeader.push_back(shower.Phi());
    vHeader.push_back(shower.ObsLvl());
    vHeader.push_back(shower.LEmod());
    vHeader.push_back(shower.HEmod());

    auto vFit = clong.GetFit(shower.ID());
    vHeader.insert(vHeader.end(),vFit.begin(),vFit.end());

    theader.Fill(vHeader.data());



    //
    // Initial shower message
    //
    std::cout << "+ Reading shower ";
    std::cout << std::setw(std::floor(std::log10(cfile.NShow()))+1) << nShowers;
    std::cout << "/";
    std::cout << std::setw(std::floor(std::log10(cfile.NShow()))+1) << cfile.NShow();
    std::cout << ":";
    std::cout << std::setw(10) << shower.Energy() << " GeV";
    std::cout << std::setw(10) << shower.Theta();
    std::cout << std::setw(10) << shower.Phi();
    std::cout << std::setw(10) << xmax;
    std::cout << std::setw(10) << shower.ID();
    std::cout << " ... ";
    std::cout << std::flush;



    //
    // Get profiles and write to output file
    //

    // depths for particle profiles
    auto vDepth = clong.GetProfile(shower.ID(),0);
    if (!clong.Slant())
      for (auto & x : vDepth)
        x = x/std::cos(shower.Theta());

    // save depths for average particle profiles
    if (vDepthPart.empty()) vDepthPart = vDepth;

    // create directories on the output file for particle profiles of this event
    froot.mkdir(("Event_" + std::to_string(shower.ID()) + "/ParticleProfiles").c_str());
    froot.cd(("Event_" + std::to_string(shower.ID()) + "/ParticleProfiles").c_str());

    // loop to build and save profiles
    for (int i=1; i<10; i++)
    {
      auto vProfile = clong.GetProfile(shower.ID(),i);
      TGraph gProfile(vDepth.size(),vDepth.data(),vProfile.data());
      gProfile.Write(clong.GetColumnName(0,i).c_str());

      // add profiles to average
      if (vAvgProfPart[i-1].size() == 0)
        vAvgProfPart[i-1]  = std::valarray<double>(vProfile.data(),vProfile.size());
      else
        vAvgProfPart[i-1] += std::valarray<double>(vProfile.data(),vProfile.size());
    }

    // depths for energy deposit profiles
    vDepth = clong.GetDepositProfile(shower.ID(),0);
    if (!clong.Slant())
      for (auto & x : vDepth)
        x = x/std::cos(shower.Theta());

    // save depths for average energy deposit profiles
    if (vDepthDep.empty()) vDepthDep = vDepth;

    // create directories on the output file for energy deposit profiles of this event
    froot.mkdir(("Event_" + std::to_string(shower.ID()) + "/DepositProfiles").c_str());
    froot.cd(("Event_" + std::to_string(shower.ID()) + "/DepositProfiles").c_str());

    // loop to build and save profiles
    for (int i=1; i<10; i++)
    {
      auto vProfile = clong.GetDepositProfile(shower.ID(),i);
      TGraph gProfile(vDepth.size(),vDepth.data(),vProfile.data());
      gProfile.Write(clong.GetColumnName(1,i).c_str());

      // add profiles to average
      if (vAvgProfDep[i-1].size() == 0)
        vAvgProfDep[i-1]  = std::valarray<double>(vProfile.data(),vProfile.size());
      else
        vAvgProfDep[i-1] += std::valarray<double>(vProfile.data(),vProfile.size());
    }

    //
    // Distributions of particles arriving at ground
    //

    // Create histograms for this shower
    std::vector<TH1D> hThetaShower(20,TH1D("","",1000,0.,10.));
    std::vector<TH1D> hDistShower(20,TH1D("","",1000,0.,1000.));
    TH2D hPhotonsAtGround("","",2*maxRadius/2,-maxRadius,maxRadius,2*maxRadius/2,-maxRadius,maxRadius);
    TH1D hPhotonDensity("","",maxRadius,0.,maxRadius);

    // Loop over particles
    while(!shower.Done())
    {
      // Convention:
      // cosu = sin(theta)*cos(phi)
      // cosv = sin(theta)*sin(phi)
      // distance in cm
      // time in nsec

      // Get the current particle
      auto vPart = shower.NextParticle();

      // Give friendly names to particle fields
      const float & bunch  = vPart[0];
      const float & posx   = vPart[1];
      const float & posy   = vPart[2];
      const float & cosu   = vPart[3];
      const float & cosv   = vPart[4];
      const float & nsec   = vPart[5];
      const float & height = vPart[6];

      // Project the emission height into the shower axis
      float cosTheta = std::sqrt(1. - cosu*cosu - cosv*cosv);
      float xem = posx - height*cosu/cosTheta;
      float yem = posy - height*cosv/cosTheta;
      float heightProj = std::cos(shower.Theta())*std::cos(shower.Theta())*(height - std::tan(shower.Theta()) * (xem*std::cos(shower.Phi()) + yem*std::sin(shower.Phi())));

      // Compute radial distance to core, emission depth, emission age and emission angle
      float posr = std::sqrt(posx*posx + posy*posy);
      float depth = catm.Depth(heightProj);
      float age = 3./(1.+2.*xmax/depth);
      float theta = std::acos(cosTheta)*180./std::acos(-1.);

      // Compute distance of emission point to shower, on the shower plane
      float delta = std::sin(shower.Theta())*(std::cos(shower.Phi())*xem + std::sin(shower.Phi())*yem) - std::cos(shower.Theta())*height;
      float dist = std::sqrt(xem*xem + yem*yem + height*height - delta*delta);

      // Fill histograms if shower is inside range
      if (age < 2. && posr*1.e-2 < maxRadius)
      {
        // Histograms with number of cherenkov photons vs. emission angle
        hThetaAverage[(int)std::floor(age*10.)].Fill(theta,bunch);
        hThetaShower[(int)std::floor(age*10.)].Fill(theta,bunch);

        // Histograms with number of cherenkov photons vs. perpendicular distance to axis
        hDistAverage[(int)std::floor(age*10.)].Fill(dist*1.e-2,bunch);
        hDistShower[(int)std::floor(age*10.)].Fill(dist*1.e-2,bunch);

        // 2D histogram with photons at ground
        hPhotonsAtGround.Fill(posx*1.e-2,posy*1.e-2,bunch);
        hGroundAverage.Fill(posx*1.e-2,posy*1.e-2,bunch);

        // Histogram of photon density vs. r
        hPhotonDensity.Fill(posr*1.e-2,bunch);
      }
    }

    // Write histograms of this shower to output file
    froot.mkdir(("Event_" + std::to_string(shower.ID()) + "/EmissionAngle").c_str());
    froot.cd(("Event_" + std::to_string(shower.ID()) + "/EmissionAngle").c_str());
    for (int i=0; i<20; i++) hThetaShower[i].Write(std::to_string(i).c_str());

    froot.mkdir(("Event_" + std::to_string(shower.ID()) + "/EmissionDist").c_str());
    froot.cd(("Event_" + std::to_string(shower.ID()) + "/EmissionDist").c_str());
    for (int i=0; i<20; i++) hDistShower[i].Write(std::to_string(i).c_str());

    froot.cd(("Event_" + std::to_string(shower.ID())).c_str());
    hPhotonsAtGround.Write("PhotonsAtGround");

    for (int i=1; i<=hPhotonDensity.GetNbinsX(); i++)
    {
      double xleft = hPhotonDensity.GetBinLowEdge(i);
      double xright = hPhotonDensity.GetBinLowEdge(i+1);
      hPhotonDensity.SetBinContent(i,hPhotonDensity.GetBinContent(i)/(std::acos(-1.)*(xright*xright-xleft*xleft)));
      hPhotonDensity.SetBinError(i,0);
    }

    froot.cd(("Event_" + std::to_string(shower.ID())).c_str());
    hPhotonDensity.Write("PhotonDensity");

    auto hPhotonDensitySquare = hPhotonDensity;
    hPhotonDensitySquare.Multiply(&hPhotonDensitySquare);

    hDensityAverage.Add(&hPhotonDensity);
    hDensitySigma.Add(&hPhotonDensitySquare);



    //
    // Final shower message
    //
    std::cout << "Done!" << std::endl;



    if (maxShowers > 0 && nShowers >= maxShowers) break;
  }

  //
  // Finish computation of average particle profiles and write them to the output file
  //
  froot.mkdir("Average/ParticleProfiles");
  froot.cd("Average/ParticleProfiles");
  for (int i=0; i<9; i++)
  {
    vAvgProfPart[i] /= double(nShowers);
    TGraph gProfile(vDepthPart.size(),vDepthPart.data(),&vAvgProfPart[i][0]);
    gProfile.Write(clong.GetColumnName(0,i+1).c_str());
  }

  froot.mkdir("Average/DepositProfiles");
  froot.cd("Average/DepositProfiles");
  for (int i=0; i<9; i++)
  {
    vAvgProfDep[i] /= double(nShowers);
    TGraph gProfile(vDepthDep.size(),vDepthDep.data(),&vAvgProfDep[i][0]);
    gProfile.Write(clong.GetColumnName(1,i+1).c_str());
  }

  //
  // Finish computations of histograms with averages and write to output file
  //
  for(int i=0; i<20; i++)
  {
    hThetaAverage[i].Scale(1./double(nShowers));
    hDistAverage[i].Scale(1./double(nShowers));
  }
  froot.mkdir("Average/EmissionAngle");
  froot.cd("Average/EmissionAngle");
  for(int i=0; i<20; i++) hThetaAverage[i].Write(std::to_string(i).c_str());
  froot.mkdir("Average/EmissionDist");
  froot.cd("Average/EmissionDist");
  for(int i=0; i<20; i++) hDistAverage[i].Write(std::to_string(i).c_str());

  froot.cd("Average");

  hGroundAverage.Scale(1./double(nShowers));
  hGroundAverage.Write("PhotonsAtGround");

  hDensityAverage.Scale(1./double(nShowers));
  hDensityAverage.Write("PhotonDensity");

  hDensitySigma.Scale(1./double(nShowers));
  auto hDensityAverageSquared = hDensityAverage;
  hDensityAverageSquared.Multiply(&hDensityAverageSquared);
  hDensitySigma.Add(&hDensityAverageSquared,-1.);
  for (int i=1; i<=hDensitySigma.GetNbinsX(); i++) hDensitySigma.SetBinContent(i,std::sqrt(hDensitySigma.GetBinContent(i)));
  hDensitySigma.Write("PhotonDensitySigma");

  froot.cd();
  theader.Write(theader.GetName(),TFile::kOverwrite);

  froot.Close();



  //
  // Final message
  //
  std::cout << std::endl;
  std::cout << "Done with run " << sRunNumber << "!" << std::endl;
  std::cout << "Root data was saved to " << sOutFil << " ." << std::endl;
  std::cout << std::endl;

  return 0;
}
