#include "Utils.h"

#include <cmath>

namespace Bengine {
  
  const double PI = 4 * atan(1); //"This works in any programming language and gives PI to the precision of the particular machine." // http://www.cplusplus.com/forum/beginner/83485/ // const long double PI = 3.141592653589793238L;
  const double TWO_PI = PI * 2; //this is 360 degrees in radians

  const glm::vec4 DEFAULT_UV(0.0f, 0.0f, 1.0f, 1.0f);
  
}
