//No include guards!!! Can include multiple times

#ifndef __ASSERTS_CONFIG_SET
#define __ASSERTS_CONFIG_SET
#endif

//Clear assert configs
#ifdef __ENABLE_ASSERTS
#undef __ENABLE_ASSERTS
#endif
#ifdef __ENABLE_ASSUMES
#undef __ENABLE_ASSUMES
#endif
#ifdef __NO_FIXME_PANIC
#undef __NO_FIXME_PANIC
#endif
#ifdef __USE_BIGNUMS
#undef __USE_BIGNUMS
#endif


//These values can be replaced with global/local variables:
//Whether to enable asserts
//Set higher number for heavier asserts
#define __ENABLE_ASSERTS 1 
//Whether to enable compiler assumption hints
//ASSUME(x) checks validity with ASSERT(x)
#define __ENABLE_ASSUMES 1 
//Whether to allow known broken/incomplete code
#define __NO_FIXME_PANIC false 

//Whether to use __SLOW_BIGNUM, __FAST_BIGNUM or __NO_BIGNUM
#define __USE_BIGNUMS __FAST_BIGNUM

#include "AssertDefinitions.h"