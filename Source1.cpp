#include "Precomp.h"

#define BOOST_ENABLE_ASSERT_HANDLER 
#include <boost/assert.hpp>



//TODOs:
//Remove all IMany after prototyping
//Use generationalPointers everywhere
//Change Tuples to have only index > 0
//Postconditions/preconditions for everything: test only function-related values, test all class invariants, don't test interfunctional relations with conditions
//Change all numeric integer types to number (all indices etc)
//  Find library that's "optimized for small case"
//Convert everything to free functions (Add UFCS later)



//Whether to enable "light/simple" asserts
#define __ENABLE_ASSERTS 0
//Whether to enable "heavy" asserts and the level to enable them at
//Higher levels have slower, more assertive code
#define __ENABLE_HEAVY_ASSERTS 0
//Whether to enable compiler assumption hints
#define __ENABLE_ASSUMES 0

//Utilities
//Assert, Assume and Unreachable
//Asserts check for a boolean condition and assert (runtime) if it is not satisfied, with an error
//Heavy asserts are selectively enabled if asserts are enabled, based on the specified "weight" of the assertion
#define ASSERT(cond,reason) do { if(__ENABLE_ASSERTS) BOOST_ASSERT_MSG(cond, reason); } while(0)
#define HEAVY_ASSERT(level,cond,reason) do { if(__ENABLE_ASSERTS && __ENABLE_HEAVY_ASSERTS && (__ENABLE_HEAVY_ASSERTS) > level) BOOST_ASSERT_MSG(cond, reason); } while(0)

//Always fail, even with asserts disabled
//Takes a single parameter, a message to display
#define PANIC(...) do { BOOST_ASSERT_MSG(false, __VA_ARGS__); UNREACHABLE_C; } while(0)
//Panics when reached, used to signal unreachable code
#define TODO { PANIC("Unimplemented."); }


#ifdef __cplusplus
#if (__cplusplus > 202302L)                 //C++ 23
#define ASSUME_C(cond,reason) do { [[assume((cond))]]; } while (0)
#define UNREACHABLE_C std::unreachable()
#else
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
#define ASSUME_C(cond,reason) do { __assume((cond)); } while (0)
#define UNREACHABLE_C __assume(0)
#else
#ifdef __GNUC__                              // GCC, Clang
#define ASSUME_C(cond,reason) do { if (!(cond)) __builtin_unreachable(); } while (0)
#define UNREACHABLE_C __builtin_unreachable()
#else                                       //Fallback - No op
#define ASSUME_C(cond,reason)
#define UNREACHABLE_C
#endif
#endif
#endif
#endif

#ifdef __cplusplus
#if (__cplusplus > 202302L)
#define IF_C if constexpr
#else
#define IF_C if
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
void CASES_FUNCTION(boost::source_location const& loc, T check_value, R cases_list) {
    ASSUME(check_value == cases_list, ("Cases not matched! Source location: " + loc.to_string()).c_str());
}

template<typename T, typename Q, typename S>
void CASES_FUNCTION(boost::source_location const& loc, T check_value, Q first_case, S second_case) {
    if (check_value != first_case) {
        CASES_FUNCTION(loc, check_value, second_case);
    }
    ASSUME(check_value == first_case or check_value == second_case, "Cases must be satisfied!");
}

template<typename T, typename Q, typename S, typename... R>
void CASES_FUNCTION(boost::source_location const& loc, T check_value, Q first_case, S second_case, R... cases_list) {
    if (check_value != first_case) {
        CASES_FUNCTION(loc, check_value, second_case, cases_list...);
    }
    //ASSUME(check_value == first_case or check_value == second_case or (check_value == cases_list...), "Cases must be satisfied!");
}

template<typename... Q>
void CASES_TRUE_FUNCTION(boost::source_location const& loc, Q... cases) {
    CASES_FUNCTION(loc, true, cases...);
}


#define CASES(check_value,...) CASES_FUNCTION((BOOST_CURRENT_LOCATION), (check_value), __VA_ARGS__)
#define CASES_TRUE(...) CASES_TRUE_FUNCTION((BOOST_CURRENT_LOCATION), __VA_ARGS__)


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


//Type aliases
//A multiprecision integer from boost

//__SLOW_BIGNUM: Use debug adaptor (shows the value as string in debuggers)
//__FAST_BIGNUM: Use bignums but no adaptor
//If neither: Use Int64s
#define __SLOW_BIGNUM
#ifdef __SLOW_BIGNUM
using number = boost::multiprecision::number<boost::multiprecision::debug_adaptor<typename boost::multiprecision::cpp_int::backend_type>, boost::multiprecision::cpp_int::et>;
std::string to_string(number x) { return x.str(); }
#else
#ifdef __FAST_BIGNUM
using number = boost::multiprecision::cpp_int;
std::string to_string(number x) { return x.str(); }
#else
using number = signed long long;
std::string to_string(number x) { return std::to_string(x); }
#endif
#endif

//An "any" type from boost
//Must model reasonably pure functions, values eg (x == x, y = x => y == x, f(y) == f(x) is true) etc
//Not expected to store eg "auto-iterating" iterators or multithreaded values
//Must be copy constructible, equality, includes typeid_ support
using any = boost::type_erasure::any<boost::mpl::vector<boost::type_erasure::copy_constructible<>, boost::type_erasure::typeid_<>, boost::type_erasure::equality_comparable<>, boost::type_erasure::relaxed>>;
//Reference-safe type_id from C++ std
using std::type_index;


//A class to hold pointer info, for GenerationalPointers
class PointerMaps {
public:
    static std::unordered_map<void*, uint64_t> generations;
    static std::unordered_map<void*, uint64_t> domains;
    static std::unordered_map<void*, uint64_t> domain_Generations;
    template<typename T>
    friend class GenerationalPointer;
};

std::unordered_map<void*, uint64_t> PointerMaps::generations;
std::unordered_map<void*, uint64_t> PointerMaps::domains;
std::unordered_map<void*, uint64_t> PointerMaps::domain_Generations;

//A single-threaded generational pointer class
//Stores a pointer and a generation number;References a static map that tracks the generation for each allocation
//Verifies the correct generation number on each deallocation
//Has a reuse mechanism
//Template parameters: (Default = no checks)
//Pointed-to type T
template<typename T>
class GenerationalPointer {
private:
    //Which allocation generation this address belongs to
    uint64_t Gen = 0;
    //The actual pointer itself
    T* raw_ptr = nullptr;
    //A domain associated to the pointer, used to track memory leaks
    uint64_t domainID = 0;
    
public:
    //A < operator for inserting into ordered sets etc
    friend bool operator<(GenerationalPointer lhs, GenerationalPointer rhs){
        return lhs.raw_ptr < rhs.raw_ptr;
    }
    
    //Retrieve the number of pointers within a given domain
    static size_t CheckDomain(uint64_t domain) {
        size_t count = std::count_if(PointerMaps::domains.begin(), PointerMaps::domains.end(), [=](const auto& e) {
            return e.second == domain;
        });
        return count;
    }

    //Create a new GenerationalPointer and perform all the associations necessary
    template<typename... Q>
    static GenerationalPointer MakeNewGenerationalT(uint64_t domain = 0, Q... q...) {
        GenerationalPointer GP;
        GP.raw_ptr = getRaw(std::forward(q)...);
        if (PointerMaps::generations.find(GP.raw_ptr) != PointerMaps::generations.end())
            PointerMaps::generations.at(GP.raw_ptr)++;
        else
            PointerMaps::generations[GP.raw_ptr] = 1;
        GP.Gen = PointerMaps::generations[GP.raw_ptr];
        AssociateDomain(GP, domain);
        GP.domainID = domain;
        ASSERT_RUN(PanicCheckQuick(GP));
        return GP;
    }

    //Verify that the generation is valid, return bool
    static bool CheckGeneration(GenerationalPointer GP) {
        bool found = (PointerMaps::generations.find(GP.raw_ptr) != PointerMaps::generations.end());
        if (not found)
            return false;
        bool check = (GP.Gen == PointerMaps::generations.at(GP.raw_ptr));
        if (found and check)
            return true;
        else
            return false;
    }

    //Panic if the generation check does not return true
    static void PanicCheckGeneration(GenerationalPointer GP) {
        if (CheckGeneration(GP) == false)
            PANIC("Generation mismatch on PanicCheck!");
        else
            return;
    }

    //Delete a pointer, add it to the "freelist" if enabled
    static void DeleteNewGenerationalT(GenerationalPointer GP) {
        bool EraseResult = TryErase(GP);
        if (EraseResult == false)
            PANIC("Memory Error: Generation mismatch on deletion.");
    }

    //Try to erase if the generation checks pass, return true on success
    static bool TryErase(GenerationalPointer GP) {
        bool Check = CheckGeneration(GP);
        if (Check) {
            DeleteRaw(GP);
            DeassociateDomain(GP);
            return true;
        }
        else if(not Check) {
            return false;
        }
        UNREACHABLE("Check or not check");
    }

    //Delete the raw pointer
    static void DeleteRaw(GenerationalPointer GP) {
        delete GP.raw_ptr;
        PointerMaps::generations[GP.raw_ptr]++;
    }

    T& operator*() {
        PanicCheckGeneration(*this);
        return *raw_ptr;
    }

private:
    //Given a GenerationalPointer and a domain, insert the GenerationalPointer into the domain
    static void AssociateDomain(GenerationalPointer GP, uint64_t domain) {
        PointerMaps::domains.insert({ GP.raw_ptr, domain });
    }

    //Given a GenerationalPointer and a domain, mark the pointer as associated
    static void DeassociateDomain(GenerationalPointer GP) {
        PointerMaps::domains.erase(GP.raw_ptr);
    }

    template<typename... Q>
    static T* getRaw(Q... q) {
            return new T(std::forward(q)...);
    }
};



//Start



//Types used in Interpreter:
//A tribool type (true, false, unknown)
class IComparisonResult;

//A polymorphic type that can store most C++ values at runtime
//Must store "reasonably pure" functions, values etc
class IAny;

//A container for polymorphic types, can model vectors/arrays/sets/etc
//Modelled as vector underneath (remembers type and ordering)
class IMany;



//A class that stores the data of an interpreter variable
class IVariable;

//A class that represents an *interpreter variable type
class IType;

//A "typed" tag type (Assign and compare equals only)
//Type is used to validate comparisons
class ITag;


//A Macrolist, which is a separation of any list into different (far-apart) sections
//API: Prepare new, append, lock, produce
//Cannot append after lock
class IMacroList;


//Variable types:
//Variables are mutable by default
//Declare immutable scoped variables for use in static asserts

//A tag used to indicate empty values
//Used like a "skip" value in Tuples etc
class Empty;

//An "indeterminate" bigint
//Represents some unspecified integer (can be restricted later)
//Used to model impurity
//Ops must account for all cases
//Is collapsed in a specific value/arbitrary value on compilation
// Specific value: for eg User inputs; arbitrary value: for 
//TODO: Dehardcode wrt theories, describe as indeterminate(type) with type computed from theories
// eg Unknown(Int) A; A * 2 == Unknown(Int:x{x = 2 * Int y})
class Indeterminate;

//Tuples, for storing many (unknown) ints at once
//Have infinite size, default constructs to zero (Use types on lengths to enforce sizes)
//Supports Lexicographic comparison and specific element set/extraction
class Tuple;

//Pure functions, guaranteed to terminate, must be provably total
//Accepts and returns Tuples and other Functions
//2-Arg Ackermann function:
//A(0, n) = n + 1
//A(m + 1, 0) = A(m, 1)
//A(m + 1, n + 1) = A(m, A(m + 1, n))
//For proof of termination, use lexicographic comparison of (m, n) to show that it always decreases
//Use a special tag to indicate empty
//Built from hardcoded values, composition of "smaller" functions (enforced totality), iota, argument redirection
class Function;

//Theories, modelling type invariants and operations
//Fundamental units are static asserts, like types in F* (all types are theories)
//Only function on upto countably infinite sequences of bigintegers
//Models fully pure restrictions, can create and use theorems
//Rapidly create theories from static asserts
//Use theories to model hardware and other non-software constraints (networks etc)
class Theory;

//Sequences, modelling different streams of time
//Different sequences represent different points in time
//Define before/after, enforce DAG
//Define DAGs, use Contraction of cyclic graphs and Topological ordering when constructing graphs and ordering on unordered graphs
//Convert to Theories, encode as a graph
//Use try-fail to enforce topological ordering
class Sequence;


#ifdef __ENABLE_ASSERTS
#undef __ENABLE_ASSERTS
#endif
#define __ENABLE_ASSERTS 0

//A tribool type
//Supports true, false and unknown
//Supports asserting conversions to bool (asserts on Unknown)
//Default construction to unknown
class IComparisonResult {
public:
    //Default construct: set to unknown
    IComparisonResult() : _isTrue(false), _isFilled(false) {
        ASSUME(isUnknown(), "Default constructed IComparisonResults must be unknown.");
    }
    //Value construct: set to a bool
    IComparisonResult(const bool value) : _isTrue(value), _isFilled(true) {
        ASSUME(_isFilled == true, "Value constructed IComparisonResults must be known.");
        ASSUME(_isTrue == value, "Value constructed IComparisonResults must be constructed to correct value.");
    }
    //Set to a bool value, is not unknown after being set
    void setBool(const bool value) {
        _isTrue = value;
        _isFilled = true;

        ASSUME(_isFilled == true, "Must construct to known value.");
        ASSUME(_isTrue == value, "Must construct to specified value.");
    }
    //Set the value to true
    void setTrue() {
        setBool(true);

        ASSUME(_isFilled == true, "Must construct to known value.");
        ASSUME(_isTrue == true, "Must construct to known true.");
    }
    //Set the value to false
    void setFalse() {
        setBool(false);

        ASSUME(_isFilled == true, "Must construct to known value.");
        ASSUME(_isTrue == false, "Must construct to false.");
    }
    //Set to unknown
    void setUnknown() {
        _isFilled = false;

        ASSUME(_isFilled == false, "Must construct to unknown.");
    }
    //Check if it is not unknown
    bool isKnown() const {
        bool return_value = _isFilled;
        if (return_value == true) {
            ASSUME(_isFilled == true, "Must be a filled value.");
        }
        return return_value;
    }
    //Check if it is a specific bool (Return false if unknown)
    bool isBool(bool b) const {
        bool return_value = isKnown() && (b == _isTrue);
        if (return_value == true) {
            ASSUME(_isFilled == true, "isBool() should imply filled.");
        }
        return return_value;
    }
    //Check if it is true (Return false if unknown)
    bool isTrue() const {
        bool return_value = isKnown() && isBool(true);
        if (return_value == true) {
            ASSUME(_isFilled == true, "True should imply filled.");
        }
        return return_value;
    }
    //Check if it is false (Return false if unknown)
    bool isFalse() const {
        bool return_value = isKnown() && isBool(false);
        if (return_value == true) {
            ASSUME(_isFilled == true, "False should imply filled.");
        }
        return return_value;
    }
    //Check if it is unknown
    bool isUnknown() const {
        bool return_value = !isKnown();
        if (return_value == true) {
            ASSUME(_isFilled == false, "isUnknown() should not imply isKnown().");
        }
        return return_value;
    }
    //Get either the stored boolean, or a fallback value
    bool getBoolOr(const bool b) const { 
        bool return_value;
        if (isKnown())
            return_value = isTrue();
        else if (isUnknown())
            return_value = b;
        else
            UNREACHABLE("");
        return return_value;
    }
    //Get the stored boolean, or throw an error
    bool getBoolOrAssert(const std::string& error) const {
        ASSUME(isKnown(), ("Boolean must be known at this point: " + error).c_str());
        return isTrue();
    }

