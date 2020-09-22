#include "../../include/cppyplot.hpp"

#include <vector>
#include <random>
#include <numeric>

#include <Eigen/Eigen/Core>

/*
* Uncomment seaborn imports in cppyplot_server.py for this example to work
*/

int main()
{
  Eigen::Matrix<double, 1000, 1> mat;
  mat.setRandom();

  Cppyplot::cppyplot pyp;

  pyp.raw(R"pyp(
    fig = plt.figure(figsize=(6,5))
    ax = sns.distplot(mat, kde=False,
                      hist_kws={"histtype": "bar", "rwidth": 3,
                                "alpha": 0.6, "color": "b"})
    sns.distplot(mat, kde=False,
                 hist_kws={"histtype": "step", "rwidth": 2,
                           "alpha": 0.9, "color": "orange"})
    ax.set_xlabel("Data bins", fontsize=12)
    ax.set_ylabel("Frequency", fontsize=12)
    ax.set_title("Histogram of C++ Eigen container", fontsize=14)
    plt.show()
  )pyp");
  pyp.data_args(_p(mat));

  // bivariate plot from https://seaborn.pydata.org/examples/layered_bivariate_plot.html

  std::vector<std::vector<double>> rand_mat (1000, std::vector<double>(2));
  std::random_device seed;
  std::mt19937 gen(seed());
  std::normal_distribution<double> norm(0.0, 10.0);

  // fill matrix
  for (auto& row: rand_mat)
  {
    for (auto& elem: row)
    { elem = norm(gen); }
  }

  pyp.raw(R"pyp(
  sns.set_theme(style="dark")
  f, ax = plt.subplots(figsize=(6,6))
  sns.scatterplot(x=rand_mat[:,0], y=rand_mat[:,1], s=5, color="0.15")
  sns.histplot(x=rand_mat[:,0], y=rand_mat[:,1], bins=50, pthresh=0.1, cmap="mako")
  sns.kdeplot(x=rand_mat[:,0], y=rand_mat[:,1], levels=5, color="w", linewidths=1)
  plt.show()
    )pyp");
  
  pyp.data_args(_p(rand_mat));

  return EXIT_SUCCESS;
}