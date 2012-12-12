#ifndef _COLOUR_DEFINITION_H_
#define _COLOUR_DEFINITION_H_

#include <string>
#include "pixel.h"
#include <iostream>

class ColourDefinition
{
 public:
  ColourDefinition();
  ColourDefinition( Pixel min, Pixel max );
  ~ColourDefinition();
  ColourDefinition(const ColourDefinition & rhs);
  ColourDefinition & operator=(const ColourDefinition & rhs);

  void reset();
  void addPixel(const Pixel & p);
  bool loadFromFile(std::string filename);
  bool saveToFile(std::string filename) const;
  bool isMatch(const Pixel & p) const;
  RawPixel getAverageColour() const;

  friend std::ostream & operator<<(std::ostream & os, ColourDefinition const & cd);
  friend std::istream & operator>>(std::istream & is, ColourDefinition & cd);

  Pixel min;
  Pixel max;
};



#endif