    //Get bool or assert
    bool operator+() const {
        return getBoolOrAssert("Conversion to bool operator called!");
    }

    //Compare with another IComparisonResult variable (propagate unknown)
    static IComparisonResult Compare(const IComparisonResult& lhs, const IComparisonResult& rhs) {
        if (lhs.isUnknown() or rhs.isUnknown())
            return ConstructUnknown();
        else if (+lhs == +rhs)
            return ConstructBool(true);
        else if (+lhs != +rhs)
            return ConstructBool(false);
        UNREACHABLE("lhs and rhs are unknown/same/different");
        return {};
    }

    //Construct a IComparisonResult from a bool
    static IComparisonResult ConstructBool(const bool b) {
        IComparisonResult return_value;
        return_value.setBool(b);

        ASSUME(return_value.isKnown() == true, "Constructed from a bool should be known!");
        ASSUME(return_value.isBool(b) == true, "Constructed from a bool should be the correct value!");
        
        return return_value;
    }
    //Construct a IComparisonResult to "Unknown"
    static IComparisonResult ConstructUnknown() {
        IComparisonResult return_value;
        return_value.setUnknown();

        ASSUME(return_value.isKnown() == false, "Constructed from unknown should not be known!");
        ASSUME(return_value.isTrue() == false, "Constructed from unknown should not be true!");
        ASSUME(return_value.isFalse() == false, "Constructed from unknown should not be false!");

        return return_value;
    }

    //The "negative" of the Comparison Result; true <-> false, unknown -> unknown
    IComparisonResult Negation() const {
        IComparisonResult return_value;
        if (isUnknown())
            return_value = ConstructUnknown();
        else if (isTrue())
            return_value = ConstructBool(false);
        else if (isFalse())
            return_value = ConstructBool(true);
        else
            UNREACHABLE("Either unknown, true or false");
        
        if (isKnown())
            ASSUME(+return_value == not this->getBoolOrAssert("Checked"), "Must be negated!");
        else if (isUnknown())
            ASSUME(return_value.isUnknown() == true, "Must propagate unknowns!");
        else
            UNREACHABLE("Either unknown or known");

        return return_value;
    }

    //Call a generic bool + bool -> bool function on the stored types
    static IComparisonResult WrapUnknownsOP(const IComparisonResult& lhs, const IComparisonResult& rhs, bool function(bool, bool)) {
        IComparisonResult return_value;
        ASSUME(function != nullptr, "Provided function must not be empty!");
        if (lhs.isUnknown() or rhs.isUnknown())
            return_value = ConstructUnknown();
        else if (lhs.isKnown() and rhs.isKnown())
            return_value = function(+lhs, +rhs);
        else
            UNREACHABLE("Either are unknown or both are known");

        return return_value;
    }

    friend std::ostream& operator<<(std::ostream&, const IComparisonResult&);

private:
    bool _isTrue;
    bool _isFilled;
};
bool operator==(const IComparisonResult& lhs, const IComparisonResult& rhs) {
    return +IComparisonResult::Compare(lhs, rhs);
}

#ifdef __ENABLE_ASSERTS
#undef __ENABLE_ASSERTS
#endif
#define __ENABLE_ASSERTS 0

//IComparisonResult that autoconverts to bool
//Asserts if unknown is implicitly converted
class IComparisonToBool {
public:
    operator bool() const { return Result.getBoolOrAssert("IComparisonToBool implicitly converted."); }
    operator IComparisonResult() const { return Result; }
    IComparisonToBool(const bool value) : Result(value) {}
    IComparisonToBool(const IComparisonResult value) : Result(value) {}

    IComparisonResult getIComparisonResult() const { return Result; }
private:
    IComparisonResult Result;
};

//Boolean operators, unknown-propagating
//Boolean negation
IComparisonToBool operator!(const IComparisonResult& operand) { 
    return operand.Negation();
}
IComparisonToBool operator!(const IComparisonToBool& operand) {
    return !operand.getIComparisonResult();
}
//Boolean and
IComparisonToBool operator&&(const IComparisonResult& lhs, const IComparisonResult& rhs) {
    return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a and b; });
}
IComparisonToBool operator&&(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
    return lhs.getIComparisonResult() and rhs.getIComparisonResult();
}

//Boolean or
IComparisonToBool operator||(const IComparisonResult& lhs, const IComparisonResult& rhs) {
    return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a or b; });
}
IComparisonToBool operator||(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
    return lhs.getIComparisonResult() or rhs.getIComparisonResult();
}
//Boolean xor
IComparisonToBool operator^(const IComparisonResult& lhs, const IComparisonResult& rhs) {
    return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a not_eq b; });
}
IComparisonToBool operator^(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
    return lhs.getIComparisonResult() xor rhs.getIComparisonResult();
}
//Boolean equals comparison
IComparisonToBool operator==(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
    return lhs.getIComparisonResult() == rhs.getIComparisonResult();
}

//Stream IComparisonResult to an ostream
std::ostream& operator<<(std::ostream& ostream, const IComparisonResult& result) {
    CASES_TRUE(result.isUnknown(), result.isTrue(), result.isFalse());
    if (result.isUnknown())
        return ostream << "unknown";
    else if(result.isTrue())
        return ostream << "true";
    else if (result.isFalse())
        return ostream << "false";
    UNREACHABLE("Unknown, true or false");
}


//Is the fallback class for IAny and is in default constructed IAny
//Always compares equal to and not-less-than to itself
class IEmptyValue {};
bool operator==(const IEmptyValue, const IEmptyValue) { return true; }
bool operator<(const IEmptyValue, const IEmptyValue) { return false; }

bool operator!=(const any x, const any y) { return !(x == y); }

//A class for storing any CPP type with runtime polymorphism
//Requires copy constructible, typeid, equality comparison and less-than comparison
class IAny {
public:
    //Get the type_index of the value stored
    type_index GetStoredType() const {
        return typeid_of(data);
    }

    //Check if a type is stored by type_info object
    IComparisonResult IsCPPType(const type_index& T) const {
        const type_index& stored_type = typeid_of(data);
        const type_index& requested_type = T;
        bool return_value;
        if (stored_type == requested_type)
            return_value = true;
        else if (stored_type != requested_type)
            return_value = false;
        else
            UNREACHABLE("Type matches or doesn't. No unknowns expected.");
        
        if (return_value == true)
            ASSUME(type_index(typeid_of(data)) == T, "Return value must equal stored type.");
        else if (return_value == false)
            ASSUME(type_index(typeid_of(data)) != T, "Return value true must imply stored type is requested type.");
        else
            UNREACHABLE("Type matches or doesn't. No unknowns expected.");

        return return_value;
    }

    //Check if a type is stored (template parameter version)
    template<typename T>
    IComparisonResult HasCPPType() const {
        IComparisonResult return_value = IsCPPType(typeid(T));

        if (return_value == true)
            ASSUME(typeid_of(data) == typeid(T), "Return value true must imply stored type is correct type.");
        else if (return_value == false)
            ASSUME(typeid_of(data) != typeid(T), "Return value false must imply stored type is not correct type.");
        else
            UNREACHABLE("Either matches or doesn't. Total, no unknowns expected.");

        return return_value;
    }

    //Extract either a stored value or an empty IAny
    any GetBoostAny() const {
        any return_value = IEmptyValue();
        if (typeid_of(data) != typeid(IEmptyValue))
            return_value = data;
        else if (typeid_of(data) == typeid(IEmptyValue))
            return_value = IEmptyValue();
        else
            UNREACHABLE("Either matches or doesn't. Total, no unknowns expected.");
        
        if (typeid_of(data) == typeid(IEmptyValue))
            ASSUME(return_value == any(IEmptyValue()), "Empty must be returned when Empty is stored.");
        if (typeid_of(data) != typeid(IEmptyValue))
            ASSUME(return_value == data, "Stored data must be returned when non-Empty is stored.");

        return return_value;
    }

    //Return a T if it is stored, or assert otherwise
    //Return type needs to be known at compile-time
    template<typename T>
    T GetCppType(std::string assert_message = "") const {
        if (assert_message == "") {
            assert_message = std::string("The wrong type is stored! The correct type is: ") + typeid_of(data).name();
        }
        if (typeid_of(data) != typeid(T)) {
            PANIC(assert_message.c_str());
        }
        try {
            T return_value = boost::type_erasure::any_cast<T>(data);
            ASSUME(return_value == boost::type_erasure::any_cast<T>(data), "Stored type must be returned!");
            return return_value;
        }
        catch (boost::type_erasure::bad_any_cast) {
            UNREACHABLE("Tested for Type-mismatch after test.");
            throw;
        }
        catch (...) {
            throw;
        }
        UNREACHABLE("Caught Type-mismatch after test. Propagated all other exceptions.");
    }

    //If it stores data of the same C++ Type
    static IComparisonResult isComparable(const IAny& lhs, const IAny& rhs) { 
        IComparisonResult return_value = (typeid_of(lhs.data) == typeid_of(rhs.data));

        if (return_value == true)
            ASSUME(typeid_of(lhs.data) == typeid_of(rhs.data), "Must only return true when both values are the same type.");
        else if (return_value == false)
            ASSUME(typeid_of(lhs.data) != typeid_of(rhs.data), "Must only return false when both values are different types.");
        else
            UNREACHABLE("Type matches or doesn't. No unknowns expected.");

        return return_value;
    }

    //Compare with another any if the same C++Type is stored, return false otherwise
    static IComparisonResult CompareOrFalse(const IAny& lhs, const IAny& rhs) {
        IComparisonResult areComparable = isComparable(lhs, rhs);
        IComparisonResult return_value;
        if (!areComparable)
            return_value = IComparisonResult::ConstructUnknown();
        else if (+areComparable)
            return_value = IComparisonResult::ConstructBool(lhs.data == rhs.data);
        else
            UNREACHABLE("Type matches or doesn't. No unknowns expected.");

        if (return_value.isUnknown())
            ASSUME((!isComparable(lhs, rhs)), "Only return unknown when not comparable!");
        else if (return_value.isTrue())
            ASSUME((lhs.data == rhs.data), "Only return true when equal!");
        else if (return_value.isFalse())
            ASSUME((lhs.data != rhs.data), "Only return false when unequal!");
        else
            UNREACHABLE("Unknown, true and false all checked.");
        
        return return_value;
    }

    //If it has the IEmptyValue value
    IComparisonResult isEmpty() const {
        IComparisonResult return_value = (typeid_of(data) == typeid(IEmptyValue));

        if (return_value.isUnknown())
            UNREACHABLE("Type matches or doesn't. No unknowns expected.");
        else if (return_value.isTrue())
            ASSUME(typeid_of(data) == typeid(IEmptyValue), "Only return true when empty is stored!");
        else if (return_value.isFalse())
            ASSUME(typeid_of(data) != typeid(IEmptyValue), "Only return false when empty is not stored!");
        else
            UNREACHABLE("Unknown, true and false all checked.");

        return return_value;
    }

    //Store a value into the IAny
    template<typename T>
    void storeAny(T t) {
        data = any(t);
    }

    //Default Constructor
    IAny() : data(IEmptyValue()) {};

    //Construct from any object, explicitly
    template<typename T>
    explicit IAny(T t) : data(t) {}

    //Construct an empty IAny
    static IAny ConstructEmpty() {
        return IAny(IEmptyValue());
    }

    ////Compare less than, sort objects of different type by their type's hash-code and same type by their intrinsic comparison
    //friend bool operator<(const IAny& lhs, const IAny& rhs) {
    //    IComparisonResult return_value;
    //    if (lhs.GetStoredType() != rhs.GetStoredType())
    //        return_value = (lhs.GetStoredType().hash_code() < rhs.GetStoredType().hash_code());
    //    else if (lhs.GetStoredType() == rhs.GetStoredType())
    //        return_value = (lhs.data < rhs.data);
    //    else
    //        UNREACHABLE("Type matches or doesn't. No unknowns expected.");

    //    if (return_value == true)
    //        CASES_TRUE(
    //            ((lhs.GetStoredType() == rhs.GetStoredType()) and (lhs.data < rhs.data)),
    //            ((lhs.GetStoredType() != rhs.GetStoredType()) and (lhs.GetStoredType().hash_code() < rhs.GetStoredType().hash_code())));
    //    else if (return_value == false)
    //        CASES_TRUE(
    //            ((lhs.GetStoredType() == rhs.GetStoredType()) and !(lhs.data < rhs.data)),
    //            ((lhs.GetStoredType() != rhs.GetStoredType()) and !(lhs.GetStoredType().hash_code() < rhs.GetStoredType().hash_code())));
    //    else
    //        UNREACHABLE("Unknown, true and false all checked.");

    //    return return_value.getBoolOrAssert("Must not be unknown");
    //}

private:
    //Stores the data of the IAny
    any data;
    friend struct std::hash<IAny>;
};
//Compare two IAnys for equality
bool operator==(const IAny& lhs, const IAny& rhs) { return +IAny::CompareOrFalse(lhs, rhs); }

template<>
class std::hash<IAny> {
public:
    size_t operator()(const IAny& operand) const {
        if (operand.GetStoredType() == typeid(int)) {
            return operand.GetCppType<size_t>();
        }
        if (operand.GetStoredType() == typeid(number)) {
            return (size_t)(operand.GetCppType<number>() % (1U << 31));
        }
        return operand.GetStoredType().hash_code();
    }
};

//An IAny wrapper that can be constructed from any type
class PolyAny {
public:
    IAny data;
    template<typename T>
    PolyAny(T t) : data(t){}
    operator IAny () {
        return data;
    }
};

//A polymorphic container of IAnys
//Implemented as a vector
//Supports more advanced compares (ignore ordering, ignore repeats etc)
//Support Map operations (Add an associative container)
class IMany {
public:

    //A function from an IAny to an IAny
    using MonoAnyFunction = std::function<auto (IAny)->IAny>;

    //A function from an IAny to an IVoid
    using MonoVoidFunction = std::function<auto (IAny)->void>;

    //A function that takes two IAnys and returns a bool
    using CompareAnyFunction = std::function<auto (const IAny&, const IAny&)->bool>;

    //A function that mutates an IAny in place
    using MonoAnyMutateFunction = std::function<auto (IAny&)->void>;

    //A function that takes an IAny by value, a list of optional other typed arguments (T...) and returns an R
    template<typename R, typename... T>
    using PolyAnyFunction = std::function<auto (IAny, T... t...)->R>;

    //A mutating function that takes an IAny by reference, a list of optional other typed arguments (T...) and returns an R
    template<typename R, typename... T>
    using PolyAnyMutateFunction = std::function<auto (IAny&, T... t)->R>;

    //A function that transforms indices
    using MonoIndexFunction = std::function<auto (number)->number>;

