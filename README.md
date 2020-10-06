# cpp-pyplot

### Simple yet powerful, header-only library to plot c++ containers using python plotting tools and ZMQ protocol

#### **C++ Requirements**
* -std >= C++17
* [cppzmq](https://github.com/zeromq/cppzmq)
* libzmq

#### **Python Requirements**
* [zeromq](https://anaconda.org/anaconda/zeromq)
* [pyzmq](https://anaconda.org/conda-forge/pyzmq)
* [asteval](https://anaconda.org/conda-forge/asteval)

## Motivation
Many C++ plotting libraries that exist today, they tend to replicate python plotting API in C++. There are 3 limitations with this approach. 
1.  There is only so much a developer can acheive in C++ to replicate python plotting API (for example, using named input args during function call is not that easy to implement on C++).  
2.  The end user need to learn new plotting API. 
3.  The most important of all, no/incomplete documentation. 

Even if the user is fine with the above limitations, he/she is limited to just one plotting library. What if there comes a need to use awesome **seaborn** capabilities or **bokeh-plot** features or **Plotly**, there are no packages that are readily available and even if they are, I am pretty sure that the above limitations will come into action.   

Instead of reinventing the wheel this library uses python to do it's plotting. Given the awesome open source libraries, ZeroMQ and python-ASTEVAL, data pipe latency with ZeroMQ for inter process communication and latency for string command execution on python is negligible. 

## To the User
⭐ this repo if you are currently using it or if you find this implementation interesting. This way, if any other user comes across this repo, he/she may find it more interesting to dig a little deeper. 

And if you are currently using this library, post a sample plotting snippet using the Issue tab and tagging the issue with label `sample_usage`.

## Usage
```cpp
// include header 
#include <cppyplot.hpp>

// instantiate plot object
Cppyplot::cppyplot pyp;

// sample container
std::vector<float> vec(100);
std::iota(vec.begin(), vec.end(), 0.0F);

/* 
  Using stream insertion operator << 
*/

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

/* 
  Using Raw-string literal with member function 'raw'
  Notice how the member function 'raw' takes containers as arguments along with the plotting commands
*/
pyp.raw("Rpyp(
  plt.figure(figsize=(12,7))
  plt.plot(vec, 'r-*', markersize=2, linewidth=1)
  plt.grid(True)
  plt.xlabel("index")
  plt.ylabel("vec");
  plt.show()
)pyp", _p(vec));
```

For more complicated examples using matplotlib-subplots and bokeh-lib see **examples** folder in this repo.

## ```pyp.data_args``` and ```pyp.raw```
Member function, `data_args` and `raw`, is a variadic template function. Each container that is passed to `data_args` (or) `raw` need to be wrapped using the macro `_p` (`p` stands for *pair*).   
For example, instead of passing containers like   
`pyp.data_args(vec_x, vec_y, vec_z, ...)`   
pass the containers by wrapping them with macro `_p`   
`pyp.data_args(_p(vec_x), _p(vec_y), _p(vec_z))`

The macro `_p` captures variable name and expands into ("variable_name", variable) pair.

**Note**: Every container that is passed to python for plotting will be converted into an numpy array. This means fancy array slicing and array manipulations is possible. Just treat data as if it is originated in python with numpy.

Currently supports
* 1D std vector
* 1D std array
* 2D std vector
* 2D std array
* Eigen containers

For information on how to include support for custom containers read section [custom container support](https://github.com/muralivnv/Cppyplot#custom-container-support). 

## How-it-works
Plot object `cppyplot` passes all the commands and containers to a python server (which is spawned automatically when the `cppyplot` object is created) using ZeroMQ. The spawned python server uses [asteval](https://anaconda.org/conda-forge/asteval) library to parse the passed commands. This means any command that can be used in python can be written on C++ side.     

Note that the usage is not limited to just matplotlib. Bokeh, Plotly, etc. can also be used as long as the required libraries are available on the python side and imported in the `cppyplot_server.py` file under **include** directory.  

## Python-path
If python is installed under different directory, pass python path to `cppyplot` object as

```cpp
Cppyplot::cppyplot pyp(python_path_str); // python_path_str is the full path to python executable
```

## Compilation
As the library uses zmq, link the source file which uses cppyplot.hpp with the libraries from `libzmq`  

## Custom Container Support
By defining 3 helper functions, any c++ container can be adapted to pass onto python side. 

Define the following functions at the end of the **cppyplot_container_support.h** located in the include folder.

#### Define `container_size`
Define function to return container size (total number of elems). See example down below where `container_size` function was defined for containers of type `std::array` and `std::vector`.

```cpp
// 1D-array
template<typename T, std::size_t N>
inline std::size_t container_size(const std::array<T, N>& data)
{ (void)data; return N; }

// 2D-vector
template<typename T>
inline std::size_t container_size(const std::vector<std::vector<T>>& data)
{ return data.size()*data[0].size(); }
```

#### Define `container_shape`
Define function to return container shape (number of row, number of cols). See example down below where `container_shape` function was defined for containers of type `std::vector` and `std::vector<std::vector>`. This shape information will be used to reshape the array on python side using numpy.

```cpp
// 1D-vector
template<typename T>
inline std::array<std::size_t,1> container_shape(const std::vector<T>& data)
{ return std::array<std::size_t, 1>{data.size()}; }

// 2D-vector
template<typename T>
inline std::array<std::size_t, 2> container_shape(const std::vector<std::vector<T>>& data)
{ return std::array<std::size_t, 2>{data.size(), data[0].size()}; }
```

#### Define `fill_zmq_buffer`
The underlying zmq buffer requires either copying data to its buffer or pointing the zmq buffer to the container buffer. 

Use the technique down below to point zmq buffer to the container buffer if the data inside the container is stored in one single continuous buffer. See example down below, where `fill_zmq_buffer` is defined for 1D-vector. As the internal vector buffer is stored in one continuous buffer, we can just point zmq buffer to vector buffer with custom dealloc function.

```cpp
// 1D-vector
template<typename T>
inline void fill_zmq_buffer(const std::vector<T>& data, zmq::message_t& buffer)
{
  buffer.rebuild((void*)data.data(), sizeof(T)*data.size(), custom_dealloc, nullptr);
}
```

If the buffer insider custom container is not stored in one single continuous buffer(for example `vector<vector<float>>`) then use mempcy to copy data.

```cpp
// 2D vector
// this uses mempcpy, there will be a runtime overhead
template<typename T, std::size_t N, std::size_t M>
inline void fill_zmq_buffer(const std::array<std::array<T, M>, N>& data, zmq::message_t& buffer)
{
  buffer.rebuild(sizeof(T)*N*M);
  char * ptr = static_cast<char*>(buffer.data());

  std::size_t offset = 0u;
  size_t n_bytes = sizeof(T)*M;
  for (std::size_t i = 0u; i < N; i++)
  {
    memcpy(ptr+offset, data[i].data(), n_bytes);
    offset += n_bytes;
  }
}
```
