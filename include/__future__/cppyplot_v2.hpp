#ifndef _CPPYPLOT_
#define _CPPYPLOT_

/* References
  * python struct: https://docs.python.org/3/library/struct.html
  * python asteval: https://newville.github.io/asteval/api.html#asteval-api
  * cpp cppzmq: https://brettviren.github.io/cppzmq-tour/
*/

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <numeric>

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <cstdlib>
#include <thread>
#include <chrono>
#include <utility>
#include <filesystem>

// Eigen
#if __has_include(<Eigen/Core>)
  #include <Eigen/Core>
  #define EIGEN_AVAILABLE
#elif __has_include (<Eigen/Eigen/Core>)
  #include <Eigen/Eigen/Core>
  #define EIGEN_AVAILABLE
#endif

using namespace std::chrono_literals;
using namespace std::string_literals;

#define PYTHON_PATH "C:/Anaconda3/python.exe"
#define PUB_HOST_ADDR "tcp://127.0.0.1:5555"
#define SUB_HOST_ADDR "tcp://127.0.0.1:5556"

template <std::size_t ... indices>
decltype(auto) build_string(const char * str, 
                            std::index_sequence<indices...>) 
{
  return std::string{str[indices]...};
}

template <std::size_t N>
constexpr decltype(auto) compile_time_str(const char(&str)[N]) 
{
  return build_string(str, std::make_index_sequence<N-1>());
}

#define STRINGIFY(X) (#X)
#define DATA_PAIR(X) std::make_pair(compile_time_str(STRINGIFY(X)), std::ref(X))
#define _p(X) DATA_PAIR(X)

namespace Cppyplot
{

// utility function for raw string literal parsing
std::string dedent_string(const std::string_view raw_str);

template<typename T>
std::string shape_str(const T& shape);

#include "../cppyplot_types.h"
#include "../cppyplot_container_support.h"

class cppyplot{
  private:
    static zmq::context_t context_;
    static zmq::socket_t socket_pub_;
    static zmq::socket_t socket_sub_;
    static bool          is_zmq_established_;
    static std::string   python_path_;
    static std::string   zmq_ip_sub_addr_;
    static std::string   zmq_ip_pub_addr_;
    static std::string   custom_import_file_;
    std::stringstream    plot_cmds_;
  public:
    cppyplot()
    {
      if (cppyplot::is_zmq_established_ == false)
      {
        cppyplot::socket_pub_.bind(cppyplot::zmq_ip_pub_addr_);
        std::this_thread::sleep_for(100ms);
      
        std::filesystem::path path(__FILE__);
        std::string server_file_spawn, parent_path = path.parent_path().string();
        if (cppyplot::custom_import_file_.empty())
        {
          cppyplot::custom_import_file_ = parent_path;
          cppyplot::custom_import_file_.append("/lib_imports.py"s);
        }

#if defined(_WIN32) || defined(_WIN64)
        server_file_spawn.append("start /min "s);
#endif
        server_file_spawn.append(cppyplot::python_path_);
        server_file_spawn += " "s;
        server_file_spawn += parent_path;
        server_file_spawn += "/cppyplot_server.py "s;
        server_file_spawn += "--sub_addr "s;
        server_file_spawn.append(cppyplot::zmq_ip_pub_addr_);
        server_file_spawn += " --pub_addr "s;
        server_file_spawn.append(cppyplot::zmq_ip_sub_addr_);
        server_file_spawn += " --custom_import_file_path "s;
        server_file_spawn.append(cppyplot::custom_import_file_);

#if defined(__unix__)
        server_file_spawn += " &"s;
#endif
        std::system(server_file_spawn.c_str());
        std::this_thread::sleep_for(1.5s);
        cppyplot::socket_sub_.connect(cppyplot::zmq_ip_sub_addr_);

        cppyplot::is_zmq_established_ = true;
        std::atexit(zmq_kill_command);
      }
    }
    