    //A function from IAny to bool
    using MonoBoolFunction = std::function<auto (IAny)->bool>;

    //Check if the two Manys are the same, with optional relaxations on the order, repeats, subset, superset and equivalence class
    //lhs and rhs are the operands
    //ordered describes whether the order matters for comparison (if false, different order may compare true)
    //no_repeats describes whether repeats matter for comparison (is false, [1] == [1, 1] == [1, 1, 1])
    //not_subset describes whether lhs cannot be a proper subset of the rhs (if false, [1] != [1, 2])
    //not_superset describes whether lhs cannot be a proper superset of the rhs (is false, [1, 2] != [1])
    //lhs_index describes the ordering of the lhs as lhs[lhs_index(0)], lhs[lhs_index(1)], ... (nullptr = no op)
    //rhs_index describes the ordering of the rhs as rhs[rhs_index(0)], rhs[rhs_index(1)], ... (nullptr = no op)
    //lhs_transform is an optional IAny->IAny transformation applied to each element of the lhs first (nullptr = no op)
    //rhs_transform is an optional IAny->IAny transformation applied to each element of the rhs first (nullptr = no op)
    static IComparisonResult CompareMany(const IMany& lhs, const IMany& rhs, bool ordered, bool no_lhs_repeats, bool no_rhs_repeats, bool not_subset, bool not_superset, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        if (ordered == true) {
            unsigned int i = 0, j = 0;
            for (i = 0, j = 0; i < lhs.container.size() and j < rhs.container.size();) {
                if (!IAny::CompareOrFalse(get_i_element(lhs, i, lhs_index, lhs_transform), get_i_element(rhs, j, rhs_index, rhs_transform))) {
                    return IComparisonResult::ConstructBool(false);
                }
                if (not no_lhs_repeats) {
                    while (i + 1 < lhs.container.size() and +IAny::CompareOrFalse(get_i_element(lhs, i, lhs_index, lhs_transform), get_i_element(lhs, i + 1, lhs_index, lhs_transform))) {
                        i++;
                    }
                }
                i++;
                if (not no_rhs_repeats) {
                    while (j + 1 < rhs.container.size() and +IAny::CompareOrFalse(get_i_element(rhs, j, rhs_index, rhs_transform), get_i_element(rhs, j + 1, rhs_index, rhs_transform))) {
                        j++;
                    }
                }
                j++;
            }
            CASES_TRUE(i == lhs.container.size(), j == rhs.container.size());
            if (not_subset == true) {
                if (i != lhs.container.size())
                    return IComparisonResult::ConstructBool(false);
            }
            if (not_superset == true) {
                if (j != rhs.container.size())
                    return IComparisonResult::ConstructBool(false);
            }
            return IComparisonResult::ConstructBool(true);
        }
        //[1, 2, 3] [1, 3, 2]
        TODO;
        UNREACHABLE("TODO");
    }

    //ordered x2, l/r hs_repeats x3, sub/superset x3 = 18

    static IAny get_i_element(const IMany& hs, number index, const MonoIndexFunction hs_index = nullptr, const MonoAnyFunction hs_transform = nullptr)
    {
        size_t new_index = (hs_index ? hs_index(index) : index).convert_to<size_t>();
        return hs_transform ? hs_transform(hs.container[new_index]) : hs.container[new_index];
    };

    class VectorIterateClass {
        IMany* target;
    public:
        auto begin() {
            return target->container.begin();
        }
        auto begin() const {
            return target->container.cbegin();
        }
        auto end() {
            return target->container.end();
        }
        auto end() const {
            return target->container.cend();
        }
        friend class IMany;
    };

    VectorIterateClass VectorIterate() {
        VectorIterateClass return_value;
        return_value.target = this;
        return return_value;
    }

    //true, true, true, true, true
    //Check if the two Manys have the same elements in the same order (1,2,3,4 == 1,2,3,4)
    //Check as if lhs and rhs are vectors
    static IComparisonResult CompareVector(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        return CompareMany(lhs, rhs, true, true, true, true, true, lhs_index, rhs_index, lhs_transform, rhs_transform);
    }
    //Check if the two Manys have the same elements, with arbitrary repeats in both sets, in the same order, rhs may have more elements after the end (1,2,3,4 == 1,2,3,4,5)
    //Check as if rhs is an extension of lhs, as vectors
    static IComparisonResult CompareVectorExtension(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        return CompareMany(lhs, rhs, true, false, false, true, true, lhs_index, rhs_index, lhs_transform, rhs_transform);
    }
    //Check if the two Manys have a (non-zero) common prefix
    static IComparisonResult CompareVectorPrefix(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        TODO;
    }
    //Check if the two Manys have a (non-zero) single common substring
    static IComparisonResult CompareVectorOverlap(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        TODO;
    }
    //Check if the two Manys have the same elements (with the same counts) in a possibly different order (1,2,3,4 == 1,3,2,4)
    //Check as if lhs and rhs are multisets
    static IComparisonResult CompareMultiset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        std::unordered_multiset<IAny> a;
        std::unordered_multiset<IAny> b;
        for (const auto& x : lhs.container) {
            a.insert(lhs_transform ? lhs_transform(x) : x);
        }
        for (const auto& x : rhs.container) {
            b.insert(rhs_transform ? rhs_transform(x) : x);
        }
        if (a == b)
            return IComparisonResult::ConstructBool(true);
        else if (a != b)
            return IComparisonResult::ConstructBool(false);
        else
            UNREACHABLE("Total comparison of true or false. No unknowns expected.");
    }
    //Check if the two Manys have the same elements, with arbitrary repeats, in any order (1,2,3,2,4 == 1,2,3,1,4 == 1,2,3,4)
    //Check as if lhs and rhs are sets
    static IComparisonResult CompareSet(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        std::unordered_set<IAny> a;
        std::unordered_set<IAny> b;
        for (const auto& x : lhs.container) {
            a.insert(lhs_transform ? lhs_transform(x) : x);
        }
        for (const auto& x : rhs.container) {
            b.insert(rhs_transform ? rhs_transform(x) : x);
        }
        if (a == b)
            return IComparisonResult::ConstructBool(true);
        else if (a != b)
            return IComparisonResult::ConstructBool(false);
        else
            UNREACHABLE("Total Comparison of true or false. No unknowns expected.");
    }
    //false, false, false, false, true
    //Check as if the lhs, as a Set, is contained in rhs (arbitrary repeats on both sides) (1,2,3,4 == 5,4,3,4,3,2,1)
    static IComparisonResult CompareSubset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        TODO;
    }
    //false, true, true, false, false
    //Check if the lhs set_intersection rhs has atleast 1 element (0,1,2,5 == 3,4,5,6) (1,2,3,4 != 5,6,7,8)
    static IComparisonResult CompareSetIntersection(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr) {
        TODO;
    }

    //Check if the rhs is a member of the lhs, if each member of the lhs was transformed
    static IComparisonResult IsMember(const IMany& lhs_container, const IAny& rhs_member, const MonoAnyFunction lhs_transform = nullptr) {
        for (const IAny& element : lhs_container.container) {
            const auto element_transformed = lhs_transform ? lhs_transform(element) : element;
            if (IAny::CompareOrFalse(element_transformed, rhs_member).getBoolOrAssert("Fallback called."))
                return IComparisonResult::ConstructBool(true);
        }
        return IComparisonResult::ConstructBool(false);
    }

    //Look for a member in a transformed range and return the index it was found at first
    //Return -1 if not found
    static size_t WhereMember(const IMany& lhs, const IAny& rhs_member, const MonoIndexFunction lhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr) {
        for (int i = 0; i < lhs.container.size(); i++) {
            if (get_i_element(lhs, i, lhs_index, lhs_transform) == rhs_member) {
                return i;
            }
        }
        return static_cast<size_t>(-1);
    }


    //Remove duplicate IAnys: Elements are transformed by the transform function (no op) as fallback
    //MonoAnyFunction defines equivalence classes, it does not mutate the final container
    //Eg ([1, 2, 3, 4, 5], is_odd) => ([1, 2])
    //   ([10, 20, 30, 40, 50], (mod 3)) => ([10, 20, 30])
    //The order is retained
    static IMany RemoveDuplicates(const IMany& container, const MonoAnyFunction transform = nullptr) {
        MonoAnyFunction transformer = transform ? transform : [](IAny a)->IAny { return a; };
        IMany buffer_container;
        std::unordered_set<IAny> seen_elements;

        for (const auto& x : container.container) {
            IAny y = transformer(x);
            if (seen_elements.count(y) == 0)
                buffer_container.InsertIAnyEnd(x);
            seen_elements.insert(y);
        }

        return buffer_container;
    }

    //Sort IManys: Elements are compared by the CompareAnyFunction
    //fallback is (differentiating within types is based on <, differentiating between types is somewhat random but same types compare equal)
    //Returns a copy
    //static IMany SortElements(const IMany& container, const CompareAnyFunction is_less = nullptr) {
    //    IMany return_value;
    //    return_value.container = container.container;
    //    auto compareAny = [](IAny& a, IAny& b, const CompareAnyFunction& is_less) -> bool {
    //        return is_less ? is_less(a, b) : a < b;
    //    };
    //    std::sort(return_value.container.begin(), return_value.container.end(), is_less);
    //    return return_value;
    //}

    //Lexicographically compare IManys:
    //Compare the elements of lhs and rhs pairwise, until lhs_element != rhs_element, return true if lhs_element < rhs_element
    //isSmallerLess describes which result will be returned if 1 is a strict subset of the other
    //isEqualOrStrictlyLess describes which result will be returned if lhs is equal to the rhs
    //static IComparisonResult IsLesserIMany(IMany& lhs, IMany& rhs, bool isSmallerLess, bool isEqualOrStrictlyLess, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr, const CompareAnyFunction is_less = nullptr) {
    //    int i = 0, j = 0;
    //    for (i = 0, j = 0; i < lhs.container.size() and j < rhs.container.size(); i++, j++) {
    //        IAny l = get_i_element(lhs, i, lhs_index, lhs_transform);
    //        IAny r = get_i_element(rhs, j, rhs_index, rhs_transform);
    //        IComparisonResult l_equal = (l == r);
    //        if (not l_equal.isTrue()) {
    //            IComparisonResult l_less = is_less ? is_less(l, r) : (l < r);
    //            if (+l_less)
    //                return IComparisonResult::ConstructBool(true);
    //            else if (not l_less)
    //                return IComparisonResult::ConstructBool(false);
    //            else
    //                UNREACHABLE("Total Comparison of lesser or not. No unknowns expected.");
    //        }
    //    }
    //    if (i < lhs.container.size() or j < rhs.container.size())
    //        return IComparisonResult::ConstructBool(isSmallerLess);
    //    HEAVY_ASSERT(1, +CompareVector(lhs, rhs, lhs_index, rhs_index, lhs_transform, rhs_transform), "Must be same to reach this point!");
    //    return IComparisonResult::ConstructBool(isEqualOrStrictlyLess);
    //}

    //Insert an IAny into the end of the vector
    void InsertIAnyEnd(IAny element) {
        size_t size = container.size();

        container.push_back(element);

        ASSUME(container.size() == size + 1, "Must have added an element!");
        ASSUME(container.back() == element, "Must have pushed element at the end!");
    }

    //Insert any T into the vector by converting to IAny
    template<typename T>
    void InsertTypeEnd(T element) {
        size_t size = container.size();

        container.emplace_back(element);

        ASSUME(container.size() == size + 1, "Must have added an element!");
        ASSUME(container.back() == IAny(element), "Must have pushed element at the end!");
        ASSUME(container.back().GetStoredType() == typeid(T), "Must have a T stored last!");
    }

    //Insert an element-wise transformation of another IMany at the end of this one (nullptr for no transform)
    void InsertManyEnd(const IMany& container_rhs, const MonoAnyFunction rhs_transform = nullptr) {
        for (const IAny& element : container_rhs.container) {
            const auto element_transformed = rhs_transform ? rhs_transform(element) : element;
            InsertIAnyEnd(element_transformed);
        }
    }

    //Transform each element of an IMany elementwise (nullptr for no transform)
    //Returns a copy
    IMany TransformMap(const MonoAnyFunction transform = nullptr) const {
        IMany result;
        for (const IAny& element : container) {
            const auto element_transformed = transform ? transform(element) : element;
            result.InsertIAnyEnd(element_transformed);
        }
        return result;
    }

    //Run a nullary transformation on each element of an IMany (nullptr for no transform)
    void OperateElement(const MonoVoidFunction operate = nullptr) const {
        if (operate) {
            for (const IAny& element : container) {
                operate(element);
            }
        }
    }

    //Run a mutable transformation on each element of an IMany (nullptr for no transform)
    //R is the result type for the function
    //T... are the template parameter types
    template<typename R, typename... T>
    void MutateElement(const PolyAnyMutateFunction<R, T...> operate = nullptr, T... t...) {
        if (operate) {
            for (IAny& element : container) {
                operate(element, t...);
            }
        }
    }

    void clear() {
        container.clear();
    }

    //Set Union (Preserves order)
    static IMany Union(const IMany& lhs, const IMany& rhs) {
        TODO;
    }
    //Set Intersection (Preserves order)
    static IMany Intersection(const IMany& lhs, const IMany& rhs) {
        TODO;
    }
    //Set Difference (Preserves order)
    static IMany Difference(const IMany& lhs, const IMany& rhs) {
        TODO;
    }
    //Set size = number of unique elements
    size_t SizeSet() const {
        TODO;
    }
    //Vector size = number of inserted elements
    size_t SizeVector() const {
        return container.size();
    }
    //Removes all IAny = a from the vector/set 
    void RemoveIf(const MonoBoolFunction predicate) {
        TODO;
    }
    //Return an unspecified element
    IAny pick() const {
        if (container.size() == 0) {
            return IAny{};
        }
        return container.back();
    }
    //Return an unspecified element and remove it (and all copies of it)
    IAny popSet() {
        TODO;
    }
    //Return an unspecified element and remove it (retain other copies)
    IAny popVector() {
        TODO;
    }
    //Retrieves ith element from vector
    //Index according to indexing function and transform the returned result
    //Return IEmptyValue if it is out of scope
    IAny nthElementVector(const number index, const MonoIndexFunction indexing = nullptr, const MonoAnyFunction transform = nullptr) const {
        number k = indexing ? indexing(index) : index;
        ASSERT(k < (1ULL << 63), "Must be small!");
        size_t k_conv = boost::multiprecision::integer_modulus(k, 1ULL << 63);
        ASSERT((number)k_conv == k, "Must convert better!");
        if (k_conv < container.size())
            return IAny::ConstructEmpty();
        IAny result = container[k_conv];
        IAny return_value = transform ? transform(result) : result;
        return return_value;
    }
    //Retrieves ith element from set (ignore previously seen transformed values)
    // eg ([1, 2, 3, 4, 5] mod 3 = [1, 2, 0]) index 2 = 0, index 3 = not found
    //Index according to indexing function and transform the returned result
    IAny nthElementSet(const number index, const MonoIndexFunction indexing = nullptr, const MonoAnyFunction transform = nullptr) const {
        std::unordered_set<IAny> seen;
        int index2 = 0;
        for (int i = 0; i < container.size() && index2 < index; i++) {
            size_t k = (indexing ? indexing(i) : i).convert_to<size_t>();
            if (k >= container.size())
                return IAny::ConstructEmpty();
            IAny result = container[k];
            IAny return_value = transform ? transform(result) : result;
            if (seen.count(return_value) == 0) {
                index2++;
                seen.insert(return_value);
            }
        }
        if (index2 < index)
            return IAny::ConstructEmpty();
        size_t k = (indexing ? indexing(index2) : index2).convert_to<size_t>();
        if (k >= container.size())
            return IAny::ConstructEmpty();
        IAny result = container[k];
        IAny return_value = transform ? transform(result) : result;
        return return_value;
    }

    template<typename T>
    void AssertAllType(std::string message) const {
        for (const IAny& element : container) {
            ASSERT(element.HasCPPType<T>(), message.c_str());
        }
        return;
    }

    ////Iterators
    ////Forward mutable iterator to first element
    //auto begin() -> std::vector<IAny>::iterator {
    //    return container.begin();
    //}
    ////Backward mutable iterator to first element
    //auto rbegin() -> std::reverse_iterator<std::vector<IAny>::iterator> {
    //    return container.rbegin();
    //}
    ////Forward constant iterator to first element
    //auto begin() const -> std::vector<IAny>::const_iterator {
    //    return container.cbegin();
    //}
    ////Backward constant iterator to first element
    //auto rbegin() const -> std::reverse_iterator<std::vector<IAny>::const_iterator> {
    //    return container.crbegin();
    //}
    ////Forward mutable iterator to last element
    //auto end() -> std::vector<IAny>::iterator {
    //    return container.end();
    //}
    ////Backward mutable iterator to last element
    //auto rend() -> std::reverse_iterator<std::vector<IAny>::iterator> {
    //    return container.rend();
    //}
    ////Forward constant iterator to last element
    //auto end() const -> std::vector<IAny>::const_iterator {
    //    return container.cend();
    //}
    ////Backward constant iterator to last element
    //auto rend() const -> std::reverse_iterator<std::vector<IAny>::const_iterator> {
    //    return container.crend();
    //}
    friend class PolyMany;

