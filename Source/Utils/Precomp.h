#pragma once

//Includes

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/debug_adaptor.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/type_erasure/any_cast.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/type_erasure/typeid_of.hpp>

//Lightweight exceptions lib
#include <boost/leaf.hpp>
//Parser lib
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/qi_stream.hpp>
#include <boost/spirit/include/qi_char.hpp>
//Expression template lib
#include <boost/proto/proto.hpp>
//Lazy function lib
#include <boost/phoenix.hpp>
//Hasher lib
//#include <boost/container_hash/hash.hpp>
//Lightweight threads lib
//#include <boost/fiber/fiber.hpp>
#undef min
#undef max

#include <iostream>
#include <unordered_map>
#include <string>
#include <typeindex>

#include <set>
#include <unordered_set>
#include <map>

#include <memory>
