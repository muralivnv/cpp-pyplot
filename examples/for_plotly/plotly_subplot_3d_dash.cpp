#include <vector>
#include <array>
#include <tuple>
#include <random>
#include <algorithm>

#include <thread>
#include <chrono>

#include "../../include/cppyplot.hpp"

using std::vector, std::array, std::size_t, std::mt19937;
using std::tuple, std::make_tuple, std::tie;
using namespace std::chrono_literals;

tuple<vector<float>, vector<float>> 
make_vectors(const array<float, 2>& xlim, const array<float, 2>& ylim, const size_t n_points)
{
  vector<float> x(n_points);
  vector<float> y(n_points);

  float step = (xlim[1] - xlim[0])/static_cast<float>(n_points);
  float next_val = xlim[0];
  auto filler = [&step, &next_val]()mutable{float val = next_val; next_val += step; return val; };

  // fill x
  std::generate(x.begin(), x.end(), filler);

  step = (ylim[1] - ylim[0])/static_cast<float>(n_points);
  next_val = ylim[0];

  // fill y
  std::generate(y.begin(), y.end(), filler);

  return make_tuple(x, y);
}

vector<float> surface_func(const vector<float>& x, const vector<float>& y)
{
  const size_t x_len = x.size();
  const size_t y_len = y.size();

  vector<float> z(x_len*y_len, 0.0F); // for faster copy
  
  std::random_device seed{};
  mt19937 gen(seed());
  std::normal_distribution<float> noise(0.0F, 0.05F);

  for (size_t i = 0u; i < x_len; i++)
  {
    for (size_t j = 0u; j < y_len; j++)
    {
      const size_t index = i*x_len + j;
      z[index] = (x[i]*x[i]*x[i]) + (y[i]*y[i]*y[i]);
      z[index] += noise(gen);
    }
  }
  return z;
}

int main()
{
  Cppyplot::cppyplot pyp;
  std::this_thread::sleep_for(5s);

  const array<float, 2> xlim {-5.0, 80.0};
  const array<float, 2> ylim {-5.0, 60.0};
  const size_t n_points = 100u;

  auto [x, y] = make_vectors(xlim, ylim, n_points);
  auto z = surface_func(x, y);

  // example reference: https://plotly.com/python/3d-subplots/
  // add the following lines to the top of cppyplot_server.py inside include folder 

  // # Import plotly
  // import plotly.io as pio
  // import plotly.graph_objects as go
  // from plotly.subplots import make_subplots
  // lib_sym['go']            = go
  // lib_sym['make_subplots'] = make_subplots
  // lib_sym['pio']           = pio

  // # Import dash
  // import dash
  // import dash_core_components as dcc
  // import dash_html_components as html
  // lib_sym['dash'] = dash
  // lib_sym['dcc'] = dcc
  // lib_sym['html'] = html

  pyp.raw(R"pyp(
  z = z.reshape((x.shape[0], y.shape[0]))
  fig = make_subplots(rows=2, cols=2, 
                      specs=[[{'type': 'surface'}, {'type': 'surface'}],
                           [{'type': 'surface'}, {'type': 'surface'}]])

  # adding surfaces to subplots
  fig.add_trace(go.Surface(x=x, y=y, z=z, colorscale='Viridis', showscale=False), row=1, col=1)
  fig.add_trace(
    go.Surface(x=x, y=y, z=z, colorscale='RdBu', showscale=False),
    row=1, col=2)

  fig.add_trace(
      go.Surface(x=x, y=y, z=z, colorscale='YlOrRd', showscale=False),
      row=2, col=1)

  fig.add_trace(
      go.Surface(x=x, y=y, z=z, colorscale='YlGnBu', showscale=False),
      row=2, col=2)

  fig.update_layout(
      title_text='3D subplots with different colorscales',
      height=800,
      width=800)
  
  app = dash.Dash()
  app.layout = html.Div([
    dcc.Graph(figure=fig)
  ])
  app.run_server(debug=False)
  )pyp", _p(x), _p(y), _p(z));

  return EXIT_SUCCESS;
}