private:
    std::vector<IAny> container;
};

class PolyMany {
    IMany data;
public:

    //Construct from a heterogeneous list
    PolyMany(std::initializer_list<PolyAny> l) {
        for (auto&& x : l) {
            data.container.push_back(x.data);
        }
    }

    operator IMany() {
        return data;
    }
};

//Builds IMany, with construct, append, lock and produce stages
//Assert on violation
class IMacroList {
public:
    //Build a new IMacroList
    static IMacroList Construct() { return IMacroList(); }
    //Append an IAny to the IMacroList (Must be unlocked)
    void Append(const IAny& A) {
        ASSUME(not _lock, "Append to locked IMacroList");
        prepared.InsertIAnyEnd(A);            
    }
    template<typename T>
    //Append an IAny to the IMacroList (Must be unlocked)
    void Append(const T& t) {
        ASSUME(not _lock, "Append to locked IMacroList");
        prepared.InsertTypeEnd(t);
    }
    //Append an IMany to the IMacroList (Must be unlocked)
    void AppendMany(const IMany& A) {
        ASSUME(not _lock, "Append to locked IMacroList");
        prepared.InsertManyEnd(A);
    }
    //Lock the IMacroList, cannot append and must produce (Must be unlocked)
    void Lock() {
        ASSUME(not _lock, "Attempt to lock locked IMacroList");
        _lock = true;
    }
    //Produce the IMany (Must NOT be unlocked)
    IMany Prepare() {
        ASSUME(_lock, "Prepare from an unlocked IMacroList");
        return prepared;
    }

private:
    IMany prepared;
    bool _lock = false;
};



//An indeterminate value of IType T
//Can be collapsed specifically or automatically
class Indeterminate {};


//A list of bigints
//Set/get
//BigInts: Math ops, comparison ops
//Lexicographic comparisons but no "pointwise" comparisons
//No "procedural" values (ie no infinite a[x] = x)
//Allow variable indexing (ie a[x] = y) (bc it can be implemented with switch-cases) (Indexing by a tuple is a[b] := a[b[0]] only)
class Tuple {
public:

    //Construct from a single number
    //Provided number is stored at the 0 index
    Tuple(number a) { GetNumber_Index(0) = a; }

    //Construct from another Tuple
    Tuple() { data[0] = 0; };
    Tuple(const Tuple&) = default;
    Tuple(Tuple&&) = default;
    Tuple& operator=(const Tuple&) = default;
    Tuple& operator=(Tuple&&) = default;

    //Lexicographically compare two Tuples
    //Returns true for a < b; false for a > b, weak for a == b
    template<bool weak = false>
    static IComparisonResult isLess(const Tuple& a, const Tuple& b) {
        if (+isEqual(a, b))
            return weak;

        Tuple startTa = getMaxHeight(a);
        Tuple startTb = getMaxHeight(b);
        Tuple endTa = getMinHeight(a);
        Tuple endTb = getMinHeight(b);

        //If a is zero, check the lead digit of b
        if (+isZero(a)) {
            number base = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
            if (base > 0)
                return true;
            else if (base < 0)
                return false;
            else if (base == 0)
                UNREACHABLE("Lead digit should not be zero!");
        }

        //If b is zero, check the lead digit of a
        if (+isZero(b)) {
            number base = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
            if (base > 0)
                return false;
            else if (base < 0)
                return true;
            else if (base == 0)
                UNREACHABLE("Lead digit should not be zero!");
        }
        ASSERT((not isZero(a)) and (not isZero(b)), "a and b should not be zero by this point!");

        //Get the number with the bigger "height"
        if (startTa.GetNumberConst_Index(0) > startTb.GetNumberConst_Index(0))
            return false;
        else if (startTa.GetNumberConst_Index(0) < startTb.GetNumberConst_Index(0))
            return true;

        //If they have the same height, compare the lead digit
        number a_lead = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
        number b_lead = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
        number a_tail = a.GetNumberConst_Index(endTa.GetNumberConst_Index(0));
        number b_tail = a.GetNumberConst_Index(endTb.GetNumberConst_Index(0));

        if (a_lead > b_lead) {
            return false;
        }
        else if (b_lead > a_lead) {
            return true;
        }
        else if (a_lead == b_lead) {
            number index = startTa.GetNumberConst_Index(0);
            while (index --> std::max(a_tail, b_tail)) {
                number a_indexed = a.GetNumberConst_Index(index);
                number b_indexed = b.GetNumberConst_Index(index);
                if (a_indexed > b_indexed) {
                    return false;
                }
                else if (a_indexed < b_indexed) {
                    return true;
                }
                else if (a_indexed == b_indexed) {
                    continue;
                }
            }
            UNREACHABLE("Equality should be checked early!");
        }
        UNREACHABLE("No more cases left!");
    }

    //Set a tuple to a constant number
    static void setIndividualNumber(Tuple& a, number value) {
        a = Tuple(value);
    }

    std::string to_str() {
        std::string s = "[";
        for (const std::pair<number, number>& entry : data) {
            s += to_string(entry.first);
            s += ": ";
            s += to_string(entry.second);
            s += ", ";
        }
        if (s.size() > 1)
            s.resize(s.size() - 2);
        s += "]";
        return s;
    }

    //Are they equal?
    friend bool operator==(const Tuple& a, const Tuple& b) {
        return +Tuple::isEqual(a, b);
    }

    //Is a < b? 
    friend bool operator<(const Tuple& a, const Tuple& b) {
        return +Tuple::isLess<false>(a, b);
    }

    //Is it zero?
    static IComparisonResult isZero(const Tuple& a) {
        for (const std::pair<number, number>& x : a.data) {
            if (x.second != 0) {
                return false;
            }
        }
        return true;
    }

    //Are they fully equal?
    static bool isEqual(const Tuple& a, const Tuple& b) {
        for (const std::pair<number, number>& x : a.data) {
            if (b.GetNumberConst_Index(x.first) != x.second) {
                return false;
            }
        }
        for (const std::pair<number, number>& x : b.data) {
            if (a.GetNumberConst_Index(x.first) != x.second) {
                return false;
            }
        }
        return true;
    }

    //T[0], T[1]
    static Tuple ConstructPair(number a, number b) {
        Tuple T;
        T.GetNumber_Index(0) = a;
        T.GetNumber_Index(1) = b;
        return T;
    }

    std::map<number, number> data;

    using BinaryMathFunction = auto(*)(number, number)->number;
    using UnaryMathFunction = auto(*)(number)->number;

    //Get a reference to the number at an index
    number& GetNumber_Index(const number& index) {
        if (data.find(index) == data.end()) {
            data.insert_or_assign(index, 0);
        }
        return data.at(index);
    }

    //Get the number at an index (no reference)
    number GetNumberConst_Index(const number& index) const {
        if (data.find(index) == data.end()) {
            return 0;
        }
        else {
            return data.at(index);
        }
    }

    //Return the maximum height for the tuple (The highest index with a non-zero value)
    //Returns (max, 0) as the max height if found, (0, 1) if not
    static Tuple getMaxHeight(const Tuple& a) {
        number max = 0;
        bool found = false;
        for (const std::pair<number, number>& entry : a.data) {
            if (entry.second != 0) {
                found = true;
                max = std::max(max, entry.first);
            }
        }
        if (found)
            return Tuple::ConstructPair(max, 0);
        else if (not found)
            return Tuple::ConstructPair(0, 1);
        else
            UNREACHABLE("Either found or not found");
    }

    //Return the minimum height for the tuple (The lowest index with a non-zero value)
    //Returns (0, min) as the min height if found, (1, _) if not
    static Tuple getMinHeight(const Tuple& a) {
        number min = 0;
        bool found = false;
        for (const std::pair<number, number>& entry : a.data) {
            if (entry.second != 0) {
                found = true;
                min = std::min(min, entry.first);
            }
        }
        if (found)
            return Tuple::ConstructPair(min, 0);
        else if (not found)
            return Tuple::ConstructPair(0, 1);
        else
            UNREACHABLE("Either found or not found. No unknowns expected.");
    }

    friend bool operator!=(const Tuple& a, const Tuple& b) {
        return not Tuple::isEqual(a, b);
    }

    //static auto neg(number a) -> number { return -a; };

    //static auto add(number a, number b) -> number { return a + b; };
    //static auto sub(number a, number b) -> number { return a - b; };
    //static auto mul(number a, number b) -> number { return a * b; };
    //static auto div(number a, number b) -> number { if (b == 0) return 0; else return a / b; };
    //static auto mod(number a, number b) -> number { if (b == 0) return a; else return a - (b * (a / b)); };

    friend class Function;
};

//A tag class for sequence points
class SequencePoint {
    number id;
    friend class  Sequence;
    friend struct std::hash<SequencePoint>;
public:
    bool operator==(const SequencePoint& other) const
    {
        return id == other.id;
    }
    bool operator!=(const SequencePoint& other) const
    {
        return !(*this == other);
    }
};
template<>
struct std::hash<SequencePoint> {
    size_t operator()(const SequencePoint& other) const
    {
        return size_t(other.id % (1ULL << 63));
    }
};

//A set of sequence-points, which are temporally ordered by a single "less than" partial order
//Irreflexive, antisymmetric, transitive
//Add points with a before relation
//Have potentially circular graphs, enforce numerical orderings and locks to assert on circularity
// Locked sequences cannot be further attached to or later unlocked
//TODO: Use tags and domains to enforce numerical locks (ie tag an event A with 4, when adding B after C (and C after A) assert that B's value > 4)
//TODO: Lock a domain permanently, no more inputs into it, and/or no more outputs from it
class Sequence {
public:
    //Start a new sequencepoint, with nothing pointing to it
    SequencePoint newSequencePointStart() {
        SequencePoint SP;
        SP.id = ++totalSequencePoints;
        return SP;
    }

    //Associate 2 preexisting sequence points (assert if already has an after)
    void insertNewLink(SequencePoint before, SequencePoint after) {
        ASSUME(not isInBeforeAfter(before), "Should not reassign already assigned SequencePoint!");
        ASSUME(not isInSelectionNodes(before), "Should not reassign already assigned SequencePoint!");
        beforeAfter.insert({ before, after });
    }

    //Create a new sequence point, associate it to "before" and return the new point
    SequencePoint newSequencePointFollower(SequencePoint before) {
        SequencePoint after = newSequencePointStart();
        insertNewLink(before, after);
        return after;
    }

    //Insert a new sequence point with many follows
    SequencePoint newSequencePointChoice(SequencePoint before, IMany sp) {
        SequencePoint after = newSequencePointStart();
        insertNewLink(before, after);
        selectionNodes.insert({ after, sp });
        return after;
    }

    //Follow a single sequence point
    //If no follower, return EmptyAny
    //Otherwise, if simple, return the next point
    //Otherwise, if comnplex, return the zeroth choice
    IAny followSequencePointSimple(SequencePoint before) {
        if (not isInBeforeAfter(before))
            return {};
        else if (isInBeforeAfter(before)) {
            SequencePoint after = beforeAfter.at(before);
            return IAny{ after };
        }
        UNREACHABLE("Cases covered!");
    }

    IAny followSequencePointChoice(SequencePoint before, number choice) {
        if (not isInSelectionNodes(before))
            return IAny{};
        IMany choices = selectionNodes[before];
        choices.AssertAllType<SequencePoint>("Must all be SequencePoints");
        if (choices.SizeVector() < choice)
            return IAny{};
        SequencePoint result = choices.nthElementVector(choice).GetCppType<SequencePoint>("Expected SequencePoint");
        return IAny{ result };
    }

    //Follow a choice point, by selecting a choice number (0 -> 0th choice, 1 -> 1st choice etc)
    //If the point is not a choice point, only 0 is a valid choice (simple follow)
    //If the index is invalid, return EmptyAny
    IAny followSequencePointAggregate(SequencePoint before, number choice) {
        if (choice < 0)
            return IAny{};

        if (not isInBeforeAfter(before) and not isInSelectionNodes(before))
            return IAny{};
        
        
        CASES_TRUE(isInBeforeAfter(before), isInSelectionNodes(before));
        if (isInBeforeAfter(before)) {
            if (choice > 0)
                return IAny{};
            return followSequencePointSimple(before);
        }
        else if (isInSelectionNodes(before)) {
            if (selectionNodes[before].SizeVector() < choice)
                return IAny{};

        }
    }

private:
    //A single-key multi-value map from after-points to before-points
    std::unordered_map<SequencePoint, SequencePoint> beforeAfter;
    //A list of nodes with multiple outputs
    std::unordered_map<SequencePoint, IMany> selectionNodes;
    //A number associated to each ordering, can be used to enforce far-apart before/after relations
    std::unordered_map<SequencePoint, number> orderingAssertNumber;
    static number totalSequencePoints;

    //If beforeAfter has a "before"
    bool isInBeforeAfter(SequencePoint before) const {
        return not (beforeAfter.contains(before));
    }

    //If selectionNodes has a "before"
    bool isInSelectionNodes(SequencePoint before) const {
        return not (selectionNodes.contains(before));
    }
};

number Sequence::totalSequencePoints = 0;