    cppyplot(cppyplot& other) = delete;
    cppyplot operator=(cppyplot& other) = delete;
    ~cppyplot() {}

    static void set_python_path(const std::string& python_path) noexcept
    { cppyplot::python_path_ = python_path; }

    static void set_host_ip(const std::string& pub_ip, const std::string& sub_ip) noexcept
    { cppyplot::zmq_ip_pub_addr_ = pub_ip; cppyplot::zmq_ip_sub_addr_ = sub_ip; }

    static void set_lib_imports(const std::string& imports_file) noexcept
    { cppyplot::custom_import_file_ = import_file; }

    static void zmq_kill_command()
    {
      if (cppyplot::is_zmq_established_ == true)
      {
        // if the python server is spawned through this class instance, then send exit command
        zmq::message_t exit_msg("exit", 4);
        cppyplot::socket_pub_.send(exit_msg, zmq::send_flags::none);
        
        cppyplot::is_zmq_established_ = false;
        cppyplot::socket_pub_.disconnect(cppyplot::zmq_ip_addr_);
        cppyplot::socket_sub_.disconnect(SUB_HOST_ADDR);
      }
    }

    inline void push(const std::string& cmds)
    { plot_cmds_ << cmds << '\n'; }

    inline void operator<<(const std::string& cmds)
    { this->push(cmds); }

    template<unsigned int N>
    void raw(const char (&input_cmds)[N]) noexcept
    {
      plot_cmds_ << dedent_string(input_cmds);
    }

    template<unsigned int N, typename... Val_t>
    void raw(const char (&input_cmds)[N], std::pair<std::string, Val_t>&&... args)
    {
      plot_cmds_ << dedent_string(input_cmds);
      data_args(std::forward<std::pair<std::string, Val_t>>(args)...);
    }

    template<unsigned int N>
    void raw_nowait(const char (&input_cmds)[N])
    {
      plot_cmds_ << dedent_string(input_cmds);
      
      zmq::message_t cmds(plot_cmds_.str());
      cppyplot::socket_pub_.send(cmds, zmq::send_flags::none);

      zmq::message_t final("finalize", 8);
      cppyplot::socket_pub_.send(final, zmq::send_flags::none);

      /* reset */
      plot_cmds_.str("");
    }

    template<unsigned int N, typename... Val_t>
    void raw_recv(const char(&input_cmds)[N], std::pair<std::string, Val_t>&&... args)
    {
      plot_cmds_ << dedent_string(input_cmds);
      
      zmq::message_t cmds(plot_cmds_.str());
      cppyplot::socket_pub_.send(cmds, zmq::send_flags::none);

      (send_header_to_recv(args.first, args.second), ...);

      zmq::message_t final("finalize", 8);
      cppyplot::socket_pub_.send(final, zmq::send_flags::none);

      (recv_data(args), ...);
    }

    template<typename T>
    inline std::string create_header(const std::string& key, const T& cont) noexcept
    {
      auto elem_type = unpack_type<T>();
      std::string header{"data|"};

      // variable name
      header += key;
      header.append("|");

      // container element type (float or int or ...)
      header += std::string{elem_type.typestr};
      header.append("|");

      // total number of elements in the container
      header += std::to_string(container_size(cont));
      header.append("|");

      // shape of the container 
      header += shape_str(container_shape(cont));

      return header;
    }

    template <typename T>
    void send_container(const std::string& key, const T& cont)
    { 
      std::string data_header{create_header(key, cont)};
      data_header += "|dont_send_back";
      zmq::message_t msg(data_header.c_str(), data_header.length());
      cppyplot::socket_pub_.send(msg, zmq::send_flags::none);

      zmq::message_t payload;
      fill_zmq_buffer(cont, payload);
      cppyplot::socket_pub_.send(payload, zmq::send_flags::none);
    }

