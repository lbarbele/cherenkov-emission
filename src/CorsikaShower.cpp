#include <iostream>
#include <iomanip>

#include <CorsikaFile.h>
#include <CorsikaShower.h>

CorsikaShower::CorsikaShower(CorsikaFile & cFile, bool good)
: filePtr(&cFile)
, iSubParticle(0)
, kGood(good)
, kDone(false)
, vCurSub(0)
, vHeader(cFile.nSubWords,-1.)
{
  //
  // Check if shower is expected to be ok
  //
  if (!good) return;



  //
  // Check file pointer
  //
  if (!this->filePtr)
  {
    std::cerr << "Could not retrieve a pointer to the current CORSIKA file to read the shower." << std::endl;
    this->kGood = false;
    return;
  }



  //
  // Get event header and check
  //
  this->vHeader = this->filePtr->NextSubBlock();

  if (this->vHeader.empty() || std::string((char*)vHeader.data(),4) != "EVTH")
  {
    std::cerr << "Could not read shower from file " << this->filePtr->sFileName << "." << std::endl;
    this->kGood = false;
    return;
  }

  //
  // Get first particle data block
  //
  this->vCurSub = this->filePtr->NextSubBlock();

  return;
}




std::vector<float> CorsikaShower::NextParticle()
{
  // Start and end position of the current particle within the current subbloc
  int init = this->iSubParticle*this->filePtr->nWordsPerParticle;
  int iend = init + this->filePtr->nWordsPerParticle;

  // Increase counter of particles within the current subblock
  this->iSubParticle++;

  // Check if we are done with the current particle subblock or simply return current particle
  if (this->iSubParticle == this->filePtr->nParticlesPerBlock)
  {
    // This is the last particle, reset counter
    this->iSubParticle = 0;

    // Store current particle
    auto v = std::vector<float>(this->vCurSub.data() + init, this->vCurSub.data() + iend);

    // Read next subblock
    this->vCurSub = this->filePtr->NextSubBlock();

    if (this->vCurSub.empty())
    {
      std::cerr << "After loop over particles for shower number " << this->Number() << ", could not read the next data sub-block!" << std::endl;
      this->kGood = false;
      return v;
    }

    // Check if next subblock is not a particle block
    auto sFirst = std::string((char*)this->vCurSub.data(),4);
    if (sFirst == "LONG" || sFirst == "EVTE")
    {
      // Tell we are done
      this->kDone = true;

      // Store the current subblock as the runEnd subblock
      this->vEnd = this->vCurSub;
    }

    return v;
  }
  else
    return std::vector<float>(this->vCurSub.data() + init, this->vCurSub.data() + iend);
}