//Describe Tuple->Tuple operations
///Operations: Lexicographically bounded recursion
///Iota function, pointwise add/subtract/multiply/division/indexing etc, index get/set, partial specialization, argument rearrangement
///Fully Immutable once built
///Atoms := (Finite) Constant values, Pointwise Identity function, Pointwise +-*/, Argument redirection+Total Piecewise
/// TODO: Add "induction variable" for defining Bounded Recursion (Decrement highest number by 1 towards zero, change others freely)
/// TODO: Remove overredundant primitives
class Function {
    //Nodes:
    //Constant values
    //Pointwise Identity = Argument Redirection, pointwise +-*/[]
    //Total piecewise based on < or > or == (ie if-else)
    //Dilate/expand: Manipulate indexing of R by a linear amount
    enum class NodeType {
        Uninitialized,      //Panic if called
        Constant,           //Return a constant value
        PointwiseIdentity,  //Return the input unchanged
        PointwisePlus,      //Return R[i] := a[i] + b[i]
        PointwiseMinus,     //Return R[i] := a[i] - b[i]
        PointwiseProduct,   //Return R[i] := a[i] * b[i]
        PointwiseQuotient,  //Return R[i] := a[i] / b[i]    (x /0 = 0)
        PointwiseRemainder, //Return R[i] := a[i] % b[i]    (x %0 = x)
        PointwiseLess,      //Return R[i] := a[i] < d[0] ? b[i] : c[i]
        PiecewiseLess,      //Return R[i] := i < c[0] ? a[i] : b[i]
        LexicographicLess,  //Return R    := a L< b ? c : d
        GetIndex,           //Return R[a[0]] = b[0]
        SetIndex,           //Return R[a[0]] = b[0]
        Borrow,             //Return Function[y] : y[-Infinity:-1] = 0, 0 L< y L< x
    };

    static std::unordered_map<number, NodeType> FunctionTypes;
    static std::unordered_map<number, IMany> FunctionArguments;
    static std::unordered_map<std::pair<Function, Tuple>, Tuple, boost::hash<std::pair<Function, Tuple>>> cache;
    static std::unordered_map<number, std::string> names;

public:
    IMany& FunctionNodes() {
        return FunctionArguments.at(id);
    }

    NodeType& NodeClass() {
        return FunctionTypes.at(id);
    }

    std::string& name() {
        return names.at(id);
    }

    const IMany& FunctionNodes() const {
        return FunctionArguments.at(id);
    }

    const NodeType& NodeClass() const {
        return FunctionTypes.at(id);
    }

    const std::string& name() const {
        return names.at(id);
    }

    Function() {
        id = ++max_id;
        FunctionTypes.insert({ id, NodeType::Uninitialized });
        FunctionArguments.insert({ id, IMany() });
        names.insert({ id, "" });
    }

    //Returns R: R[i] = T[i]
    //Ignores variable and returns T
    void SetConstant(Tuple T) {
        NodeClass() = Function::NodeType::Constant;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(T);
    }

    void SetIdentity() {
        NodeClass() = Function::NodeType::PointwiseIdentity;
        FunctionNodes().clear();
    }

    void SetAddFunction(Function lhs, Function rhs) {
        NodeClass() = Function::NodeType::PointwisePlus;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(lhs);
        FunctionNodes().InsertTypeEnd(rhs);
    }

    void SetSubtractFunction(Function lhs, Function rhs) {
        NodeClass() = Function::NodeType::PointwiseMinus;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(lhs);
        FunctionNodes().InsertTypeEnd(rhs);
    }

    void SetProductFunction(Function lhs, Function rhs) {
        NodeClass() = Function::NodeType::PointwiseProduct;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(lhs);
        FunctionNodes().InsertTypeEnd(rhs);
    }

    void SetQuotientFunction(Function lhs, Function rhs) {
        NodeClass() = Function::NodeType::PointwiseQuotient;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(lhs);
        FunctionNodes().InsertTypeEnd(rhs);
    }

    void SetRemainderFunction(Function lhs, Function rhs) {
        NodeClass() = Function::NodeType::PointwiseRemainder;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(lhs);
        FunctionNodes().InsertTypeEnd(rhs);
    }

    void SetBorrow(Function shrink, Tuple basecase, std::shared_ptr<Function> top) {
        NodeClass() = Function::NodeType::Borrow;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(top);
        FunctionNodes().InsertTypeEnd(shrink);
        FunctionNodes().InsertTypeEnd(basecase);
    }

    //Returns R[i] := check[i] < threshold[i] ? ifLess[i] : ifEqualOrMore[i]
    void SetPointwiseLess(Function check, Function threshold, Function ifLess, Function ifEqualOrMore) {
        NodeClass() = Function::NodeType::PointwiseLess;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(check);
        FunctionNodes().InsertTypeEnd(ifLess);
        FunctionNodes().InsertTypeEnd(ifEqualOrMore);
        FunctionNodes().InsertTypeEnd(threshold);
    }

    //Returns R[-Infinity:threshold[0] - 1] = left[i], R[threshold, Infinity] = right[i]
    void SetPiecewiseLess(Function threshold, Function leftExclusive, Function rightInclusive) {
        NodeClass() = Function::NodeType::PiecewiseLess;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(leftExclusive);
        FunctionNodes().InsertTypeEnd(rightInclusive);
        FunctionNodes().InsertTypeEnd(threshold);
    }

    //Compares from most-significant element to least significant element, like a dictionary
    //If a[i] != b[i], early exit
    //Returns a < b ? ifLess : ifNotLess
    void SetLexicographicLess(Function a, Function b, Function ifLess, Function ifNotLess) {
        NodeClass() = Function::NodeType::LexicographicLess;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(a);
        FunctionNodes().InsertTypeEnd(b);
        FunctionNodes().InsertTypeEnd(ifLess);
        FunctionNodes().InsertTypeEnd(ifNotLess);
    }

    //R[a[0]] = b[0]
    void SetSetIndex(Function a, Function b) {
        NodeClass() = Function::NodeType::SetIndex;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(a);
        FunctionNodes().InsertTypeEnd(b);
    }

    //R[0] = b[a[0]]
    void SetGetIndex(Function a, Function b) {
        NodeClass() = Function::NodeType::GetIndex;
        FunctionNodes().clear();
        FunctionNodes().InsertTypeEnd(a);
        FunctionNodes().InsertTypeEnd(b);
    }

    //Call the function associated with this node
    //Retrieve the functions/operands from within the FunctionNodes()
    Tuple dispatchThis(Tuple input) {
        CASES(NodeClass(), 
            NodeType::Uninitialized,
            NodeType::Constant,
            NodeType::PointwiseIdentity,
            NodeType::PointwisePlus,
            NodeType::PointwiseMinus,
            NodeType::PointwiseProduct,
            NodeType::PointwiseQuotient,
            NodeType::PointwiseRemainder,
            NodeType::PointwiseLess,
            NodeType::PiecewiseLess,
            NodeType::GetIndex,
            NodeType::SetIndex,
            NodeType::Borrow,
            NodeType::LexicographicLess
            );
        Tuple output;
        if (cache.find({ *this, input }) != cache.end()) {
            return cache.at({ *this, input });
        }

        switch (NodeClass()) {
        case NodeType::Uninitialized: {
            PANIC("Execute an unitialized function!");
            UNREACHABLE("Function should be initialized!");
            break;
        }
        case NodeType::Constant: {
            ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
            Tuple target = FunctionNodes().pick().GetCppType<Tuple>("FunctionNode should store Tuple!");
            output = EvaluateConstant(target);
            break;
        }
        case NodeType::PointwiseIdentity: {
            ASSUME(FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
            output = EvaluatePointwiseIdentity(input);
            break;
        }
        case NodeType::PointwisePlus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwisePlus(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseMinus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwiseMinus(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseProduct: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwiseProduct(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseQuotient: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple    lhs_t = lhs_f(input);
            Tuple    rhs_t = rhs_f(input);
            output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseRemainder: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple    lhs_t = lhs_f(input);
            Tuple    rhs_t = rhs_f(input);
            output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = IMany::get_i_element(FunctionNodes(), 3).GetCppType<Function>("Fourth element must be a Function!");
            Tuple    ahs_t = ahs_f(input);
            Tuple    bhs_t = bhs_f(input);
            Tuple    chs_t = chs_f(input);
            Tuple    dhs_t = dhs_f(input);
            output = EvaluatePointwiseLess(ahs_t, bhs_t, chs_t, dhs_t);
            break;
        }
        case NodeType::PiecewiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            Tuple    ahs_t = ahs_f(input);
            Tuple    bhs_t = bhs_f(input);
            Tuple    chs_t = chs_f(input);
            output = EvaluatePiecewiseLess(ahs_t, bhs_t, chs_t.GetNumberConst_Index(0));
            break;
        }
        case NodeType::GetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple ahs_t    = ahs_f(input);
            Tuple bhs_t    = bhs_f(input);
            output = EvaluateGetIndex(ahs_t, bhs_t);
            break;
        }
        case NodeType::SetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Tuple ahs_t    = ahs_f(input);
            Tuple bhs_t    = bhs_f(input);
            output = EvaluateSetIndex(ahs_t, bhs_t);
            break;
        }
        case NodeType::Borrow: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode Induct must store 3 elements!");
            Function* top       = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function*>("[0] element must be a Function Pointer! (Top)");
            Function shrink     = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("[1] element must be a Function! (shrink)");
            Tuple basecase      = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
            output = EvaluateBorrow(input, shrink, basecase, top);
            break;
        }
        case NodeType::LexicographicLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = IMany::get_i_element(FunctionNodes(), 3).GetCppType<Function>("Fourth element must be a Function!");
            Tuple ahs_t = ahs_f(input);
            Tuple bhs_t = bhs_f(input);
            Tuple chs_t = chs_f(input);
            Tuple dhs_t = dhs_f(input);
            output = EvaluateLexicographicLess(ahs_t, bhs_t, chs_t, dhs_t);
            break;
        }
        default:
            UNREACHABLE("All cases should be covered.");
            PANIC("Default Case!");
        }
        if (cache.find({ *this, input }) == cache.end()) {
            cache.insert({ { *this, input }, output});
        }
        return output;
    }

    //Returns a constant tuple
    static Tuple EvaluateConstant(Tuple target) {
        Tuple result = target;
        return result;
    };

    //Returns the input unchanged
    static Tuple EvaluatePointwiseIdentity(Tuple input) {
        return input;
    };

    //Returns the "pointwise" sum of both inputs
    static Tuple EvaluatePointwisePlus(Tuple input_a, Tuple input_b) {
        std::unordered_set<number> storedIndices;
        Tuple result;
        for (const auto& index : input_a.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : input_b.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : storedIndices) {
            result.data[index] = input_a.GetNumberConst_Index(index) + input_b.GetNumberConst_Index(index);
        }
        return result;
    };

    //Returns the "pointwise" difference of both inputs
    static Tuple EvaluatePointwiseMinus(Tuple input_a, Tuple input_b) {
        std::unordered_set<number> storedIndices;
        Tuple result;
        for (const auto& index : input_a.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : input_b.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : storedIndices) {
            result.data[index] = input_a.GetNumberConst_Index(index) - input_b.GetNumberConst_Index(index);
        }
        return result;
    };

    //Returns the "pointwise" product of both inputs
    static Tuple EvaluatePointwiseProduct(Tuple input_a, Tuple input_b) {
        std::unordered_set<number> storedIndices;
        Tuple result;
        for (const auto& index : input_a.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : input_b.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : storedIndices) {
            result.data[index] = input_a.GetNumberConst_Index(index) * input_b.GetNumberConst_Index(index);
        }
        return result;
    };

    //Returns the "pointwise" quotient of both inputs
    // x/0 == 0
    static Tuple EvaluatePointwiseQuotient(Tuple input_a, Tuple input_b) {
        std::unordered_set<number> storedIndices;
        Tuple result;
        for (const auto& index : input_a.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : input_b.data) {
            storedIndices.insert(index.first);
        }
        for (const auto& index : storedIndices) {
            number a = input_a.GetNumberConst_Index(index);
            number b = input_b.GetNumberConst_Index(index);
            number c = 0;
            if (b == 0)
                c = 0;
            else
                c = (a / b);
            result.data[index] = c;
        }
        return result;
    };
    
    //R[i] := a[i] < d[i] ? b[i] : c[i]
    static Tuple EvaluatePointwiseLess(Tuple input_a, Tuple input_b, Tuple input_c, Tuple input_d) {
        Tuple result;
        std::unordered_set<number> shared_indices;
        for (const auto& i : input_a.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_b.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_c.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& index : shared_indices) {
            number a_val = input_a.GetNumberConst_Index(index);
            number b_val = input_b.GetNumberConst_Index(index);
            number c_val = input_c.GetNumberConst_Index(index);
            number d_val = input_d.GetNumberConst_Index(index);
            if (a_val < d_val) {
                result.data[index] = b_val;
            }
            else if (a_val >= d_val) {
                result.data[index] = c_val;
            }
        }
        return result;
    }

    //R[-Infinity:threshold - 1] = a[i], R[threshold, Infinity] = b[i]
    static Tuple EvaluatePiecewiseLess(Tuple input_a, Tuple input_b, number threshold) {
        Tuple result;
        std::unordered_set<number> shared_indices;
        for (const auto& i : input_a.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_b.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& index : shared_indices) {
            number a_val = input_a.GetNumberConst_Index(index);
            number b_val = input_b.GetNumberConst_Index(index);
            if (index < threshold) {
                result.data[index] = a_val;
            }
            else if (index >= threshold) {
                result.data[index] = b_val;
            }
        }
        return result;
    }

    //R[i] := a[i] == threshold ? b[i] : c[i]
    static Tuple EvaluatePiecewiseEqual(Tuple input_a, Tuple input_b, number threshold) {
        Tuple result;
        std::unordered_set<number> shared_indices;
        for (const auto& i : input_a.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_b.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& index : shared_indices) {
            number a_val = input_a.GetNumberConst_Index(index);
            number b_val = input_b.GetNumberConst_Index(index);
            if (index == threshold) {
                result.data[index] = a_val;
            }
            else if (index != threshold) {
                result.data[index] = b_val;
            }
        }
        return result;
    }

    //R[i] := a[i] == threshold ? b[i] : c[i]
    static Tuple EvaluatePointwiseEqual(Tuple input_a, Tuple input_b, Tuple input_c, number threshold) {
        Tuple result;
        std::unordered_set<number> shared_indices;
        for (const auto& i : input_a.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_b.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& i : input_c.data) {
            shared_indices.insert(i.first);
        }
        for (const auto& index : shared_indices) {
            number a_val = input_a.GetNumberConst_Index(index);
            number b_val = input_b.GetNumberConst_Index(index);
            number c_val = input_c.GetNumberConst_Index(index);
            if (a_val == threshold) {
                result.data[index] = b_val;
            }
            else if (a_val != threshold) {
                result.data[index] = c_val;
            }
        }
        return result;
    }

    //R[a[0]] = b[0]
    static Tuple EvaluateSetIndex(Tuple input_a, Tuple input_b) {
        Tuple result;
        number index = input_a.GetNumberConst_Index(0);
        result.GetNumber_Index(index) = input_b.GetNumberConst_Index(0);
        return result;
    }

    //R[0] = b[a[0]]
    static Tuple EvaluateGetIndex(Tuple input_a, Tuple input_b) {
        Tuple result;
        number index = input_a.GetNumberConst_Index(0);
        result.GetNumber_Index(0) = input_b.GetNumberConst_Index(index);
        return result;
    }

    Tuple operator()(Tuple input) {
        return dispatchThis(input);
    }

    //Takes four parameters: Input, Recursive call argument generator, basecase, and full-function
    //Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
    //If shrunk operand is L< 0 or has any element at -1, -2, ... or has any element < 0; return the basecase
    //Return the result of applying the "top" function to the shrunk value
    //If top returns the "true" function, can be used to create recursive calls
    //eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
    static Tuple EvaluateBorrow(Tuple Op1, Function shrinker, Tuple basecase, Function* top) {
        if (+Tuple::isZero(Op1)) {
            return basecase;
        }
        Tuple shrunk1 = shrinker(Op1);
        Tuple shrunk1Left = EvaluatePiecewiseLess(shrunk1, number(0), number(0));
        bool isLeftNotEmpty = not + Tuple::isZero(shrunk1Left);
        bool isNegative = +Tuple::isLess(shrunk1, number(0));
        bool isNotSmaller = not +Tuple::isLess(shrunk1, Op1);
        bool hasAnyNegative = not +Tuple::isZero(Function::EvaluatePointwiseLess(shrunk1, Tuple(), shrunk1, Tuple()));
        if (isLeftNotEmpty or isNegative or isNotSmaller or hasAnyNegative) {
            return basecase;
        }
        return (*top)(shrunk1);
    }

    //If lessOp1 < lessOp2, return ifLess else ifNotLess
    static Tuple EvaluateLexicographicLess(Tuple lessOp1, Tuple lessOp2, Tuple ifLess, Tuple ifNotLess) {
        IComparisonResult x = Tuple::isLess(lessOp1, lessOp2);
        ASSUME(x.isKnown(), "Tuple::isLess should be complete!");
        if (+x) {
            return ifLess;
        }
        else if (!x) {
            return ifNotLess;
        }
        else
            UNREACHABLE("Either less or not less.");
    }

    std::string print_Function() {
        if (name() != "")
            return name();
        switch (NodeClass())
        {
        case NodeType::Constant: {
            ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
            Tuple target = FunctionNodes().pick().GetCppType<Tuple>("FunctionNode should store Tuple!");
            return target.to_str();
        }
        case NodeType::PointwiseIdentity: {
            ASSUME(FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
            return "x";
        }
        case NodeType::PointwisePlus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(") + lhs_f.print_Function() + ") + (" + rhs_f.print_Function() + ")";
        }
        case NodeType::PointwiseMinus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(") + lhs_f.print_Function() + ") - (" + rhs_f.print_Function() + ")";
        }
        case NodeType::PointwiseProduct: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(") + lhs_f.print_Function() + ") * (" + rhs_f.print_Function() + ")";
        }
        case NodeType::PointwiseQuotient: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(") + lhs_f.print_Function() + ") / (" + rhs_f.print_Function() + ")";
        }
        case NodeType::PointwiseRemainder: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
            Function lhs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(") + lhs_f.print_Function() + ") % (" + rhs_f.print_Function() + ")";
        }
        case NodeType::PointwiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = IMany::get_i_element(FunctionNodes(), 3).GetCppType<Function>("Fourth element must be a Function!");
            return std::string("(") + ahs_f.print_Function() + "[i]) < (" + bhs_f.print_Function() + "[i]) ? (" + chs_f.print_Function() + "[i]) : (" + dhs_f.print_Function() + "[i])";
        }
        case NodeType::PiecewiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            return std::string("(i) < (") + ahs_f.print_Function() + "[0]) ? (" + bhs_f.print_Function() + "[i]) : (" + chs_f.print_Function() + "[i])";
        }
        case NodeType::GetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("[0: ") + bhs_f.print_Function() + "[" + ahs_f.print_Function() + "[0]]]";
        }
        case NodeType::SetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            return std::string("(i) == (") + ahs_f.print_Function() + "[0]) ? (" + bhs_f.print_Function() + "[i]) : (0)";
        }
        case NodeType::Borrow: {
            //Change, replace with "independent improper functions"
            PANIC("Disimplement!");
            Function* top = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function*>("[0] element must be a Function! (Top)");
            Function shrink = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("[1] element must be a Function! (shrink)");
            Tuple basecase = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
            std::string top_name = (top->name() == "" ? "top" : top->name());
            return top_name + "(" + shrink.print_Function() + ")";
        }
        case NodeType::LexicographicLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
            Function ahs_f = IMany::get_i_element(FunctionNodes(), 0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = IMany::get_i_element(FunctionNodes(), 1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = IMany::get_i_element(FunctionNodes(), 2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = IMany::get_i_element(FunctionNodes(), 3).GetCppType<Function>("Fourth element must be a Function!");
            return std::string("(") + ahs_f.print_Function() + ") L< (" + bhs_f.print_Function() + ") ? (" + chs_f.print_Function() + ") : (" + dhs_f.print_Function() + ")";
        }
        default:
            UNREACHABLE("All cases should be covered.");
            PANIC("Default Case!");
        }
    }

    //Structural equality
    friend bool operator==(const Function& a, const Function& b) {
        if (a.NodeClass() != b.NodeClass()) {
            return false;
        }
        if (not IMany::CompareVector(a.FunctionNodes(), b.FunctionNodes())) {
            return false;
        }
        else {
            return true;
        }
    }

    number id;
    static number max_id;
};

