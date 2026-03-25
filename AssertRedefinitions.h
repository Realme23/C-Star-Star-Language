//No include guards!!! Can include multiple times

#ifndef __ASSERTS_CONFIG_SET
#error "Must set asserts before including this file"
#endif

#define CASES(check_value,...) CASES_FUNCTION(__ENABLE_ASSERTS, __ENABLE_ASSUMES, (BOOST_CURRENT_LOCATION), (check_value), __VA_ARGS__)
#define CASES_TRUE(...) CASES_TRUE_FUNCTION(__ENABLE_ASSERTS, __ENABLE_ASSUMES, (BOOST_CURRENT_LOCATION), __VA_ARGS__)

#undef ASSERT
#undef HEAVY_ASSERT
#undef PANIC
#undef TODO
#undef TODO_
#undef FIXME
#undef ASSUME
#undef UNREACHABLE
#undef ASSERT_RUN
#undef number_

#if __USE_BIGNUMS == __SLOW_BIGNUM
#define number_ c_star_star::number::number_stringd
#else
#if __USE_BIGNUMS == __FAST_BIGNUM
#define number_ c_star_star::number::number_fast
#else
#if __USE_BIGNUMS == __NO_BIGNUM
#define number_ c_star_star::number::number_small
#endif
#endif
#endif

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



//Compiler specific declaration of assumptions used
//Suggested: Off in debug mode, on in release mode
#define ASSUME(cond,reason) do { ASSERT((cond), reason); if(__ENABLE_ASSUMES) ASSUME_C(cond,reason); } while(0)
//Compiler specific declaration of unreachability
//Suggested: Off in debug mode, on in release mode
#define UNREACHABLE(reason)  do { if(__ENABLE_ASSUMES) UNREACHABLE_C; else if(__ENABLE_ASSERTS) PANIC(reason); } while(0)
//Code that only runs when asserts are enabled
//eg Panic checks, preparations for asserts
#define ASSERT_RUN(level,...) do { if(__ENABLE_ASSERTS > level) __VA_ARGS__; } while(0)

//The invariant section, with include guards
#include "AssertInvariants.h"
