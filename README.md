# Cppyplot

### Simple, header-only library to plot c++ containers using python plotting tools and ZMQ protocol

#### **C++ Requirements**
* -std >= C++17
* [cppzmq](https://github.com/zeromq/cppzmq)
* libzmq

#### **Python Requirements**
* [zeromq](https://anaconda.org/anaconda/zeromq)
* [pyzmq](https://anaconda.org/conda-forge/pyzmq)
* [asteval](https://anaconda.org/conda-forge/asteval)

## Motivation
Many C++ plotting libraries that exist today, they tend to replicate python plotting API as much as they can to replicate API on C++. There are 3 limitations I observed 
1.  There is only much of what a developer can acheive on C++ to replicate python API (for example, using named input args during function call is not that easy to implement on C++). 
2.  The end user need to learn new plotting API. 
3.  No proper documentation. 

Even if the user is fine with the above limitations, he/she is limited to just one plotting library. What if there comes a need to use awesome **seaborn** capabilities or **bokeh-plot** features or **Plotly**, there are no packages that are readily available and even if they are, I am pretty sure that the above limitations will come into action.   

Instead of reinventing the wheel this library just uses python to do it's plotting. Given the awesome open source libraries, ZeroMQ and python-ASTEVAL, data pipe latency with ZeroMQ for inter process communication and latency for string command execution on python is negligible. 

## Usage

```cpp
// include header 
#include <cppyplot.hpp>

// instantiate plot object
cppyplot pyp;

// sample container
std::vector<float> vec(100);
std::iota(vec.begin(), vec.end(), 0.0F);

// to plot the above container, use the same plotting commands from python
pyp << "plt.figure(figsize=(12,7))";

// specify variable name directly in the plotting command
pyp << "plt.plot(vec, 'r-*', markersize=2, linewidth=1)";
pyp << "plt.grid(True)";
pyp << "plt.xlabel(\"index\")";
pyp << "plt.ylabel(\"vec\")";
pyp << "plt.show()";

// after all the plotting commands are written finalize the plot by passing data to plot object
pyp.data_args(_p(vec));
```

For more complicated examples using matplotlib-subplots and bokeh-lib see **examples** folder in this repo.

## ```pyp.data_args```
Member function, `data_args`, is a variadic template function. Each container that is passed into `data_args` need to be wrapped using the macro `_p` (`p` stands for *pair*).   
For example, instead of passing containers like   
`pyp.data_args(vec_x, vec_y, vec_z, ...)`   
pass the containers by wrapping them with macro `_p`   
`pyp.data_args(_p(vec_x), _p(vec_y), _p(vec_z))`

The macro `_p` captures variable name and expands into ("variable_name", variable) pair.

## ToDo
* ### Support for Raw-String literals  
By adding raw string literal support it is easy to write plotting commands in the following manner.

**Example-Matplotlib**
```cpp
pyp.raw(R"pyp(
  
  plt.figure(figsize=(6,5))
  plt.subplot(2,1,1)
  plt.plot(angles_rad, sin_angle, 'r-*', markersize=2, linewidth=1)
  plt.ylabel("sin", fontsize=12)
  plt.grid(True)

  plt.subplot(2,1,2)
  plt.plot(angles_rad, cos_angle, 'g-o', markersize=2, linewidth=1)
  plt.grid(True)
  plt.xlabel("angle (rad)")
  plt.ylabel("cos", fontsize=12)
  plt.show()

  )pyp", _p(angles_rad), _p(cos_angle), _p(sin_angle));
```
**Example-Bokeh**
```cpp
pyp.raw(R"pyp(

  source = ColumnDataSource(data=dict(x=rand_vecx, y=rand_vecy))
  plot = figure(plot_width=800, plot_height=800, x_axis_label='x', y_axis_label='y')
  plot.scatter('x', 'y', source=source, fill_color="orange", alpha=0.6)
  layout = row(plot)
  show(layout)

  )pyp", _p(rand_vecx), _p(rand_vecy));
```
This is easy to read and write.


## How-it-works
Plot object `cppyplot` passes all the commands and containers to a python server (which is spawned automatically when the `cppyplot` object is created) using ZeroMQ. The spawned python server uses [asteval](https://anaconda.org/conda-forge/asteval) library to parse the passed commands. This means any command that can be used in python can be written on C++ side.     

Note that the usage is not limited to just matplotlib. Bokeh, Plotly, etc. can also be used as long as the required libraries are available on the python side and imported in the `cppyplot_server.py` file under **include** directory.  

## Python-path
If python is installed under different directory, pass python path to `cppyplot` object as

```cpp
cppyplot pyp(python_path_str); // python_path_str is the full path to python executable
```

## Compilation
As the library uses zmq, link the source file which uses cppyplot.hpp with the libraries from `libzmq`  

## Custom Container Support
By defining 3 helper functions, any c++ container can be adapted to pass onto python side. 

#### Define `size_str`
Define function to transform container size (total number of elems) into string object. See example down below where `size_str` function was defined for containers of type `std::array` and `std::vector`.

```cpp
// 1D-array
template<typename T, std::size_t>
std::string size_str(const std::array<T>& container)
{
  return std::to_string(container.size());
}

// 2D-vector
template<typename T>
std::string size_str(const std::vector<std::vector<T>>& container)
{
  return std::to_string(container.size()*container[0].size());
}
```

Follow the link, [Eigen_Support](https://github.com/muralivnv/Cppyplot/blob/master/include/cppyplot.hpp#L171) to look at Eigen container support.  
**Note**: This function need to be defined before the class defintion `cppyplot` in file `cppyplot.hpp`.  

#### Define `shape_str`
Define function to transform container shape (number of row, number of cols) into string object. See example down below where `shape_str` function was defined for containers of type `std::vector` and `std::vector<std::vector>`. This shape information will be used to reshape the array on python side using numpy.

```cpp
// 1D-vector
template<typename T>
std::string shape_str(const std::vector<T>& container)
{
  return "(" + std::to_string(container.size()) + ",)";
} // returns "(n_rows,)"

// 2D-vector
template<typename T>
std::string shape_str(const std::vector<std::vector<T>>& container)
{
  return "(" + std::to_string(container.size()) + "," + container.front().size() + ")";
} // returns "(n_rows, n_cols)"
```

#### Define `void_ptr`
The underlying zmq publisher requires pointer to the raw buffer inside the container to pass the data to python server. For that purpose define function `void_ptr` to extract data pointer and cast it into `(void*)`. See example down below which shows how to extract raw pointers for containers of type `std::vector` and `std::array`.

```cpp
// 1D-vector
template<typename T>
void* void_ptr(const std::vector<T>& vec)
{
  return (void*)vec.data();
}

// 1D-array
template<typename T, std::size_t N>
void* void_ptr(const std::array<T, N>& arr)
{
  return (void*)arr.data();
}

```