namespace std {
    template <> struct hash<Tuple>
    {
        size_t operator()(const Tuple& x) const
        {
            return (size_t)boost::multiprecision::abs(((x.GetNumberConst_Index(0)) ^ x.GetNumberConst_Index(1)) % (1ULL << 31));
        }
    };

    template <> struct hash<Function>
    {
        size_t operator()(const Function& x) const
        {
            return (size_t)boost::multiprecision::abs((x.id) % (1UL << 31));
        }
    };
}

size_t hash_value(std::pair<Function, Tuple> x) {
    return std::hash<Function>()(x.first) ^ std::hash<Tuple>()(x.second);
}

number Function::max_id = 0;
std::unordered_map<number, Function::NodeType> Function::FunctionTypes;
std::unordered_map<number, IMany> Function::FunctionArguments;
std::unordered_map<std::pair<Function, Tuple>, Tuple, boost::hash<std::pair<Function, Tuple>>> Function::cache;
std::unordered_map<number, std::string> Function::names;

//Construct a single tuple from many tuples
//a, b, c, d -> a[-1], b[-1], c[-1], d[-1], 0: sizeof(a, b, c, d), 1: a[0], b[0], c[0], d[0], a[1], b[1], c[1], d[1], ...
Tuple storeManyTuples(const IMany a) {
    const size_t size = a.SizeVector();
    number maxHeight = 0;
    number minHeight = 0;
    Tuple return_value;
    for (int i = 0; i < size; i++) {
        Tuple getTuple = a.nthElementVector(i).GetCppType<Tuple>("Must have tuples!");
        number currentHeight    = Tuple::getMaxHeight(getTuple).GetNumberConst_Index(0);
        number currentLowHeight = Tuple::getMinHeight(getTuple).GetNumberConst_Index(0);
        maxHeight = maxHeight > currentHeight    ? currentHeight    : maxHeight;
        minHeight = minHeight > currentLowHeight ? currentLowHeight : minHeight;
    }
    //Returns the index for tuple id, limb index
    //if index<0,  size*index + id
    //if index>=0, size*index + id + 1
    auto mapIndex = [&size](number id, number index) -> number {
        if (index < 0)
            return size * index + id;
        else
            return size * index + id + 1;
    };
    for (size_t tuple_id = 0; tuple_id < size; tuple_id++) {
        Tuple getTuple = a.nthElementVector(tuple_id).GetCppType<Tuple>("Must have tuples!");
        for (number i = minHeight; i <= maxHeight; i++) {
            return_value.GetNumber_Index(mapIndex(i, tuple_id)) = getTuple.GetNumberConst_Index(i);
        }
    }
    return return_value;
}

//Retrieve many tuples from a store
IMany retrieveManyTuples(const Tuple& a) {
    const number size_n = a.GetNumberConst_Index(0);
    if (size_n < 0 or size_n >= SIZE_MAX) {
        //Invalid
        return IMany{};
    }
    //Number of tuples
    const size_t      size = size_n.convert_to<size_t>();
    const number minHeight = Tuple::getMinHeight(a).GetNumberConst_Index(0);
    const number maxHeight = Tuple::getMaxHeight(a).GetNumberConst_Index(0);
    std::vector<Tuple> return_vector(size);
    //Returns the index for tuple id, limb index
    //if index<0,  size*index + id
    //if index>=0, size*index + id + 1
    auto mapIndex = [&size](number id) -> std::pair<number, number> {
        if (id < 0) {
            number index = (id / size);
            number id_out = (id % size + size) % size;
            ASSERT(size * index + id_out == id, "Must reverse cleanly!");
            return std::pair<number, number>(index, id_out);
        }
        else if (id >= 0) {
            number index = ((id - 1) / size);
            number id_out = ((id - 1) % size + size) % size;
            ASSERT(size * index + id_out + 1 == id, "Must reverse cleanly!");
            return std::pair<number, number>(index, id_out);
        }
        UNREACHABLE("Should be covered!");
    };
    for (number index = minHeight; index <= maxHeight; index++) {
        std::pair<number, number> index_pair = mapIndex(index);
        number nTh = index_pair.second;
        number nsIndex = index_pair.first;
        return_vector[nTh.convert_to<size_t>()].GetNumber_Index(nsIndex) = a.GetNumberConst_Index(nTh);
    }
    IMany returnValue;
    for (const auto& x : return_vector) {
        returnValue.InsertTypeEnd(x);
    }
    return returnValue;
}

//Convert a pair of Tuples into a single Tuple
//Map input values to index>=0 and ghost values to index<0
Tuple addGhostTuple(Tuple input, Tuple ghost) {
    Tuple result;
    Tuple input_maxHeight = Tuple::getMaxHeight(input);
    Tuple input_minHeight = Tuple::getMinHeight(input);
    Tuple ghost_maxHeight = Tuple::getMaxHeight(ghost);
    Tuple ghost_minHeight = Tuple::getMinHeight(ghost);
    number input_min = 0;
    number input_max = 0;
    number ghost_min = 0;
    number ghost_max = 0;

    if (input_maxHeight.data[1] == 0) {
        input_max = input_maxHeight.data[0];
    }
    if (input_minHeight.data[1] == 0) {
        input_min = input_minHeight.data[0];
    }
    if (ghost_maxHeight.data[1] == 0) {
        ghost_max = ghost_maxHeight.data[0];
    }
    if (ghost_minHeight.data[1] == 0) {
        ghost_min = ghost_minHeight.data[0];
    }
    TODO;
}

//A sequence with associated code
class SequencedExecution {
public:
    Sequence Flow;
    IMany TupleIntermediates;
    using TuplesIndex = number;
    std::unordered_map<SequencePoint, number> AssignLHS;
    std::unordered_map<SequencePoint, Function> AssignRHS;
    
    //Builds a sequenced execution corresponding to a single function call+assign
    static SequencedExecution BuildSingleStatement(Function F) {
        //Create a new SE
        SequencedExecution SE;
        //Set its Flow to a single sequence point
        SequencePoint SP = SE.Flow.newSequencePointStart();
        //Create a single TupleVariable, set its id to zero
        number FirstTuple = 0;
        SE.AssignLHS[SP] = FirstTuple;
        SE.AssignRHS[SP] = F;
        SE.TupleIntermediates.clear();
        return SE;
    }


};



//A static assert over a tuple of integers
//Describe equality, set-theoretic notions
//Describe specifications with theories like N, Z, etc
//Describe mid-end code with restrictions on Tuples and prove equivalence to specifications like N, Z, etc
//Use proved equivalences to prove further equivalences
/*
* Theory example:
* Construction of N:
* 0 is in N
* succ N: n is in N
* n < succ n
* <: irreflexive, antisymmetric, transitive
* =: a = b iff !(a < b) and !(b < a)
* +: a + 0 = a, succ(a) + b = succ(a + b)
* -: c = a + b => c - a = b; c - b = a
* *: a * 0 = 0; a * (b + c) = (a * b) + (a * c) = (b + c) * a
* /: d = q * a + r, 0 < r < a => d / a = q, d % a = r
*
Implementation:

For proofs:
Use unevaluated function calls
Transform specific subexpressions by following a general, explicit rule
For verifying proofs, verify that the expressions transform correctly with only the general rules specified used
Within a single proof, different variables have the same name
The allowed transformations are decided by user specification

//Basic theory logic: GST-like
a{b} is a valid expression for all a, b with possibly zero admissions (Describes a theory a, constrained such that b accepts a)
x == y iff for all a, b a{x} == b{y}    (if all members admitted by x are also admitted by y, x = y)
x{a}{b}             (is the set of x that is accepted by a, and also by b)
z.cases(a, b{c})    (Exists a z that accepts any theory, and all members of another theory)
Implication: N exists
Include a calculus for a single limit n->Infinity for the entire program
Include a calculus for describing the other theories, with single-valued encoding



//Define equality (reflexive, symmetric, transitive)
new Theory Equality(lhs, rhs): {
    Equality.addCase(lhs, lhs);
    Equality.addCase(lhs, rhs)(rhs, lhs{=rhs});
    Equality.addCase(lhs, rhs)(lhs, mhs{=lhs}, rhs{=mhs});
    Equality.closeCases();
}

//Define N as a theory that is modelled by two concepts: Zero and Successor
//Use Peano axioms
new Theory N(Zero, Successor): {
    N.cases(Zero, Successor(n))(n{N});                          //N accepts Zero and Successor(n) where N accepts n (only!)
    N.Equality.addCase(Equality);                               //N1 == N2 if modelled by equality (not only!)
    N.Equality.addCase(c, d)(a{N}{=Successor(c)}, b{N}{=Successor(d)}{=a});   //S(a) == S(b) implies a == b
    N.Equality.closeCases();
    Successor(n{N}) != Zero;                                    //No n{N} such that S(n) is 0
}
Sugar: 0 = Zero, 1 = S(Zero), 2 = S(1), ...

new Theory Add;                //N is still in scope
Add(a{N}, N.Zero) = a;         //Add may also accept eg Q or R; so add takes N instead of N takes add
Add(a{N}, N.Successor(b{N})) = N.Successor(Add(a, b));   //a + 0 = a; a + S(b) = S(a + b);
Sugar: a + b = Add[a, b]

new Proof N.cases.independent:  //N's cases are independent, can be applied in any order
//Prove by demonstrating that case1 != case2, case1 != case3, case2 != case3, ...
N.cases = Zero, Successor(n{N});
a := Zero;
b := Successor(n{N});
Successor(n{N}) != Zero;        //By axiom
b != Zero;
N.cases = a{=Zero}, b{!=Zero};  //a, b, ...



TODO: Prove induction
Idea: Prove that the set of all x for which f(x) must be satisfied is 0 and its successive closure; and that it models N
TODO: Prove, for all x{N} and n{N}, Less accepts (n, n + 1 + x)
Prove for all x{N} and n{N}, Less rejects (n + 1 + x, n)






*/




