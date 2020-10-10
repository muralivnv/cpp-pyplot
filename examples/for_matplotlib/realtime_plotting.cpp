#include "../../include/cppyplot.hpp"
#include <random>

int main()
{
  std::random_device seed;
  std::mt19937 gen(seed());
  std::normal_distribution<float> norm(0.0, 0.5F);

  Cppyplot::cppyplot pyp;

  pyp.raw(R"pyp(
    plt.ion()

    fig = plt.figure(figsize=(6,5))
    plt.axis([0, 500, -1.5,1.5])
    plt.grid(True)
    plt.xlabel("Index", fontsize=12)
    plt.ylabel("Data", fontsize=12)
  )pyp");

  for (std::size_t i = 0u; i < 500u; i++)
  {
    auto data = norm(gen);
    pyp.raw(R"pyp(
      plt.scatter(i, data, s=2, c='b', linewidths=1, alpha=0.4)
      plt.show()
      plt.pause(0.1)
    )pyp", _p(data), _p(i));
  }
  return EXIT_SUCCESS;
}