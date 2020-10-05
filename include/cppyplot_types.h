#ifndef _CPPYPLOT_TYPES_H_
#define _CPPYPLOT_TYPES_H_

template<typename T, char str>
struct ValType{
  const static std::size_t elem_size = sizeof(T);
  const static char typestr{str};
};

template<typename T>
constexpr auto unpack_type()
{  return unpack_type<typename T::value_type>();  }

template<>
constexpr auto unpack_type<char>()
{  return ValType<char, 'c'>{};  }

template<>
constexpr auto unpack_type<signed char>()
{  return ValType<signed char, 'b'>{};  }

template<>
constexpr auto unpack_type<unsigned char>()
{  return ValType<unsigned char, 'B'>{};  }

template<>
constexpr auto unpack_type<short> ()
{  return ValType<short, 'h'>{};  }

template<>
constexpr auto unpack_type<unsigned short> ()
{  return ValType<unsigned short, 'H'>{};  }

template<>
constexpr auto unpack_type<int> ()
{  return ValType<int, 'i'>{};  }

template<>
constexpr auto unpack_type<unsigned int> ()
{  return ValType<unsigned int, 'I'>{};  }

template<>
constexpr auto unpack_type<long> ()
{  return ValType<long, 'l'>{};  }

template<>
constexpr auto unpack_type<unsigned long> ()
{  return ValType<unsigned long, 'L'>{};  }

template<>
constexpr auto unpack_type<long long> ()
{  return ValType<long long, 'q'>{};  }

template<>
constexpr auto unpack_type<unsigned long long> ()
{  return ValType<unsigned long long, 'Q'>{};  }

template<>
constexpr auto unpack_type<float> ()
{  return ValType<float, 'f'>{};  }

template<>
constexpr auto unpack_type<double> ()
{  return ValType<double, 'd'>{};  }

#endif