//Takes a number "depth" and returns a function
//When depth is independent of function input, is equivalent to a proper function and can implement recursion
//When depth can depend on function input, this can result in "tuples" with infinitely many non-zero elements, including eg f(i) = i
//Specific action for limit depth->infinity
//Function nodes include number_input and induct
//Induct describes f(x+1) as something(f(x))
//Can be called with "depth" and "input" as inputs, can be partially specialized into f(depth) to return a function


//Optimization:
//Propagate constants fully
//Monomorphize all function calls   f(int a, int b) -> f(3, b) etc
//Monomorphize all variables (eg int a, b; -> int1 a; int2 b;)
//Strengthen all theories from the end  f(int a, int b): int c := a + b -> f(int a, int b): int c(a + b)
//Delay all computation "production" until "consumption" in the sequence

//Purify pass:
//The "full type" of a function is the equivalence class of all functions that produce the same output given the same input
//Strength reduction to for(i = ...; i != ...; i++) { /*No i*/; }
//Eliminate all bounded recursion with monomorphization (ie f(3, a) = f(2, b) + c becomes f_3(a) = f_2(b) + c)
//  Transform to non-recursive and with basecases
//Merge all unbounded loops program-wide into a single unbounded loop with a single inductor+local variables
//  (ie convert generate(x): co_return next x; generate(y): co_return next y; generate(z): generate(x) generate(y) co_return next z;
//      to
//      generate(z): x++; y++; co_return next z;)

//Search pass: (Opposites)
//Monomorphize function calls <-> deduplicate ("template-ize") function calls
//Inline function calls <-> extract function calls
//Cache vs "functionize" pure variables
//eg
//  a = b + c
//  f(a)
// to/from
//  f(b + c)

//Builds a Function from C++ code
class FunctionBuilder {
public:

    //Tag type
    struct Input {};

    enum class FunctionBuilderNodeTypes {
        Uninitialized,          //Error on usage
        ConstantTerminal,       //Represent a Constant Tuple
        VariableTerminal,       //Represent a Variable
        FunctionPlus,           //Build expression a + b
        FunctionMinus,          //Build expression a - b
        FunctionProduct,        //Build expression a * b
        FunctionQuotient,       //Build expression a / b
        FunctionMod,            //Build expression a % b
        FunctionPointwiseLess,     //Build expression a[i] < b[i] ? c[i] : d[i]
        FunctionPiecewiseLess,     //Build expression i < a[0] ? b[i] : c[i]
        FunctionLexicographicLess, //Build expression a L< b ? c : d
        FunctionGetIndex,          //Build Result[0] = a[b[0]]
        FunctionSetIndex,       //Build Result[a[0]] = b[0]
        FunctionBorrow,         //Build recursive call
        BorrowFromNode,         //The node that is borrowed from
        CallFunctionNode,       //Calls a predefined function
    } FunctionBuilderNode;

    IMany data;

    static bool isNotUninitialized(FunctionBuilder S) {
        if (S.FunctionBuilderNode == FunctionBuilderNodeTypes::Uninitialized)
            return not true;
        else if (S.FunctionBuilderNode != FunctionBuilderNodeTypes::Uninitialized)
            return not false;
        UNREACHABLE("True or not true");
        return {};
    }

    FunctionBuilder() {
        FunctionBuilderNode = FunctionBuilderNodeTypes::Uninitialized;
    }

    FunctionBuilder(Tuple T) {
        FunctionBuilderNode = FunctionBuilderNodeTypes::ConstantTerminal;
        data.InsertTypeEnd(T);
    }

    FunctionBuilder(number n) {
        FunctionBuilderNode = FunctionBuilderNodeTypes::ConstantTerminal;
        data.InsertTypeEnd(Tuple(n));
    }

    FunctionBuilder(Function F) {
        FunctionBuilderNode = FunctionBuilderNodeTypes::CallFunctionNode;
        data.InsertTypeEnd(F);
    }

    template<std::integral Int>
    FunctionBuilder(Int i) {
        FunctionBuilderNode = FunctionBuilderNodeTypes::ConstantTerminal;
        data.InsertTypeEnd(Tuple(i));
    }

    static FunctionBuilder Variable() {
        FunctionBuilder SB;
        SB.FunctionBuilderNode = FunctionBuilderNodeTypes::VariableTerminal;
        SB.data.clear();
        return SB;
    }

    static FunctionBuilder BorrowFrom() {
        FunctionBuilder SB;
        SB.FunctionBuilderNode = FunctionBuilderNodeTypes::BorrowFromNode;
        SB.data.clear();
        return SB;
    }

    FunctionBuilder operator+(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionPlus;
        ASSUME(data.SizeVector() == 2, "Must have 2 elements!");
        data.AssertAllType<FunctionBuilder>("Must have 2 FunctionBuilders!");
        return result;
    }

    FunctionBuilder operator-(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionMinus;
        return result;
    }

    FunctionBuilder operator*(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionProduct;
        return result;
    }

    FunctionBuilder operator/(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionQuotient;
        return result;
    }

    FunctionBuilder operator%(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionMod;
        return result;
    }

