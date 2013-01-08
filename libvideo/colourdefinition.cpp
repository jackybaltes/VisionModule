#include "colourdefinition.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <vector>

ColourDefinition::ColourDefinition() :
  name(""),
  min(255,255,255),
  max(0,0,0)
{

}

ColourDefinition::ColourDefinition( std::string name, Pixel min, Pixel max )
  :name(name),
   min(min),
   max(max)
{
}

ColourDefinition::ColourDefinition(const ColourDefinition & rhs) :
  name(rhs.name),
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
  name = c.name;
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
  name = "";
  min = Pixel(255,255,255);
  max = Pixel(0,0,0);
}

void 
ColourDefinition::addPixel(const Pixel & p)
{
  min.setMinimum(p);
  max.setMaximum(p);
#if defined(DEBUG)
  std::cout << "Min: " << min << std::endl;
  std::cout << "Max: " << max << std::endl;
  std::cout << "Pixel: " << p << std::endl;
#endif
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
  

  return true &&
    p.red >= min.red               &&
    p.green >= min.green           &&
    p.blue >= min.blue             &&
    p.red_green >= min.red_green   &&
    p.red_blue >= min.red_blue     &&
    p.green_blue >= min.green_blue &&
    //    p.red_ratio >= min.red_ratio   &&
    //    p.green_ratio >= min.green_ratio &&
    //    p.blue_ratio >= min.blue_ratio &&

    p.red <= max.red               &&
    p.green <= max.green           &&
    p.blue <= max.blue             &&
    p.red_green <= max.red_green   &&
    p.red_blue <= max.red_blue     &&
    p.green_blue <= max.green_blue &&
    //    p.red_ratio <= max.red_ratio   &&
    //    p.green_ratio <= max.green_ratio &&
    //    p.blue_ratio <= max.blue_ratio && 
         true;
}


std::ostream & 
operator<<(std::ostream & os, ColourDefinition const & cd)
{
  os << "{" 
     << cd.name 
     << "&"
     << cd.min.red
     << "&"
     << cd.min.green
     << "&"
     << cd.min.blue
     << "&"
     << cd.max.red
     << "&"
     << cd.max.green
     << "&"
     << cd.max.blue
     << "&"
     << cd.min.red_green
     << "&"
     << cd.min.red_blue
     << "&"
     << cd.min.green_blue
     << "&"
     << cd.max.red_green
     << "&"
     << cd.max.red_blue
     << "&"
     << cd.max.green_blue
     << "&"
     << cd.min.red_ratio
     << "&"
     << cd.min.green_ratio
     << "&"
     << cd.min.blue_ratio
     << "&"
     << cd.max.red_ratio
     << "&"
     << cd.max.green_ratio
     << "&"
     << cd.max.blue_ratio
     << "}";
  return os;
}

struct amp_reader: std::ctype<char> 
{
  amp_reader(): std::ctype<char>(get_table()) 
  {
  }
  
    static std::ctype_base::mask const* get_table() 
  {
    static std::vector<std::ctype_base::mask> rc(table_size, std::ctype_base::mask());
      
    rc[' '] = std::ctype_base::space;
    rc['&'] = std::ctype_base::space;
    rc['\n'] = std::ctype_base::space;
    return &rc[0];
  }
};

std::istream & 
operator>>(std::istream & is, ColourDefinition & cd)
{
  std::locale ampLocale( std::locale(), new amp_reader() );
  is.imbue( ampLocale );

  is.ignore(1,'{');
  is >> cd.name;
  //  is.ignore(1,'&');
  is >> cd.min.red;
  //  is.ignore(1,'&');
  is >> cd.min.green;
  //  is.ignore(1,'&');
  is >> cd.min.blue;
  //  is.ignore(1,'&');
  is >> cd.max.red;
  //  is.ignore(1,'&');
  is >> cd.max.green;
  //  is.ignore(1,'&');
  is >> cd.max.blue;
  //  is.ignore(1,'&');
  is >> cd.min.red_green;
  //  is.ignore(1,'&');
  is >> cd.min.red_blue;
  //  is.ignore(1,'&');
  is >> cd.min.green_blue;
  //  is.ignore(1,'&');
  is >> cd.max.red_green;
  //  is.ignore(1,'&');
  is >> cd.max.red_blue;
  //  is.ignore(1,'&');
  is >> cd.max.green_blue;
  //  is.ignore(1,'&');
  is >> cd.min.red_ratio;
  //  is.ignore(1,'&');
  is >> cd.min.green_ratio;
  //  is.ignore(1,'&');
  is >> cd.min.blue_ratio;
  //  is.ignore(1,'&');
  is >> cd.max.red_ratio;
  //  is.ignore(1,'&');
  is >> cd.max.green_ratio;
  //  is.ignore(1,'&');
  is >> cd.max.blue_ratio;
  is.ignore(1,'}');
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



