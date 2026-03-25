#ifndef __DEFINE_ASSERT_FUNCTIONS
#define __DEFINE_ASSERT_FUNCTIONS 

#include "Main.h"



//Include Boost assert handlers
#define BOOST_ENABLE_ASSERT_HANDLER 
#include <boost/assert.hpp>

//Define boost assert handlers
namespace boost {
    void assertion_failed(char const* expr, char const* function, char const* file, long line) {
        std::cerr << "Expression \"" << expr << "\" in file " << file << " in function " << function << " on line " << line << " has failed! The program will shut down." << std::endl;
        std::terminate();
    }

    void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, long line) {
        std::cerr << "Expression \"" << expr << "\" in file " << file << " in function " << function << " on line " << line << " has failed! Reason:\n" << msg << "\nThe program will now shut down." << std::endl;
        std::terminate();
    }
}

//Define ASSUMEs and UNREACHABLEs
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

#define __SLOW_BIGNUM 0
#define __FAST_BIGNUM 1

#endif