    //Build expression a[i] < b[i] ? c[i] : d[i]
    static FunctionBuilder BuildPointwiseLess(const FunctionBuilder a, const FunctionBuilder b, const FunctionBuilder c, const FunctionBuilder d) {
        ASSUME(isNotUninitialized(a), "Must be initialized");
        ASSUME(isNotUninitialized(b), "Must be initialized");
        ASSUME(isNotUninitialized(c), "Must be initialized");
        ASSUME(isNotUninitialized(d), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(a);
        result.data.InsertTypeEnd<FunctionBuilder>(b);
        result.data.InsertTypeEnd<FunctionBuilder>(c);
        result.data.InsertTypeEnd<FunctionBuilder>(d);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionPointwiseLess;
        return result;
    }

    //Build expression i < x[0] ? a[i] : b[i]
    static FunctionBuilder BuildPiecewiseLess(const FunctionBuilder x, const FunctionBuilder a, const FunctionBuilder b) {
        ASSUME(isNotUninitialized(a), "Must be initialized");
        ASSUME(isNotUninitialized(b), "Must be initialized");
        ASSUME(isNotUninitialized(x), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(a);
        result.data.InsertTypeEnd<FunctionBuilder>(b);
        result.data.InsertTypeEnd<FunctionBuilder>(x);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionPiecewiseLess;
        return result;
    }

    //Build expression a < b ? c : d
    static FunctionBuilder BuildLexicographicLess(const FunctionBuilder a, const FunctionBuilder b, const FunctionBuilder c, const FunctionBuilder d) {
        ASSUME(isNotUninitialized(a), "Must be initialized");
        ASSUME(isNotUninitialized(b), "Must be initialized");
        ASSUME(isNotUninitialized(c), "Must be initialized");
        ASSUME(isNotUninitialized(d), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(a);
        result.data.InsertTypeEnd<FunctionBuilder>(b);
        result.data.InsertTypeEnd<FunctionBuilder>(c);
        result.data.InsertTypeEnd<FunctionBuilder>(d);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionLexicographicLess;
        return result;
    }

    //Build Expression result[0] = lhs[index[0]]
    FunctionBuilder GetIndex(const FunctionBuilder index) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(index), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(index);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionGetIndex;
        return result;
    }

    //Build Expression result[index[0]] = lhs[0]
    FunctionBuilder NewSetIndex(const FunctionBuilder index) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(index), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(lhs);
        result.data.InsertTypeEnd<FunctionBuilder>(index);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionSetIndex;
        return result;
    }

    //Build Expression result[index[0]] = lhs[0]
    FunctionBuilder operator()(const FunctionBuilder shrink, const FunctionBuilder basecase) const {
        const FunctionBuilder& top = *this;
        ASSUME(isNotUninitialized(top), "Must be initialized!");
        ASSUME(isNotUninitialized(basecase), "Must be initialized");
        ASSUME(isNotUninitialized(shrink), "Must be initialized");
        FunctionBuilder result;
        result.data.InsertTypeEnd<FunctionBuilder>(top);
        result.data.InsertTypeEnd<FunctionBuilder>(basecase);
        result.data.InsertTypeEnd<FunctionBuilder>(shrink);
        result.FunctionBuilderNode = FunctionBuilderNodeTypes::FunctionSetIndex;
        return result;
    }

    //Apply the function to the output of F rather than input
    FunctionBuilder operator()(const FunctionBuilder F) const {
        const FunctionBuilder& applier = *this;
        ASSUME(isNotUninitialized(applier), "Must be initialized!");
        ASSUME(isNotUninitialized(F), "Must be initialized!");
        return ReplaceIdentity(F);
    }

    friend IComparisonToBool operator==(const FunctionBuilder& a, const FunctionBuilder& b) {
        if (a.FunctionBuilderNode != b.FunctionBuilderNode)
            return false;
        else if (+IMany::CompareVector(a.data, b.data))
            return false;
        else
            return true;
    }

    //Sets a borrowed-from node to a non borrowed-from node
    void setBorrowed(const FunctionBuilder& F) {
        ASSUME(  FunctionBuilderNode == FunctionBuilderNodeTypes::BorrowFromNode, "Can only set Borrowed-from nodes!");
        ASSUME(F.FunctionBuilderNode != FunctionBuilderNodeTypes::BorrowFromNode, "Cannot set to Borrowed-from nodes!");
        FunctionBuilderNode = F.FunctionBuilderNode;
        data = F.data;
    }

    //returns a function that can evaluate the constructed builder
    Function buildFunction() const {
        switch (FunctionBuilderNode)
        {
        case FunctionBuilder::FunctionBuilderNodeTypes::Uninitialized:
            PANIC("Uninitialized!");
            return {};
        case FunctionBuilder::FunctionBuilderNodeTypes::ConstantTerminal: {
            Function return_value;
            ASSERT(data.SizeVector() == 1, "Expecting 1 element!");
            Tuple ConstantValue = data.pick().GetCppType<Tuple>();
            return_value.SetConstant(ConstantValue);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::VariableTerminal: {
            Function return_value;
            ASSERT(data.SizeVector() == 0, "Expecting no element!");
            return_value.SetIdentity();
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPlus: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            data.AssertAllType<FunctionBuilder>("Expecting 2 FunctionBuilders!");
            Function F1 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetAddFunction(F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMinus: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetSubtractFunction(F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionProduct: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetProductFunction(F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionQuotient: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetQuotientFunction(F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMod: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetRemainderFunction(F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPointwiseLess: {
            Function return_value;
            ASSERT(data.SizeVector() == 4, "Expecting 4 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            Function F3 = data.nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetPointwiseLess(F0, F1, F2, F3);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPiecewiseLess: {
            Function return_value;
            ASSERT(data.SizeVector() == 3, "Expecting 3 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetPiecewiseLess(F0, F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionLexicographicLess: {
            Function return_value;
            ASSERT(data.SizeVector() == 4, "Expecting 4 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data.nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            Function F3 = data.nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetLexicographicLess(F0, F1, F2, F3);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionGetIndex: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetGetIndex(F0, F1);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionSetIndex: {
            Function return_value;
            ASSERT(data.SizeVector() == 2, "Expecting 2 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data.nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetSetIndex(F0, F1);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionBorrow: {
            Function return_value;
            ASSERT(data.SizeVector() == 3, "Expecting 3 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Tuple    F1 = data.nthElementVector(1).GetCppType<Tuple>();
            std::shared_ptr<Function> F2 = std::make_shared<Function>(data.nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction());
            return_value.SetBorrow(F0, F1, F2);
            return return_value;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::BorrowFromNode: {
            PANIC("Conversion of a borrow-from node! Set it first");
            UNREACHABLE("Panic");
            return {};
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::CallFunctionNode: {
            Function return_value;
            ASSERT(data.SizeVector() == 1, "Expecting 1 elements!");
            Function F0 = data.nthElementVector(0).GetCppType<Function>();
            return F0;
        }
        default:
            break;
        }
        UNREACHABLE("Unreachable code!");
    }

private:
    FunctionBuilder ReplaceIdentity(const FunctionBuilder& F) const {
        if (FunctionBuilderNode == FunctionBuilderNodeTypes::VariableTerminal) {
            return F;
        }
        else if (FunctionBuilderNode == FunctionBuilderNodeTypes::ConstantTerminal) {
            return *this;
        }
        else {
            IMany::MonoAnyFunction ReplaceRecursive = [&](IAny element) -> IAny { 
                if (+element.HasCPPType<FunctionBuilder>())
                    return IAny{ element.GetCppType<FunctionBuilder>("").ReplaceIdentity(F) };
                else
                    return element;
            };
            FunctionBuilder FB = *this;
            FB.data = data.TransformMap(ReplaceRecursive);
            //Everything else is unchanged, only data is recursively transformed
            return FB;
        }
        UNREACHABLE("If-else");
    }
};

const FunctionBuilder var_ = FunctionBuilder::Variable();



class Pointwise;
class PointwiseCase1;
class PointwiseCase2;

class PointwiseCase1 {
public:
    FunctionBuilder test;
    FunctionBuilder value;

    PointwiseCase1(FunctionBuilder value, FunctionBuilder test);
    friend PointwiseCase2 operator||(PointwiseCase1 PC, FunctionBuilder alt_1);
};

class PointwiseCase2 {
    FunctionBuilder test;
    FunctionBuilder value;
    FunctionBuilder alt_1;
public:
    PointwiseCase2(PointwiseCase1 PC, FunctionBuilder alt_1);
    friend FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2);
};

PointwiseCase1::PointwiseCase1(FunctionBuilder value, FunctionBuilder test) : value(value), test(test) {}
PointwiseCase2::PointwiseCase2(PointwiseCase1 PC, FunctionBuilder alt_1) : test(PC.test), value(PC.value), alt_1(alt_1) {}

PointwiseCase2 operator||(PointwiseCase1 PC, FunctionBuilder alt_1) { return PointwiseCase2(PC, alt_1); }
FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPointwiseLess(PC.test, PC.value, PC.alt_1, alt_2); }
PointwiseCase1 operator<(FunctionBuilder F, FunctionBuilder P) { return PointwiseCase1(F, P); }


//alt_1 | test | alt_2
class Piecewise;
class PiecewiseCase1;

class PiecewiseCase1 {
public:
    FunctionBuilder test;
    FunctionBuilder value;

    PiecewiseCase1(FunctionBuilder value, Piecewise test);
    friend FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_1);
};

class Piecewise {
public:
    Piecewise(FunctionBuilder F);
    friend PiecewiseCase1 operator|(FunctionBuilder F, Piecewise P);
    FunctionBuilder index;
};

PiecewiseCase1::PiecewiseCase1(FunctionBuilder value, Piecewise test) : value(value), test(test.index) {}
Piecewise::Piecewise(FunctionBuilder F) : index(F) {}

FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPiecewiseLess(PC.value, PC.test, alt_2); }
PiecewiseCase1 operator|(FunctionBuilder alt_1, Piecewise index) { return PiecewiseCase1(alt_1, index); }


class Lexicographic;
class LexicographicCase1;
class LexicographicCase2;

class LexicographicCase1 {
public:
    FunctionBuilder test;
    FunctionBuilder value;

    LexicographicCase1(FunctionBuilder value, Lexicographic test);
    friend LexicographicCase2 operator|(LexicographicCase1 PC, FunctionBuilder alt_1);
};

class LexicographicCase2 {
    FunctionBuilder test;
    FunctionBuilder value;
    FunctionBuilder alt_1;
public:
    LexicographicCase2(LexicographicCase1 PC, FunctionBuilder alt_1);
    friend FunctionBuilder operator|(LexicographicCase2 PC, FunctionBuilder alt_2);
};

class Lexicographic {
public:
    Lexicographic(FunctionBuilder F);
    friend LexicographicCase1 operator<<(FunctionBuilder F, Lexicographic P);
    FunctionBuilder test;
};


LexicographicCase1::LexicographicCase1(FunctionBuilder value, Lexicographic test) : value(value), test(test.test) {}
LexicographicCase2::LexicographicCase2(LexicographicCase1 PC, FunctionBuilder alt_1) : test(PC.test), value(PC.value), alt_1(alt_1) {}
Lexicographic::Lexicographic(FunctionBuilder F) : test(F) {}

LexicographicCase2 operator|(LexicographicCase1 PC, FunctionBuilder alt_1) { return LexicographicCase2(PC, alt_1); }
FunctionBuilder operator|(LexicographicCase2 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildLexicographicLess(PC.test, PC.value, PC.alt_1, alt_2); }
LexicographicCase1 operator<<(FunctionBuilder F, Lexicographic P) { return LexicographicCase1(F, P); }


//If Test(x) == case_, return Value(x), else return Main(x);
static Function CreateCase(Tuple case_, FunctionBuilder Value, FunctionBuilder Main, FunctionBuilder Test) {
    FunctionBuilder Identity = var_;
    FunctionBuilder Case = case_;
    FunctionBuilder IsLess = Test << Case | Main | Value;
    FunctionBuilder IsMore = Case << Test | Main | IsLess;
    return IsMore.buildFunction();
}

static Function CreateIndexConstant(Tuple T) {
    FunctionBuilder GetIndex = var_.GetIndex(T);
    return GetIndex.buildFunction();
}

static Function CreateAddConstant(Tuple T) {
    FunctionBuilder Adder = var_ + T;
    return Adder.buildFunction();
}

//base[index] = value, base[!index] = base
static Function CreatePiecewiseAt(FunctionBuilder base, FunctionBuilder index, FunctionBuilder value) {
    //F[-Inf:Index] = base; F[Index:Index+1] = value; F[Index + 1:Infinity] = base;
    FunctionBuilder FinalResult = base | index | value | index + 1 | base;
    return FinalResult.buildFunction();
}


int main() {
    
    //Input variable
    FunctionBuilder in = var_;
    FunctionBuilder FibonacciX = (in - 1) + (in - 2);
    //If in < 1 ? in : in > 1 ? in : 1
    Function Fibonacci = FibonacciX.buildFunction();
    std::cout << Fibonacci(Tuple(3)).to_str() << '\n';

    

    return 0;
    //    unsigned int x = 1;
    //    ASSUME(x != 0, "x should not be zero");
    //    CASES(x, 0, 2, 1, 3);
    //
    //    const IMany::MonoVoidFunction print = [](const IAny A) {
    //        if (+A.HasCPPType<int>())
    //            std::cout << A.GetCppType<int>() << '\n';
    //        else if (+A.HasCPPType<char>())
    //            std::cout << A.GetCppType<char>() << '\n';
    //        else if (+A.HasCPPType<std::string>())
    //            std::cout << A.GetCppType<std::string>() << '\n';
    //        else
    //            return;
    //    };
    //    const IMany::PolyAnyMutateFunction<void> double_all = [](IAny& A) {
    //        if (+A.HasCPPType<int>())
    //            A.storeAny(
    //                (int)
    //                A.GetCppType<int>() * 2
    //            );
    //    };
    //
    //    Tuple A, B;
    //    //A[0] = "18446744073709551616"
    //    A.GetNumber_Index(0) = number("18446744073709551616");
    //    //A[1] = "4"
    //    A.GetNumber_Index(1) = number("4");
    //    //A[2] = "16"
    //    A.GetNumber_Index(2) = number("16");
    //
    //    //B[0] = "2^64"
    //    B.GetNumber_Index(0) = number("18446744073709551616");
    //    //B[1] = "4"
    //    B.GetNumber_Index(1) = number("4");
    //    //B[2] = "16"
    //    A.GetNumber_Index(2) = number("16");
    //
    //    std::cout << A.to_str();
    //    std::cout << " " << B.to_str() << '\n';
    //    std::cout << Tuple::isLess(A, B) << '\n';
    //
    //
    //    IMany list, list2;
    //    list.InsertIAnyEnd(IAny(1));
    //    list.InsertIAnyEnd(IAny(2));
    //    list.InsertTypeEnd<int>(3);
    //    list.OperateElement(print);
    //    list.MutateElement(double_all);
    //    list.OperateElement(print);
    //    
    //    list2.InsertIAnyEnd(IAny(6));
    //    list2.InsertIAnyEnd(IAny(2));
    //    list2.InsertIAnyEnd(IAny(6));
    //    list2.InsertIAnyEnd(IAny(4));
    //
    //    std::cout << IMany::CompareSet(list, list2) << '\n';
    //    std::cout << IMany::CompareMultiset(list, list2) << '\n';
    //
    //    list.InsertTypeEnd<int>(34);
    //    list.InsertTypeEnd<std::string>("Hello!");
    //    list.InsertTypeEnd<char>('z');
    //
    //    list.OperateElement(print);
    //
    //#ifdef __ENABLE_ASSERTS
    //#undef __ENABLE_ASSERTS
    //#endif
    //
    //    bool __ENABLE_ASSERTS = false;
    //    for (int i = 0; i < 10000; i++) {
    //        __ENABLE_ASSERTS = not __ENABLE_ASSERTS;
    //        ASSERT((i % 2 == 0), "Int must be even");
    //    }
    //
    //#ifdef __ENABLE_ASSERTS
    //#undef __ENABLE_ASSERTS
    //#endif
    //#define __ENABLE_ASSERTS 0
    //
    //    for (int i = 0; i < 10000; i++) {
    //        if (i % 2 != 1) {
    //            ASSERT((i % 2 == 0), "Int must be even");
    //        }
    //    }
    //
    //    Sequence Z;
    //    auto Z1 = Z.newSequencePointStart();
    //    auto Z2 = Z.newSequencePointStart();
    //    auto Z3 = Z.newSequencePointFollower(Z1);
    //    Z.insertNewAfter(Z2, Z3);
    //    Z.insertNewAfter(Z3, Z1);
    //    std::map<SequencePoint, number> pointList;
    //    pointList[Z1] = 7;
    //    pointList[Z2] = 8;
    //    pointList[Z3] = 9;
    //    SequencePoint Zx = Z2;
    //    for (int i = 0; i < 10; i++) {
    //        std::cout << pointList[Zx] << '\n';
    //        Zx = Z.followSequencePointSimple(Zx).GetCppType<SequencePoint>();
    //    }

    //Function Reverse, DecrementJump, Negate, Height;
    ////Ackermann's Function:
    ////A(0, n) = n + 1
    ////A(m, 0) = A(m - 1, 1)
    ////A(m, n) = A(m - 1, A(m, n - 1))

    ////B(n, m, 0...)
    ////B(0, 0...) = 1
    ////B(x, 0...) = x + 1
    ////B(x, 1, 0...) = A(0, A(1, x - 1)) = B(B(x - 1, 1), 0)
    ////B(n, m, ...) = B(B(n - 1, m), m - 1, ...)

    //Function Identity = Function::CreateIdentity();
    //Function Zero = Function::CreateConstant(number(0));
    //Function One = Function::CreateConstant(number(1));
    //Function Two = Function::CreateConstant(number(2));
    //Function Increment = CreateAddConstant(number(1));
    //Function GetIndexZero = CreateIndexConstant(number(0));
    //Function GetIndexOne = CreateIndexConstant(number(1));

    //std::cout << Identity.print_Function() << '\n';
    //std::cout << Zero.print_Function() << '\n';
    //std::cout << One.print_Function() << '\n';
    //std::cout << Two.print_Function() << '\n';
    //std::cout << Increment.print_Function() << '\n';
    //std::cout << GetIndexZero.print_Function() << '\n';
    //std::cout << GetIndexOne.print_Function() << '\n';

    ////Function FibonacciShrink1 = Function::CreateSubFunction(Identity, One);
    ////Function FibonacciShrink2 = Function::CreateSubFunction(Identity, Two);
    ////Function Fibonacci;
    ////Function FibonacciBorrow1 = Function::CreateBorrowNode(FibonacciShrink1, number(0), &Fibonacci);
    ////Function FibonacciBorrow2 = Function::CreateBorrowNode(FibonacciShrink2, number(0), &Fibonacci);
    ////std::cout << FibonacciShrink1.print_Function() << '\n';
    ////std::cout << FibonacciShrink2.print_Function() << '\n';
    ////std::cout << FibonacciBorrow1.print_Function() << '\n';
    ////std::cout << FibonacciBorrow2.print_Function() << '\n';

    ////Function FibonacciAdd = Function::CreateAddFunction(FibonacciBorrow1, FibonacciBorrow2);
    ////std::cout << FibonacciAdd.print_Function() << '\n';

    
    ////std::cout << FibonacciAddCase1.print_Function() << '\n';
    ////Function FibonacciAddCase0 = CreateCase(number(0), Zero, FibonacciAddCase1, Identity);
    ////std::cout << FibonacciAddCase0.print_Function() << '\n';

    ////Fibonacci.SetAddFunction(FibonacciAddCase0, Zero);
    ////std::cout << Fibonacci.print_Function() << '\n';

    ////Tuple input = number(2000);
    ////std::cout << Fibonacci(input).to_str();

    //Tuple secondOne;
    //secondOne.data[1] = 1;
    //Function SecondOne = Function::CreateConstant(secondOne);
    ////Ackermann Function (modified):
    ////B(n, m, 0...)
    ////B(0, 0...) = 1
    ////B(x, 0...) = x + 1
    ////B(x, 1, 0...) = A(0, A(1, x - 1)) = B(B(x - 1, 1), 0)
    ////B(n, m, ...) = B(B(n - 1, m), m - 1, ...)
    //Function Ackermann;
    //Function AckermannGetFirst = Function::CreateGetIndex(Zero, Identity);
    //AckermannGetFirst.name() = "x[0]";
    //Function AckermannGetSecond = Function::CreateGetIndex(One, Identity);
    //AckermannGetSecond.name() = "x[1]";
    //Function AckermannBorrowValue = Function::CreateSubFunction(Identity, One);
    //AckermannBorrowValue.name() = "x - [1,...]";
    //Function AckermannBorrowCall = Function::CreateBorrowNode(AckermannBorrowValue, number(1), &Ackermann);
    ////AckermannBorrowCall.name() = "Ackermann_T1";
    //Function AckermannGetSecondBorrow = Function::CreateGetIndex(One, AckermannBorrowCall);
    //Function AckermannReduceSecondBorrow = Function::CreateSubFunction(AckermannGetSecondBorrow, One);
    //Function AckermannShiftReducedBorrow = Function::CreateSetIndex(One, AckermannReduceSecondBorrow);
    //Function AckermannShiftSecondBorrow = Function::CreateSetIndex(One, AckermannGetSecondBorrow);
    //Function AckermannRemoveSecond = Function::CreateSubFunction(AckermannBorrowCall, AckermannShiftSecondBorrow);
    //Function AckermannSetSecond = Function::CreateAddFunction(AckermannRemoveSecond, AckermannShiftReducedBorrow);
    //Function AckermannRecurse = Function::CreateBorrowNode(AckermannSetSecond, number(1), &Ackermann);
    ////AckermannRecurse.name() = "Ackermann_T2";
    //Function AckermannAddOne = Function::CreateAddFunction(Identity, One);
    //AckermannAddOne.name() = "(x + 1)";
    //Function AckermannCaseX0 = CreateCase(number(0), AckermannAddOne, AckermannRecurse, AckermannGetSecond);
    //Ackermann.SetAddFunction(AckermannCaseX0, Zero);
    //Ackermann.name() = "B";
    //std::cout << AckermannCaseX0.print_Function() << '\n';

    //std::cout << Ackermann(Tuple::ConstructPair(1, 1)).to_str();

    ////Reverse.NodeClass() = Function::NodeType::PointwiseIdentity;
    ////DecrementJump.NodeClass() = Function::NodeType::PointwiseIdentity;
    ////Negate.NodeClass() = Function::NodeType::PointwiseIdentity;
    ////Height.NodeClass() = Function::NodeType::PointwiseIdentity;
    ////
    ////Tuple D = Function::EvaluateInduct(A, Tuple(), DecrementJump, Negate, Reverse, Increment, Height);
    ////std::cout << D.to_str();
    ////

    ////Tuple basecase(1);

        

    //Theory N;
    //Theory Zero;
    //Theory Successor;
    //Theory Less;

    ////Zero is fixed, no further modifications
    //Zero.lock();
    ////Successor takes an N and returns an N
    ////Successor = operation(N) with witness(N), result is an N
    //Successor.setCreateOperation(TheoryExpression::TheoryExpression3(TheoryExpression::TheoryMember(N), TheoryExpression::TheoryEmpty(), TheoryExpression::TheoryMember(N)));
    ////No further modifications to successor
    //Successor.lock();

    ////Accepts Succ(X) if N accepts X
    //Theory NSuccessor;
    //NSuccessor.setCases(TheoryExpression::TheoryExpressionN(TheoryExpression::TheoryCall(TheoryExpression::TheoryEqual(Successor), TheoryExpression::TheoryMember(N))));

    ////N's cases are either 0 or Successor of something that N accepts
    //N.setCases(TheoryExpression::TheoryExpressionN(TheoryExpression::TheoryExpression1(TheoryExpression::TheoryEqual(Zero)), TheoryExpression::TheoryExpression1(TheoryExpression::TheoryCall(TheoryExpression::TheoryEqual(Successor), TheoryExpression::TheoryMember(N)))));

    ////Describe equivalence over N
    //Theory NEquivalence;
    //
    ////if c == d, succ(c) == succ(d)
    ////Successor(Variable1) = Successor(Variable2), witnesses var1: N, var2: N and var2 == var1
    //NEquivalence.setCreateOperation(TheoryExpression::TheoryExpression3(TheoryExpression::TheoryExpression1(TheoryExpression::TheoryCall(TheoryExpression::TheoryEqual(Successor), TheoryExpression::TheoryVariable(1))), TheoryExpression::TheoryExpression1(TheoryExpression::TheoryCall(TheoryExpression::TheoryEqual(Successor), TheoryExpression::TheoryVariable(2))), TheoryExpression::TheoryContext(TheoryExpression::TheoryVariables(1, 2), TheoryExpression::TheoryTypes(TheoryExpression::TheoryMember(N), TheoryExpression::MultiSubtype(TheoryExpression::TheoryMember(N), TheoryExpression::TheoryEqual(TheoryExpression::TheoryVariable(1)))))));

    return 0;
}

#undef ASSERT
#undef ASSUME
#undef UNREACHABLE