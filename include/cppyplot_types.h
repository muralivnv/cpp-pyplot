#ifndef _CPPYPLOT_TYPES_H_
#define _CPPYPLOT_TYPES_H_

template<typename T, char str>
struct ValType{
  const static std::size_t elem_size = sizeof(T);
  const static char typestr{str};
};

template<typename T>
auto unpack_type(const T& arg)
{
  (void)arg;
  return unpack_type<typename T::value_type>(typename T::value_type{});
}

template<>
auto unpack_type<char> (const char& arg)
{
  (void)arg;
  return ValType<char, 'c'>{};
}

template<>
auto unpack_type<signed char> (const signed char& arg)
{
  (void)(arg);
  return ValType<signed char, 'b'>{};
}

template<>
auto unpack_type<unsigned char> (const unsigned char& arg)
{
  (void)(arg);
  return ValType<unsigned char, 'B'>{};
}

template<>
auto unpack_type<short> (const short& arg)
{
  (void)(arg);
  return ValType<short, 'h'>{};
}

template<>
auto unpack_type<unsigned short> (const unsigned short& arg)
{
  (void)arg;
  return ValType<unsigned short, 'H'>{};
}

template<>
auto unpack_type<int> (const int& arg)
{
  (void)arg;
  return ValType<int, 'i'>{};
}

template<>
auto unpack_type<unsigned int> (const unsigned int& arg)
{
  (void)arg;
  return ValType<unsigned int, 'I'>{};
}

template<>
auto unpack_type<long> (const long& arg)
{
  (void)arg;
  return ValType<long, 'l'>{};
}

template<>
auto unpack_type<unsigned long> (const unsigned long& arg)
{
  (void)arg;
  return ValType<unsigned long, 'L'>{};
}

template<>
auto unpack_type<long long> (const long long& arg)
{
  (void)arg;
  return ValType<long long, 'q'>{};
}

template<>
auto unpack_type<unsigned long long> (const unsigned long long& arg)
{
  (void)arg;
  return ValType<unsigned long long, 'Q'>{};
}

template<>
auto unpack_type<float> (const float& arg)
{
  (void)arg;
  return ValType<float, 'f'>{};
}

template<>
auto unpack_type<double> (const double& arg)
{
  (void)arg;
  return ValType<double, 'd'>{};
}

template<typename T>
decltype(auto) get_ValType(const T& arg)
{
  (void)arg;
  return unpack_type(arg);
}

#endif
