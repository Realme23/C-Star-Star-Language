#pragma once

#include "Precomp.h"

//These values can be replaced with global/local variables:
//Whether to enable asserts
//Set higher number for heavier asserts
#define __ENABLE_ASSERTS 1 
//Whether to enable compiler assumption hints
#define __ENABLE_ASSUMES 0 
//Whether to allow known broken code
#define __NO_FIXME_PANIC true 

//Whether to use __SLOW_BIGNUM, __FAST_BIGNUM or __NO_BIGNUM
#define __USE_BIGNUMS __SLOW_BIGNUM





//Utilities
//Assert, Assume and Unreachable
//Asserts check for a boolean condition and assert (runtime) if it is not satisfied, with an error
//Heavy asserts are selectively enabled if asserts are enabled, based on the specified "weight" of the assertion
#define ASSERT(cond,reason) do { if(__ENABLE_ASSERTS > 0) BOOST_ASSERT_MSG(cond, reason); } while(0)
#define HEAVY_ASSERT(level,cond,reason) do { if(__ENABLE_ASSERTS > level) BOOST_ASSERT_MSG(cond, reason); } while(0)

//Always fail, even with asserts disabled
//Takes a single parameter, a message to display
#define PANIC(...) do { BOOST_ASSERT_MSG(false, __VA_ARGS__); UNREACHABLE_C; } while(0)
//Panics when reached, used to signal unreachable code
//Optionally take a parameter that describes why unimplemented
#define TODO { PANIC("Unimplemented."); }
#define TODO_(...) { PANIC("TODO: " __VA_ARGS__); }

//Indicate known, broken code
//Disable before release
//Only panic if __NO_FIXME_PANIC is disabled
#define FIXME(...) do { if(__NO_FIXME_PANIC) PANIC(__VA_ARGS__); } while(0)


#ifdef __cplusplus
#if (__cplusplus > 202302L)                  //C++ 23
#define ASSUME_C(cond,reason) do { [[assume((cond))]]; } while (0)
#define UNREACHABLE_C std::unreachable()
#else
#if defined(_MSC_VER) && !defined(__clang__) //MSVC
#define ASSUME_C(cond,reason) do { __assume((cond)); } while (0)
#define UNREACHABLE_C __assume(0)
#else
#ifdef __GNUC__                              //GCC, Clang
#define ASSUME_C(cond,reason) do { if (!(cond)) __builtin_unreachable(); } while (0)
#define UNREACHABLE_C __builtin_unreachable()
#else                                        //Fallback - No op
#define ASSUME_C(cond,reason)
#define UNREACHABLE_C
#endif
#endif
#endif
#endif


//Compiler specific declaration of assumptions used
//Suggested: Off in debug mode, on in release mode
#define ASSUME(cond,reason) do { ASSERT((cond), reason); if(__ENABLE_ASSUMES) ASSUME_C(cond,reason); } while(0)
//Compiler specific declaration of unreachability
//Suggested: Off in debug mode, on in release mode
#define UNREACHABLE(reason)  do { if(__ENABLE_ASSUMES) UNREACHABLE_C; else if(__ENABLE_ASSERTS) PANIC(reason); } while(0)
//Code that only runs when asserts are enabled
//eg Panic checks, preparations for asserts
#define ASSERT_RUN(...) do { if(__ENABLE_ASSERTS) __VA_ARGS__; } while(0)
//Code that only runs when heavy asserts and regular asserts are enabled
//eg Sorting, >O(n) preparations for asserts, run and check vs naive impl
#define HEAVY_ASSERT_RUN(level,...) do { if(__ENABLE_HEAVY_ASSERTS && __ENABLE_HEAVY_ASSERTS > level) __VA_ARGS__; } while(0)

//CASES: Assert that atleast one of the following is equal to the test value
// eg CASES(color, red, green, blue)
//CASES_TRUE: Assert that atleast one of the following evaluates to true
// eg CASES_TRUE(isEven(x), isOdd(x))
//Disabled without asserts
template<typename T, typename R>
constexpr void CASES_FUNCTION(boost::source_location const& loc, T check_value, R cases_list) {
    ASSUME(check_value == cases_list, ("Cases not matched! Source location: " + loc.to_string()).c_str());
}

template<typename T, typename Q, typename S>
constexpr void CASES_FUNCTION(boost::source_location const& loc, T check_value, Q first_case, S second_case) {
    if (check_value != first_case) {
        CASES_FUNCTION(loc, check_value, second_case);
    }
    ASSUME(check_value == first_case or check_value == second_case, "Cases must be satisfied!");
}

template<typename T, typename Q, typename S, typename... R>
constexpr void CASES_FUNCTION(boost::source_location const& loc, T check_value, Q first_case, S second_case, R... cases_list) {
    if (check_value != first_case) {
        CASES_FUNCTION(loc, check_value, second_case, cases_list...);
    }
    //ASSUME(check_value == first_case or check_value == second_case or (check_value == cases_list...), "Cases must be satisfied!");
}

template<typename... Q>
constexpr void CASES_TRUE_FUNCTION(boost::source_location const& loc, Q... cases) {
    CASES_FUNCTION(loc, true, cases...);
}


#define CASES(check_value,...) CASES_FUNCTION((BOOST_CURRENT_LOCATION), (check_value), __VA_ARGS__)
#define CASES_TRUE(...) CASES_TRUE_FUNCTION((BOOST_CURRENT_LOCATION), __VA_ARGS__)


//Type aliases
//A multiprecision integer from boost


#define __SLOW_BIGNUM 0
#define __FAST_BIGNUM 1

//__SLOW_BIGNUM: Use debug adaptor (shows the value as string in debugger)
//__FAST_BIGNUM: Use bignums but no adaptor
//If neither: Use Signed long long
#ifndef __USE_BIGNUMS
#error "Must specify bignums"
#endif

#if __USE_BIGNUMS == __SLOW_BIGNUM
using number = boost::multiprecision::number<boost::multiprecision::debug_adaptor<typename boost::multiprecision::cpp_int::backend_type>, boost::multiprecision::cpp_int::et>;
inline std::string to_string(number x) { return x.str(); }
constexpr size_t to_size_t(number n) { ASSERT(n <= std::numeric_limits<size_t>::max(), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n.convert_to<size_t>(); }
#else
#if __USE_BIGNUMS == __FAST_BIGNUM
using number = boost::multiprecision::cpp_int;
inline std::string to_string(number x) { return x.str(); }
constexpr size_t to_size_t(number n) { ASSERT(n <= std::numeric_limits<size_t>::max(), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n.convert_to<size_t>(); }
#else
#if __USE_BIGNUMS == __NO_BIGNUM
using number = signed long long;
constexpr std::string to_string(number x) { return std::to_string(x); }
constexpr size_t to_size_t(number n) { ASSERT(n <= std::numeric_limits<size_t>::max(), "Too big to fit"); ASSERT(n >= 0, "Must be non-negative"); return n; }
#endif
#endif
#endif

//An "any" type from boost
//Must model reasonably pure functions, values eg (x == x, y = x => y == x, f(y) == f(x) is true) etc
//Not expected to store eg "auto-iterating" iterators or multithreaded values
//Must be copy constructible, equality, includes typeid_ support
using any = boost::type_erasure::any<boost::mpl::vector<boost::type_erasure::copy_constructible<>, boost::type_erasure::typeid_<>, boost::type_erasure::equality_comparable<>, boost::type_erasure::relaxed>>;
//Reference-safe type_id from C++ std
using std::type_index;


#define BOOST_ENABLE_ASSERT_HANDLER 
#include <boost/assert.hpp>
