#define BOOST_ENABLE_ASSERT_HANDLER 
#include <boost/assert.hpp>
#include <iostream>

//Assertion handlers
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
