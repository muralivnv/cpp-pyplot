# cpp-pyplot

### Simple yet powerful, header-only library to plot c++ containers using python plotting tools and ZMQ protocol

## Requirements
* **C++**
  - --std >= c++17
  - [cppzmq](https://github.com/zeromq/cppzmq)
  - libzmq
* **Python**
  - [zeromq](https://anaconda.org/anaconda/zeromq)
  - [pyzmq](https://anaconda.org/conda-forge/pyzmq)
  - [asteval](https://anaconda.org/conda-forge/asteval)

![](https://img.shields.io/badge/tested_on-Windows-brightgreen) ![](https://img.shields.io/badge/tested_on-Linux-brightgreen)

## Table of Contents
* [Motivation](https://github.com/muralivnv/cpp-pyplot#Motivation)
* [Usage](https://github.com/muralivnv/cpp-pyplot#Usage)
* [How it Works](https://github.com/muralivnv/cpp-pyplot#How-it-works)
* [Set-up](https://github.com/muralivnv/cpp-pyplot#Set-up)
* [API](https://github.com/muralivnv/cpp-pyplot#cppyplot)
  - [set_python_path](https://github.com/muralivnv/cpp-pyplot#set_python_path)
  - [set_host_ip](https://github.com/muralivnv/cpp-pyplot#set_host_ip)
  - [operator <<](https://github.com/muralivnv/cpp-pyplot#operator-)
  - [data_args](https://github.com/muralivnv/cpp-pyplot#data_args)
  - [raw](https://github.com/muralivnv/cpp-pyplot#raw)
* [Message to the User](https://github.com/muralivnv/cpp-pyplot#Message-to-the-User)
* [Container Support](https://github.com/muralivnv/cpp-pyplot#Container-Support)
  - [Custom Container Support](https://github.com/muralivnv/cpp-pyplot#Custom-Container-Support)
<br/> <br/>


## Motivation
Many C++ plotting libraries that exist today, they tend to replicate python plotting API in C++. There are 3 limitations with this approach. 
1.  There is only so much a developer can acheive in C++ to replicate python plotting API (for example, using named input args during function call is not that easy to implement on C++).  
2.  The end user need to learn new plotting API. 
3.  The most important of all, no/incomplete documentation. 

Even if the user is fine with the above limitations, he/she is limited to just one plotting library. What if there comes a need to use awesome **seaborn** capabilities or **bokeh-plot** features or **Plotly**, there are no packages that are readily available and even if they are, I am pretty sure that the above limitations will come into action.   

Given the awesome open source libraries, ZeroMQ and python-ASTEVAL, data pipe latency with ZeroMQ for inter process communication and latency for string command execution on python is negligible. 


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

// to plot the above std::vector, use the same plotting commands from python
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
  Member function 'raw' takes both plotting commands in string literal and data that need to be plotted.
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


### Animation

See [Sinusoidal_animation.cpp](https://github.com/muralivnv/cpp-pyplot/blob/master/examples/for_matplotlib/sinusoidal_animation.cpp) to reproduce below animation.
![](https://github.com/muralivnv/cpp-pyplot/blob/master/misc/sin.gif)

### 2D Distribution Plotting
See [distplot.cpp](https://github.com/muralivnv/cpp-pyplot/blob/master/examples/for_seaborn/distplot.cpp) to reproduce below figure.  
![](https://github.com/muralivnv/cpp-pyplot/blob/master/misc/distplot.png)

## How-it-works
Plot object `cppyplot` passes all the commands and containers to a python server (which is spawned automatically when an `cppyplot` object is created) using ZeroMQ. The spawned python server uses [asteval](https://anaconda.org/conda-forge/asteval) library to parse the passed commands. This means any command that can be used in python can be written on C++ side.     

Note that the usage is not limited to just matplotlib. Bokeh, Plotly, etc. can also be used as long as the required libraries are available on the python side and imported in the `cppyplot_server.py` file under **include** directory.  



## Set-up
As the library uses zmq, link the source file which uses cppyplot.hpp with the libraries from `libzmq`.  

### Installing Dependencies with **Conan**
If conan package manager is used, use the provided conanfile.txt along with the following commands to install dependencies.

* Debug mode  
At project head run the following commands in shell
```shell
mkdir build && cd build
mkdir Debug && cd Debug
conan install ../.. --build missing -s build_type=Debug
```
* Release mode  
At project head run the following commands in shell
```shell
cd build && mkdir Release
cd Release
conan install ../.. --build missing -s build_type=Release
```

### Building
Once the dependencies are installed, use the following cmake commands to build the examples. 
**Note:** `CMakeFiles.txt` was set to use MSVC and the following commands uses `Ninja` make system.
* Debug Mode  
```shell
cd build && cd Debug
cmake ../.. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
ninja
```
* Release Mode
```shell
cd build && cd Release
cmake ../.. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
```

## ```cppyplot```
Class `cppyplot` is a singleton class. This means multiple instantiations of cppyplot will use single zmq publisher and subscriber. 

### ```set_python_path```
If python is installed under different directory, pass python path to `cppyplot` using the function `set_python_path`. 
```cpp
#include "cppyplot.hpp"

int main()
{
  // This static function need to be called only once before the first instantiation of the plot object
  Cppyplot::cppyplot::set_python_path(PYTHON_PATH);
  Cppyplot::cppyplot pyp;
  ...
}
```

### ```set_host_ip```
If ZMQ connection need to be established under different address, specify it using the function `set_host_ip`. By default ```"tcp://127.0.0.1:5555"``` will be used.

```cpp
#include "cppyplot.hpp"

int main()
{
  // This static function need to be called only once before the first instantiation of the plot object
  Cppyplot::cppyplot::set_host_ip(HOST_ADDRESS);
  Cppyplot::cppyplot pyp;
  ...
}
```

### ```operator <<```
Plotting commands can be specified using stream insertion operator `<<`.
```cpp
pyp << "plt.figure(figsize=(12,7))";

// specify variable name directly in the plotting command
pyp << "plt.plot(vec, 'r-*', markersize=2, linewidth=1)";
pyp << "plt.grid(True)";
pyp << "plt.xlabel(\"index\")";
pyp << "plt.ylabel(\"vec\")";
pyp << "plt.show()";
```

### ```data_args```
Member function, `data_args`,  is a variadic template function. Each container that is passed to `data_args` need to be wrapped using the macro `_p` (`p` stands for *pair*).   
For example, instead of passing containers like   
`pyp.data_args(vec_x, vec_y, vec_z, ...)`   
pass the containers by wrapping them with macro `_p`   
`pyp.data_args(_p(vec_x), _p(vec_y), _p(vec_z))`

The macro `_p` captures variable name and expands into ("variable_name", variable) pair.

**Note:** Calling `data_args` function finalizes the plot and sends all the commands to the python server to plot.  

### ```raw```
Member function, `raw`, takes plotting commands in raw string literal format. An additional overload is provided for `raw` function to take data as arguments as well. Each container that is passed to `raw` need to be wrapped using the macro `_p` (similar to `data_args`).  This member function can be used in 2 ways. 

* **Usage_1**:
> Pass plotting commands to `raw` in string literal format and use `data_args` to specify containers to use.
```cpp  
pyp.raw("Rpyp(
  plt.figure(figsize=(12,7))
  plt.plot(vec, 'r-*', markersize=2, linewidth=1)
  plt.grid(True)
  plt.xlabel("index")
  plt.ylabel("vec");
  plt.show()
)pyp");
pyp.data_args(_p(vec));
```

* **Usage_2**:
> Pass both plotting commands and containers to use to function `raw`
```cpp  
pyp.raw("Rpyp(
  plt.figure(figsize=(12,7))
  plt.plot(vec, 'r-*', markersize=2, linewidth=1)
  plt.grid(True)
  plt.xlabel("index")
  plt.ylabel("vec");
  plt.show()
)pyp", _p(vec));
```

**Note**: Every container that is passed to python for plotting will be converted into an numpy array. This means python array slicing and data manipulations is possible.



## Message to the User
⭐ this repo if you are currently using this (or) like the approach.  
If you are currently using this library, post a sample plotting snippet by creating an issue and tagging it with the label `sample_usage`.


## Container Support
Following are the containers that are currently supported
* Integral and floating point types
* std::string and std::string_view
* 1D vector (`std::vector<T>`)
* 1D array  (`std::array<T, len>`)
* 2D vector (`std::vector<std::vector<T>>`)
* 2D array  (`std::array<std::array<T, cols>, rows>`)
* Eigen containers

### Custom Container Support
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
