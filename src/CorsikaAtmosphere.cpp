#include <iostream>
#include <iomanip>
#include <cmath>

#include <CorsikaAtmosphere.h>
#include <CorsikaFile.h>

CorsikaAtmosphere::CorsikaAtmosphere(CorsikaFile & cfile)
: a(0), b(0), c(0), h(0), d(0)
{
  if (cfile.Good()) this->Initialize(cfile);
  return;
}



CorsikaAtmosphere::CorsikaAtmosphere(std::string s)
: a(0), b(0), c(0), h(0), d(0)
{
  CorsikaFile cfile(s);
  if (cfile.Good()) this->Initialize(cfile);
  return;
}



void CorsikaAtmosphere::Initialize(CorsikaFile &cfile)
{
  for (int i=0; i<5; i++)
  {
    this->h.push_back(cfile.GetHeader(249+i));
    this->a.push_back(cfile.GetHeader(254+i));
    this->b.push_back(cfile.GetHeader(259+i));
    this->c.push_back(cfile.GetHeader(264+i));
  }

  for (int i=0; i<5; i++) this->d.push_back(this->Depth(this->h[i]));

  return;
}



void CorsikaAtmosphere::Print()
{
  for (int i=0; i<5; i++)
  {
    std::cout << std::setw(15) << this->h[i];
    std::cout << std::setw(15) << this->d[i];
    std::cout << std::setw(15) << this->a[i];
    std::cout << std::setw(15) << this->b[i];
    std::cout << std::setw(15) << this->c[i];
    std::cout << std::endl;
  }
}



double CorsikaAtmosphere::Depth(double height)
{
  if      (this->h[0] <= height && height < this->h[1]) return this->a[0] + this->b[0]*std::exp(-height/this->c[0]);
  else if (this->h[1] <= height && height < this->h[2]) return this->a[1] + this->b[1]*std::exp(-height/this->c[1]);
  else if (this->h[2] <= height && height < this->h[3]) return this->a[2] + this->b[2]*std::exp(-height/this->c[2]);
  else if (this->h[3] <= height && height < this->h[4]) return this->a[3] + this->b[3]*std::exp(-height/this->c[3]);
  else if (this->h[4] <= height) return this->a[4] - this->b[4]*height/this->c[4];
  else return -1.;
}



double CorsikaAtmosphere::Height(double depth)
{
  if      (this->d[0] >= depth && depth > this->d[1]) return this->c[0]*std::log(this->b[0]/(depth-this->a[0]));
  else if (this->d[1] >= depth && depth > this->d[2]) return this->c[1]*std::log(this->b[1]/(depth-this->a[1]));
  else if (this->d[2] >= depth && depth > this->d[3]) return this->c[2]*std::log(this->b[2]/(depth-this->a[2]));
  else if (this->d[3] >= depth && depth > this->d[4]) return this->c[3]*std::log(this->b[3]/(depth-this->a[3]));
  else if (this->d[4] >= depth) return this->c[4]*(this->a[4]-depth)/this->b[4];
  else return -1.;
}
