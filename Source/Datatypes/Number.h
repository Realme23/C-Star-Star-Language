#pragma once
#include "../Main.h"
//Used for the ASSERT etc in Function definitions; set choice of Bignum elsewhere
#include "../Asserts/ConfigReleaseChecked.h"

namespace c_star_star {
	namespace number {

		//number_: A configurable bignum for speed/debug/smallnum
		//set number_ from the implementations with Asserts_.h files

		using number_stringd = boost::multiprecision::number<boost::multiprecision::debug_adaptor<typename boost::multiprecision::cpp_int::backend_type>, boost::multiprecision::cpp_int::et>;
		using number_fast = boost::multiprecision::cpp_int;
		using number_small = signed long long;

		//__SLOW_BIGNUM: Use debug adaptor (shows the value as string in debugger)
		//__FAST_BIGNUM: Use bignums but no adaptor
		//If neither: Use Signed long long

		inline std::string to_string(number_stringd x) { return x.str(); }
		inline std::string to_string(number_fast x) { return x.str(); }
		inline std::string to_string(number_small x) { return std::to_string(x); }

		constexpr size_t to_size_t(number_stringd n) { ASSERT(n <= std::numeric_limits<size_t>::max(), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n.convert_to<size_t>(); }
		constexpr size_t to_size_t(number_fast n) { ASSERT(n <= std::numeric_limits<size_t>::max(), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n.convert_to<size_t>(); }
		constexpr size_t to_size_t(number_small n) { ASSERT(n <= (number_small)(std::numeric_limits<size_t>::max()), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n; }
	}
}