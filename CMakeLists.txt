cmake_minimum_required(VERSION 3.11)
project(cppyplot)

set (CMAKE_CXX_STANDARD 17)

## To Generate CMakeFiles for Debug mode with folder structure build/Debug
  # cmake ../.. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug 

# To generate CMakeFiles for Release mode with folder structure build/Release
  # cmake ../.. -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# Once the required CMakeFiles have been generated run command ninja to build
  # ninja

# MSVC compiler
set(CMAKE_C_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.27.29110/bin/Hostx86/x86/cl.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.27.29110/bin/Hostx86/x86/cl.exe")
if ((${CMAKE_BUILD_TYPE} MATCHES "Debug"))
  add_compile_options(-EHsc -Zi -RTC1 -MDd)
else()
  add_compile_options(/W4 /EHsc /O2 /fp:precise)
endif()

# CONAN package manager Specific
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# include directories
include(${CMAKE_BINARY_DIR}/custominclude.txt)

# matplotlib
add_executable(sinusoidal_animation examples/for_matplotlib/sinusoidal_animation.cpp)
add_executable(container_2d_imshow examples/for_matplotlib/container_2d_imshow.cpp)
add_executable(subplot             examples/for_matplotlib/subplot.cpp)

target_link_libraries(sinusoidal_animation ${CONAN_LIBS})
target_link_libraries(container_2d_imshow ${CONAN_LIBS})
target_link_libraries(subplot ${CONAN_LIBS})

# seaborn
add_executable(distplot examples/for_seaborn/distplot.cpp)
target_link_libraries(distplot ${CONAN_LIBS})

#bokeh
add_executable(scatter_plot examples/for_bokeh/scatter_plot.cpp)
target_link_libraries(scatter_plot ${CONAN_LIBS})