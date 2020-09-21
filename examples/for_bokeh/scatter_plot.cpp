#include <algorithm>
#include <vector>

#include <random>
#include "../../include/cppyplot.hpp"

/*
  NOTE: For this to work, uncomment bokeh imports in cppyplot_server.py under folder 'include'
*/

int main()
{
  std::random_device seed;
  std::mt19937 gen(seed());

  std::normal_distribution<float> norm(0.0F, 2.0F);

  std::vector<float> rand_vecx(1000), rand_vecy(1000);
  std::for_each(rand_vecx.begin(), rand_vecx.end(), [&](float& elem){elem = norm(gen);});
  std::for_each(rand_vecy.begin(), rand_vecy.end(), [&](float& elem){elem = norm(gen);});

  Cppyplot::cppyplot pyp;

  /* 
    Using the stream insertion operator <<
  */
  pyp << "source = ColumnDataSource(data=dict(x=rand_vecx, y=rand_vecy))";
  pyp << "plot = figure(plot_width=800, plot_height=800, x_axis_label='x', y_axis_label='y')";
  pyp << "plot.scatter('x', 'y', source=source, fill_color=\"orange\", alpha=0.6)";
  pyp << "layout = row(plot)";
  pyp << "show(layout)";
  pyp.data_args(_p(rand_vecx), _p(rand_vecy));

  /*
    Using the raw string literal support with member function 'raw'
  */
  pyp.raw(R"pyp(

  source = ColumnDataSource(data=dict(x=rand_vecx, y=rand_vecy))
  plot = figure(plot_width=800, plot_height=800, x_axis_label='x', y_axis_label='y')
  plot.scatter('x', 'y', source=source, fill_color="orange", alpha=0.6)
  layout = row(plot)
  show(layout)
  )pyp");

  pyp.data_args(_p(rand_vecx), _p(rand_vecy));
  


}