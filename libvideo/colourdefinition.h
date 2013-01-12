#ifndef _COLOUR_DEFINITION_H_
#define _COLOUR_DEFINITION_H_

#include <string>
#include "pixel.h"
#include <iostream>

class ColourDefinition
{
 public:
  ColourDefinition();
  ColourDefinition( std::string name );
  ColourDefinition( std::string name, Pixel min, Pixel max );
  ~ColourDefinition();
  ColourDefinition(const ColourDefinition & rhs);
  ColourDefinition & operator=(const ColourDefinition & rhs);
  std::string ToString( void ) const;

  void reset();
  void addPixel(const Pixel & p);
  bool loadFromFile(std::string filename);
  bool saveToFile(std::string filename) const;
  bool isMatch(const Pixel & p) const;
  RawPixel getAverageColour() const;

  friend std::ostream & operator<<(std::ostream & os, ColourDefinition const & cd);
  friend std::istream & operator>>(std::istream & is, ColourDefinition & cd);

  std::string name;
  Pixel min;
  Pixel max;
};



#endif

