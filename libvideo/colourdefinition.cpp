#include "colourdefinition.h"
#include <iostream>
#include <fstream>

ColourDefinition::ColourDefinition() :
  min(255,255,255),
  max(0,0,0)
{

}

ColourDefinition::ColourDefinition( Pixel min, Pixel max )
  :min(min),
   max(max)
{
}

ColourDefinition::ColourDefinition(const ColourDefinition & rhs) :
  min(rhs.min),
  max(rhs.max)
{
  // 0 
}

ColourDefinition::~ColourDefinition()
{
  // 0
}

ColourDefinition & ColourDefinition::operator=(const ColourDefinition & c)
{
  min.red = c.min.red;
  min.green = c.min.green;
  min.blue = c.min.blue;          
  min.red_green = c.min.red_green;
  min.red_blue = c.min.red_blue;
  min.green_blue = c.min.green_blue;
  min.red_ratio = c.min.red_ratio;
  min.green_ratio = c.min.green_ratio;
  min.blue_ratio = c.min.blue_ratio;

  max.red = c.max.red;
  max.green = c.max.green;
  max.blue = c.max.blue;
  max.red_green = c.max.red_green;
  max.red_blue = c.max.red_blue;
  max.green_blue = c.max.green_blue;
  max.red_ratio = c.max.red_ratio;
  max.green_ratio = c.max.green_ratio;
  max.blue_ratio = c.max.blue_ratio;
  
  return *this;
}

void 
ColourDefinition::reset()
{
  min = Pixel(255,255,255);
  max = Pixel(0,0,0);
}

void 
ColourDefinition::addPixel(const Pixel & p)
{
  min.setMinimum(p);
  max.setMaximum(p);
  std::cout << "Min: " << min << std::endl;
  std::cout << "Max: " << max << std::endl;
  std::cout << "Pixel: " << p << std::endl;
}

bool 
ColourDefinition::loadFromFile(std::string filename)
{
  bool success = false;
  if(filename == "")
    reset();
  else
    {
     
      std::ifstream file(filename.c_str());
      if(file)
	{
	  file >> (*this);
	  file.close();
	  success = true;
	}
      else
	{
	  std::cerr << "ColourDefinition::loadFromFile(std::string filename) error loading from file: " << filename << "\n";
	  reset();
	}
	
    }
  return success;
}

bool 
ColourDefinition::saveToFile(std::string filename) const
{
  bool success = false;
  if(filename != "")
    {
      std::ofstream file(filename.c_str());
      if(file)
	{
	  file << (*this);
	  file.close();
  	  success = true;
	}
      else
	{
	  std::cerr << __PRETTY_FUNCTION__ << " error saving to file: " << filename << "\n";
	}
	
    }
  return success;
}

bool 
ColourDefinition::isMatch(const Pixel & p) const
{
  //  std::cout << "Min: ";
  //  min.writeTo(std::cout);
  //  std::cout << "\nMax: ";
  //  max.writeTo(std::cout);
  //  std::cout << "\nPixel: ";
  //  Pixel pp(p);
  //  pp.writeTo(std::cout);
  //  std::cout << "\n";
  

  return p.red >= min.red               &&
         p.green >= min.green           &&
         p.blue >= min.blue             &&
         p.red_green >= min.red_green   &&
         p.red_blue >= min.red_blue     &&
         p.green_blue >= min.green_blue &&
         p.red_ratio >= min.red_ratio   &&
         p.green_ratio >= min.green_ratio &&
         p.blue_ratio >= min.blue_ratio &&

         p.red <= max.red               &&
         p.green <= max.green           &&
         p.blue <= max.blue             &&
         p.red_green <= max.red_green   &&
         p.red_blue <= max.red_blue     &&
         p.green_blue <= max.green_blue &&
         p.red_ratio <= max.red_ratio   &&
         p.green_ratio <= max.green_ratio &&
         p.blue_ratio <= max.blue_ratio;
}


std::ostream & 
operator<<(std::ostream & os, ColourDefinition const & cd)
{
  os << cd.min;
  os << cd.max;
  os << std::endl;
  return os;
}

std::istream & 
operator>>(std::istream & is, ColourDefinition & cd)
{
  is >> cd.min;
  is >> cd.max;
  return is;
}


RawPixel 
ColourDefinition::getAverageColour() const
{
  RawPixel rp(min);
  rp = rp + max;
  rp = rp / 2.0;

  return rp;
}



