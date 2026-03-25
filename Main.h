#ifndef __MAIN_H_INCLUDE_GUARD
#define __MAIN_H_INCLUDE_GUARD
#include "Precomp.h"

//An "any" type from boost
//Must model reasonably pure functions, values eg (x == x, y = x => y == x, f(y) == f(x) is true) etc
//Not expected to store eg "auto-iterating" iterators or multithreaded values
//Must be copy constructible, equality, includes typeid_ support
using any = boost::type_erasure::any<boost::mpl::vector<boost::type_erasure::copy_constructible<>, boost::type_erasure::typeid_<>, boost::type_erasure::equality_comparable<>, boost::type_erasure::relaxed>>;
//Reference-safe type_id from C++ std
using std::type_index;

using number_stringd = boost::multiprecision::number<boost::multiprecision::debug_adaptor<typename boost::multiprecision::cpp_int::backend_type>, boost::multiprecision::cpp_int::et>;
using number_fast = boost::multiprecision::cpp_int;
using number_small = signed long long;



#endif