#include "cppyplot.hpp"

#include <cmath>
#include <algorithm>

int main()
{
  std::vector<float> angles_rad(100);
  float n = 0.0F, step=0.1F;
  std::generate(angles_rad.begin(), angles_rad.end(),   [&n, step]() mutable{ return (n += step);});
  
  std::vector<float> sin_angle(100), cos_angle(100);
  std::transform(angles_rad.begin(), angles_rad.end(), sin_angle.begin(), [](auto& elem){return std::sinf(elem);});
  std::transform(angles_rad.begin(), angles_rad.end(), cos_angle.begin(), [](auto& elem){return std::cosf(elem);});

  cppyplot pyp;

  pyp << "plt.figure(figsize=(6,5))";
  pyp << "plt.subplot(2,1,1)";
  pyp << "plt.plot(angles_rad, sin_angle, 'r-*', markersize=2, linewidth=1)";
  pyp << "plt.ylabel(\"sin\", fontsize=12)";
  pyp << "plt.grid(True)";
  pyp << "plt.subplot(2,1,2)";
  pyp << "plt.plot(angles_rad, cos_angle, 'g-o', markersize=2, linewidth=1)";
  pyp << "plt.grid(True)";
  pyp << "plt.xlabel(\"angle (rad)\")";
  pyp << "plt.ylabel(\"cos\", fontsize=12)";
  pyp << "plt.show()";
  pyp.data_args(_p(angles_rad), _p(sin_angle), _p(cos_angle));

  std::cout << "Good bye ...";
  
  return EXIT_SUCCESS;
}