    template<typename T>
    void send_header_to_recv(const std::string& key, const T& cont)
    {
      std::string data_header{create_header(key, cont)};
      data_header += "|send_back";
      zmq::message_t msg(data_header.c_str(), data_header.length());
      cppyplot::socket_pub_.send(msg, zmq::send_flags::none);
    }

    template<typename... Val_t>
    void data_args(std::pair<std::string, Val_t>&&... args)
    {
      (send_container(args.first, args.second), ...);

      zmq::message_t cmds(plot_cmds_.str());
      cppyplot::socket_pub_.send(cmds, zmq::send_flags::none);

      zmq::message_t final("finalize", 8);
      cppyplot::socket_pub_.send(final, zmq::send_flags::none);

      /* reset */
      plot_cmds_.str("");
    }

    // TODO: Finish this
    template<typename T>
    void recv_data(std::pair<std::string, T>&& args)
    {
      zmq::message_t contents;
      zmq::recv_result_t result = cppyplot::socket_sub_.recv(contents, zmq::recv_flags::none);
      auto var_name             = contents.to_string_view();

      zmq::mutable_buffer mbuf = zmq::buffer(args.second.data());
      result                   = cppyplot::socket_sub_.recv(mbuf, zmq::recv_flags::none);
      std::this_thread::sleep_for(10ms);
    }
};

// initialize static variables
zmq::context_t cppyplot::context_             = zmq::context_t();
zmq::socket_t  cppyplot::socket_pub_          = zmq::socket_t(cppyplot::context_, ZMQ_PUB);
zmq::socket_t  cppyplot::socket_sub_          = zmq::socket_t(cppyplot::context_, zmq::socket_type::sub);
bool           cppyplot::is_zmq_established_  = false;
std::string    cppyplot::python_path_{PYTHON_PATH};
std::string    cppyplot::zmq_ip_pub_addr_{PUB_HOST_ADDR};
std::string    cppyplot::zmq_ip_sub_addr_{SUB_HOST_ADDR};

// utility functions
auto non_empty_line_idx(const std::string_view in_str)
{
  int new_line_pos        = -1;
  unsigned int num_spaces = 0u;
  for (unsigned int i = 0u; i < in_str.length(); i++)
  {
    if (in_str[i] == '\n')
    { new_line_pos = i; num_spaces = 0u; }
    else if (    (in_str[i] != ' ' ) 
              && (in_str[i] != '\t'))
    { break; }
    else
    { num_spaces ++; }
  }
  return std::make_tuple(new_line_pos+1, num_spaces);
}

std::string dedent_string(const std::string_view raw_str)
{
  auto [occupied_line_start, num_spaces] = non_empty_line_idx(raw_str);
  if (num_spaces == 0u)
  { return std::string(raw_str); }
  else
  {
    std::string out_str;
    out_str.reserve(raw_str.length());
    
    int line_start = -1;
    bool process_spaces = true;
    int cur_space_count = 0;
    for (unsigned int i = static_cast<unsigned int>(occupied_line_start); i < raw_str.length(); i++)
    {
      if (process_spaces == true)
      {
        if (raw_str[i] == '\n')
        { 
          process_spaces  = true;
          line_start      = -1;
          cur_space_count = 0;
        }
        else if ((raw_str[i] != ' ') && (raw_str[i] != '\t'))
        {
          process_spaces = false;
          line_start     = i;
        }
        else
        { cur_space_count++; }
      }
      else if (raw_str[i] == '\n')
      {
        if (line_start != -1)
        {
          line_start -= (cur_space_count - num_spaces);
          out_str.append(raw_str.substr(line_start, i - line_start + 1u)); 
        }
        process_spaces  = true;
        line_start      = -1;
        cur_space_count = 0;
      }
    }
    return out_str;
  }
}

template<typename T>
std::string shape_str(const T& shape)
{
  std::stringstream out_str;
  out_str << '(';
  for (auto size : shape)
  {
    out_str << size << ',';
  }
  out_str << ')';

  return out_str.str();
}

}

#endif
