#include <iostream>
#include <iomanip>
#include <string>

#include <CorsikaFile.h>
#include <CorsikaShower.h>


//
// The constructor
//
CorsikaFile::CorsikaFile(std::string s)
: stream(s, std::ifstream::in | std::ifstream::binary)
, nBlockSize(0)          // to be determiend
, nSubWords(0)           // to be determined
, nSubBlocks(21)         // from manual
, nWordSize(4)           // from manual
, nParticlesPerBlock(39) // from manual
, nWordsPerParticle(7)   // from manual (+1 below if thinning)
, iCurSub(0)
, kThin(false)
, kSkip(false)
, kGood(true)
, kDone(false)
, sFileName(s)
{
  //
  // Check if file is open
  //
  if (!this->stream.is_open())
  {
    std::cerr << "Could not open file " << s << "." << std::endl;
    this->kGood = false;
    return;
  }

  //
  // Learn how to read the current file
  //
  char buf[this->nWordSize];

  // Read the first word
  this->stream.read(buf,this->nWordSize);

  // Check if it has block size information
  if (std::string(buf,this->nWordSize) == "RUNH")
  { // There is no information about the block size

    // Guess that the sub-block size has 273 words and check it
    this->stream.ignore(this->nWordSize*272);
    this->stream.read(buf,this->nWordSize);

    if (std::string(buf,this->nWordSize) == "EVTH")
    { // The sub block has 273 words (no thinning)
      this->nBlockSize = 22932;
    }
    else
    { // Still ditn't find the event header, guess thinning is enabled
      this->stream.ignore(this->nWordSize*39);
      this->stream.read(buf,this->nWordSize);
      if (std::string(buf,this->nWordSize) == "EVTH")
      { // The sub block has 312 words (thinning enabled)
        this->nBlockSize = 26208;
      }
    }
  }
  else
  {
    this->nBlockSize = *(int*)buf;
    this->kSkip = true;
  }

  // Check block size
  if (this->nBlockSize == 22932)
  {
    // No thinning case
  }
  else if (this->nBlockSize == 26208)
  {
    // Thinning case
    this->kThin = true;
    this->nWordsPerParticle++;
  }
  else
  {
    std::cerr << "I don't know how to read the file " << s << "." << std::endl;
    std::cerr << "Is this a sane CORSIKA output file?" << std::endl;
    this->kGood = false;
    return;
  }

  // Get number of words per subblock
  this->nSubWords = this->nBlockSize/(this->nWordSize*this->nSubBlocks);

  // Rewind file
  this->stream.seekg(0);



  //
  // Read run header
  //
  this->vHeader = this->NextSubBlock();

  if (this->vHeader.empty())
  {
    std::cerr << "I could not read the run header for the file " << s << "." << std::endl;
    std::cerr << "Is this a sane CORSIKA output file?" << std::endl;
    this->kGood = false;
    return;
  }

  this->stream.seekg(0, std::ios::end);
  this->stream.seekg(size_t(this->stream.tellg()) - this->nBlockSize - 2*this->nWordSize*int(this->kSkip));



  //
  // Look for run end block
  //

  // Reset read status
  this->Reset();

  // Go to the end of file
  this->stream.seekg(0, std::ios::end);

  // Go to the beginning of the last block
  this->stream.seekg(size_t(this->stream.tellg()) - this->nBlockSize - 2*this->nWordSize*int(this->kSkip));

  // Seek for the run end subblock
  bool kEnd = false;
  for (int i = 0; i<this->nSubBlocks; i++)
  {
    auto subBlk = this->NextSubBlock();
    if (std::string((char*)subBlk.data(),4) == "RUNE")
    {
      this->vEnd = subBlk;
      kEnd = true;
      break;
    }
  }

  // Reset read status
  this->Reset();

  // Check if run end block was found
  if (!kEnd)
  {
    std::cerr << "I could not find the run end subblock in the file " << s << "." << std::endl;
    this->kGood = false;
    return;
  }

  return;
}



//
// The destructor
//
CorsikaFile::~CorsikaFile()
{
}


std::vector<float> CorsikaFile::NextSubBlock()
{
  if (this->iCurSub == 0 && this->kSkip)
  {
    char buf[this->nWordSize];
    this->stream.read(buf,this->nWordSize);
  }

  std::vector<float> v(this->nSubWords);
  this->stream.read((char*)v.data(),this->nSubWords*this->nWordSize);

  if (!this->stream.good() || this->stream.eof()) return std::vector<float>(0);

  iCurSub++;

  if (this->iCurSub == this->nSubBlocks)
  {
    iCurSub = 0;
    if (this->kSkip)
    {
      char buf[this->nWordSize];
      this->stream.read(buf,this->nWordSize);
    }
  }

  return v;
}



//
// Rewind the previously readed sub block
//
void CorsikaFile::RewindSubBlock()
{
  std::size_t pos = std::size_t(this->stream.tellg());

  if (this->iCurSub == 0)
  {
    pos -= (this->nSubWords+int(this->kSkip))*this->nWordSize;
    this->iCurSub = this->nSubBlocks - 1;
  }
  else if (this->iCurSub == 1)
  {
    pos -= (this->nSubWords+int(this->kSkip))*this->nWordSize;
    this->iCurSub = 0;
  }
  else
  {
    pos -= this->nSubWords*this->nWordSize;
    this->iCurSub--;
  }

  if (pos < 0) pos = 0;

  this->stream.seekg(pos);
}



//
// Retrieve the next shower
//
CorsikaShower CorsikaFile::NextShower()
{
  std::string sHeader = "";

  while(sHeader != "EVTH")
  {
    auto subBlk = this->NextSubBlock();

    // The following block could not be read by some reason.
    // Return an empty shower object and tell that something is wrong with the
    // current file.
    if (subBlk.empty())
    {
      if (this->stream.eof())
      {
        std::cerr << "Reached end of file " << this->sFileName << " before run end subblock!." << std::endl;
        std::cerr << "Was this simulation complete?" << std::endl;
        this->kGood = false;
      }
      else
      {
        std::cerr << "Could not read a subblock of data from " << this->sFileName << "!" << std::endl;
        std::cerr << "This was not expected!" << std::endl;
        this->kGood = false;
      }

      return CorsikaShower(*this,false);
    }

    sHeader = std::string((char*)subBlk.data(),4);

    // Check if we have reached the run end block
    if (sHeader == "RUNE")
    {
      this->kDone = true;
      this->vEnd = subBlk;

      return CorsikaShower(*this,false);
    }
  }

  this->RewindSubBlock();

  return CorsikaShower(*this);
}



//
// Go to beginning of file and reset subbloc counter
//
void CorsikaFile::Reset()
{
  this->stream.seekg(0);
  this->iCurSub = 0;
}
