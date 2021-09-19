#ifndef _CPPYPLOT_CONTAINER_SUPPORT_H_
#define _CPPYPLOT_CONTAINER_SUPPORT_H_


/* ZMQ requires custom dealloc function for zero-copy */
void custom_dealloc(void* data, void* hint)
{
  (void)data;
  (void)hint; 
  return; 
}

template<typename T>
struct is_string : std::false_type {};

template<>
struct is_string<std::string> : std::true_type {};

template<>
struct is_string<std::string_view> : std::true_type {};

template<typename T>
inline constexpr bool is_string_v = is_string<T>::value;

/*
  * Integral and floating point datatypes
*/
template<typename T>
inline auto container_size(const T data)
        -> typename std::enable_if<std::is_arithmetic_v<T>, std::size_t>::type
{ (void)(data); return 1u; }

template<typename T>
inline auto container_shape(const T data)
        -> typename std::enable_if<std::is_arithmetic_v<T>, std::array<std::size_t, 1>>::type
{ (void)(data); return std::array<std::size_t, 1>{0u}; }

template<typename T>
inline auto fill_zmq_buffer(const T data, zmq::message_t& buffer)
        -> typename std::enable_if<std::is_arithmetic_v<T>, void>::type
{
  buffer.rebuild(&data, sizeof(T));
}


/*
  * std::string and std::string_view
*/
template<typename T>
inline auto container_size(const T& data)
        -> typename std::enable_if<is_string_v<T>, std::size_t>::type
{ return data.size(); }

template<typename T>
inline auto container_shape(const T& data)
        -> typename std::enable_if<is_string_v<T>, std::array<std::size_t, 1>>::type
{ (void)(data); return std::array<std::size_t, 1>{data.size()}; }

template<typename T>
inline auto fill_zmq_buffer(const T& data, zmq::message_t& buffer)
        -> typename std::enable_if<is_string_v<T>, void>::type
{
  buffer.rebuild((void*)data.data(), sizeof(typename T::value_type)*data.size(), custom_dealloc, nullptr);
}

/*  
  * 1D Vector 
*/
template<typename T>
inline std::size_t container_size(const std::vector<T>& data)
{ return data.size(); }

template<typename T>
inline std::array<std::size_t,1> container_shape(const std::vector<T>& data)
{ return std::array<std::size_t, 1>{data.size()}; }

template<typename T>
inline void fill_zmq_buffer(const std::vector<T>& data, zmq::message_t& buffer)
{
  buffer.rebuild((void*)data.data(), sizeof(T)*data.size(), custom_dealloc, nullptr);
}

/*
  * 1D Array
*/
template<typename T, std::size_t N>
inline std::size_t container_size(const std::array<T, N>& data)
{ (void)data; return N; }

template<typename T, std::size_t N>
inline std::array<std::size_t,1> container_shape(const std::array<T, N>& data)
{ (void)data; return std::array<std::size_t, 1>{N}; }

template<typename T, std::size_t N>
inline void fill_zmq_buffer(const std::array<T, N>& data, zmq::message_t& buffer)
{
  buffer.rebuild((void*)data.data(), sizeof(T)*N, custom_dealloc, nullptr);
}

/*
  * 2D Vector
*/
template<typename T>
inline std::size_t container_size(const std::vector<std::vector<T>>& data)
{ return data.size()*data[0].size(); }

template<typename T>
inline std::array<std::size_t, 2> container_shape(const std::vector<std::vector<T>>& data)
{ return std::array<std::size_t, 2>{data.size(), data[0].size()}; }

// this uses mempcpy, there will be a runtime overhead
template<typename T>
inline void fill_zmq_buffer(const std::vector<std::vector<T>>& data, zmq::message_t& buffer)
{
  buffer.rebuild(sizeof(T)*data.size()*data[0].size());
  char * ptr = static_cast<char*>(buffer.data());

  std::size_t offset = 0u;
  for (std::size_t i = 0u; i < data.size(); i++)
  {
    std::size_t n_bytes = sizeof(T)*data[i].size();
    memcpy(ptr+offset, data[i].data(), n_bytes);
    offset += n_bytes;
  }
}

/*
  * 2D Array
*/
template<typename T, std::size_t N, std::size_t M>
inline std::size_t container_size(const std::array<std::array<T, M>, N>& data)
{ (void)data; return N*M; }

template<typename T, std::size_t N, std::size_t M>
inline std::array<std::size_t, 2> container_shape(const std::array<std::array<T, M>, N>& data)
{ (void)data; return std::array<std::size_t, 2>{N, M}; }

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


// Eigen Container support
#if defined (EIGEN_AVAILABLE)
/*Reference: https://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html */
template<typename Derived>
inline std::size_t container_size(const Eigen::EigenBase<Derived>& eigen_container)
{
  return eigen_container.size();
}

template<typename Derived>
inline std::array<std::size_t, 2> container_shape(const Eigen::EigenBase<Derived>& eigen_container)
{
  return std::array<std::size_t, 2>{(std::size_t)eigen_container.rows(), (std::size_t)eigen_container.cols()};
}

template<typename Derived>
inline void fill_zmq_buffer(const Eigen::EigenBase<Derived>& eigen_container, zmq::message_t& buffer)
{
  auto elem_size = sizeof(typename Derived::value_type);
  buffer.rebuild((void*)eigen_container.derived().data(), elem_size*eigen_container.size(), 
                 custom_dealloc, nullptr);
}

#endif

#endif
