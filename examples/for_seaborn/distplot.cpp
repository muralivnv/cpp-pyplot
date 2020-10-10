#include "../../include/cppyplot.hpp"

#include <vector>
#include <random>
#include <numeric>

/*
* Uncomment seaborn imports in cppyplot_server.py for this example to work
*/

// bivariate plot from https://seaborn.pydata.org/examples/layered_bivariate_plot.html
int main()
{
  Cppyplot::cppyplot pyp;

  std::vector<std::vector<double>> rand_mat (5000, std::vector<double>(2));
  std::random_device seed;
  std::mt19937 gen(seed());
  std::normal_distribution<double> norm(0.0, 5.0);

  // fill 2D vector
  for (auto& row: rand_mat)
  {
    for (auto& elem: row)
    { elem = norm(gen); }
  }

#ifdef EIGEN_AVAILABLE
  // fill eigen matrix
  Eigen::Matrix<double, 5000, 2> mat;
  for (auto& row: mat.rowwise())
  {
    for (auto& elem : row)
    { elem = norm(gen); }
  }

  // Using Eigen matrix
  pyp.raw(R"pyp(
  sns.set_theme(style="dark")
  f, ax = plt.subplots(figsize=(6,6))
  sns.scatterplot(x=mat[:,0], y=mat[:,1], s=5, color="0.15")
  sns.histplot(x=mat[:,0], y=mat[:,1], bins=50, pthresh=0.1, cmap="mako")
  sns.kdeplot(x=mat[:,0], y=mat[:,1], levels=5, color="gray", linewidths=1)
  plt.grid(True)
  plt.xlabel("X", fontsize=12)
  plt.ylabel("Y", fontsize=12)
  plt.title("Numpy type slicing on C++ Eigen matrix", fontsize=14)
  plt.show(block=False)
    )pyp", _p(mat));
#endif

  // Using 2D std-vector
  pyp.raw(R"pyp(
  f, ax = plt.subplots(figsize=(6,6))
  sns.scatterplot(x=rand_mat[:,0], y=rand_mat[:,1], s=5, color="0.15")
  sns.histplot(x=rand_mat[:,0], y=rand_mat[:,1], bins=50, pthresh=0.1, cmap="mako")
  sns.kdeplot(x=rand_mat[:,0], y=rand_mat[:,1], levels=5, color="gray", linewidths=1)
  plt.grid(True)
  plt.xlabel("X", fontsize=12)
  plt.ylabel("Y", fontsize=12)
  plt.title("2D distribution plotting with seaborn", fontsize=14)
  plt.show()
    )pyp", _p(rand_mat));

  return EXIT_SUCCESS;
}