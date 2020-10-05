#include "../../include/cppyplot.hpp"

#include <cmath>
#include <algorithm>
#include <random>

int main()
{
  std::random_device seed;
  std::mt19937 gen(seed());
  std::normal_distribution<double> norm(0.0, 10.0);
 
  std::vector<std::vector<float>> image_vec(50, std::vector<float>(60));
  std::array<std::array<double, 100>, 100> image_arr;
  for (auto& row: image_vec)
  {
    std::iota(row.begin(), row.end(), 1.0F);
  }
  for (auto& row : image_arr)
  {
    for (auto& elem : row)
    { elem = norm(gen); }
  }

  Cppyplot::cppyplot pyp;

  pyp.raw(R"pyp(
  plt.figure(figsize=(6,5))
  plt.imshow(image_vec)
  plt.title("2D-C++ std vector", fontsize=14)
  plt.show(block=False)
  plt.figure(figsize=(6,5))
  plt.imshow(image_arr)
  plt.title("2D-C++ std array", fontsize=14)
  plt.show(block=True)
  )pyp", _p(image_vec), _p(image_arr));

  std::cout << "Good bye ...";
  
  return EXIT_SUCCESS;
}