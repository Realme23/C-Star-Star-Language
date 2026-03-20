
#include "Source1.h"
#include "GenerationalPointer.h"

//TODOs:
//Remove all IManys after prototyping
//Use generationalPointers for regular pointers
//Postconditions/preconditions for everything: test only function-related values, test all class invariants, don't test interfunctional relations with conditions
//Change all numeric integer types to number (all indices etc)
//  Find library that's "optimized for small case"
//Convert everything to free functions (Add UFCS later)
//Combine SequenceBuilder and FunctionBuilder; Separate out Function and Sequence execution
//A class to implement the static max_id; map<id, data>; value() = map[id] pattern (Use a single max_id for all classes)


//Start:
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
//Coercing comparison
//Used in asserts
bool operator&&(const IComparisonToBool& lhs, const char*) {
    return +lhs.getIComparisonResult();
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
            assert_message = std::string("The wrong type is stored! The stored type is: ") + typeid_of(data).name() + "\nThe expected type is: " + typeid(T).name();
        }
        if (typeid_of(data) != typeid(T)) {
            PANIC(assert_message.c_str());
        }
        try {
            T return_value = boost::type_erasure::any_cast<T>(data);
            ASSUME(return_value == boost::type_erasure::any_cast<T>(data), (std::string("Stored class may not be pure! Failing x == x! The stored class is: ") + typeid(T).name()).c_str());
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
    explicit IAny(const T& t) : data(t) {}

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

//Any with implicit casts from all const& t, used for accepting function parameters etc
struct PolyAny {
    IAny a;
    template<typename T>
    PolyAny(const T& t): a(t) {};
    operator IAny() { return a; }
};

template<typename T>
bool operator==(const IAny& lhs, const T& rhs) {
    if (not lhs.HasCPPType<T>()) {
        return false;
    }
    return lhs.GetCppType<T>("Already tested") == rhs;
}


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
        TODO_("Add unordered comparisons");
        UNREACHABLE("TODO");
    }

    //ordered x2, l/r hs_repeats x3, sub/superset x3 = 18

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
        IAny elem = IAny{ element };

        container.push_back(elem);

        IAny back = container.back();
        bool x = (back.GetCppType<T>() == elem.GetCppType<T>());
        ASSUME(container.size() == size + 1, "Must have added an element!");
        ASSUME(back.GetStoredType() == typeid(T), "Must have a T stored last!");
        ASSUME((back == elem), "Must have pushed element at the end!");
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
    //T... are the template parameter types for the additional operands to operate
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

    //Ordered multiSet Union (lhs first unmodified, rhs elements after if not in lhs)
    static IMany Union(const IMany& lhs, const IMany& rhs) {
        IMany ret = lhs;
        for (int i = 0; i < rhs.container.size(); i++) {
            auto element = rhs.container[i];
            if (!IsMember(lhs, element)) {
                ret.InsertIAnyEnd(element);
            }
        }
        FIXME("Change to account for element counts");
        return ret;
    }
    //Ordered multiSet Intersection (Remove from lhs elements that are in rhs, as many times as they're there)
    static IMany Intersection(const IMany& lhs, const IMany& rhs) {
        TODO;
    }
    //Ordered multiSet Difference (Preserves order)
    static IMany Difference(const IMany& lhs, const IMany& rhs) {
        TODO;
    }
    //Set size = number of unique elements
    size_t SizeSet() const {
        std::unordered_set<IAny> z;
        for (const auto& element : container) {
            z.insert(element);
        }
        return z.size();
    }
    //Vector size = number of inserted elements
    size_t SizeVector() const {
        return container.size();
    }
    //Removes all IAny that satisfy a predicate
    void RemoveIf(const MonoBoolFunction predicate) {
        //Use std::erase_if
        std::erase_if(container, predicate);
        return;
    }
    //Return an unspecified element; no guarantees on randomness
    IAny pickLast() const {
        if (container.size() == 0) {
            return IAny{};
        }
        return container.back();
    }
    //Selects a random element; as a Set
    IAny pickRandomSet() const {
        TODO;
    }
    //Return an unspecified element and remove it (and all copies of it)
    IAny popSet() {
        TODO;
    }
    //Return an unspecified element and remove it (retain other copies)
    IAny popVector() {
        IAny return_value = {};
        if (container.size() == 0) {
            return return_value;
        }
        return_value = container.back();
        container.pop_back();
        return return_value;
    }
    //Retrieves ith element from vector
    //Index according to indexing function and transform the returned result
    //Return IEmptyValue if it is out of scope
    IAny nthElementVector(const number index, const MonoIndexFunction indexing = nullptr, const MonoAnyFunction transform = nullptr) const {
        number k = indexing ? indexing(index) : index;
        ASSERT(k < (1ULL << 63), "Must be small!");
        size_t k_conv = boost::multiprecision::integer_modulus(k, 1ULL << 63);
        ASSERT((number)k_conv == k, "Must convert better!");
        if (k_conv >= container.size())
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
            size_t k = to_size_t(indexing ? indexing(i) : i);
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
        size_t k = to_size_t(indexing ? indexing(index2) : index2);
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

    //A friend class that has easy construction; useful for function arguments
    friend class PolyMany;

private:
    //Stored data
    std::vector<IAny> container;

    //A list of names; maps container index to names
    std::unordered_map<number, std::string> names;

    //Retrieve the ith element, according to index and transform
    static IAny get_i_element(const IMany& hs, number index, const MonoIndexFunction hs_index = nullptr, const MonoAnyFunction hs_transform = nullptr)
    {
        size_t new_index = to_size_t(hs_index ? hs_index(index) : index);
        return hs_transform ? hs_transform(hs.container[new_index]) : hs.container[new_index];
    };
};


//Construct IManys from heterogeneous lists
//Use as the argument of a function to automatically receive an IMany from an initializer list
class PolyMany {
    IMany data;
public:
    PolyMany() = default;
    //Construct from a heterogeneous list
    PolyMany(std::initializer_list<IAny> l) {
        for (auto&& x : l) {
            data.container.push_back(x);
        }
    }
    //Construct from a homogeneous list
    template<typename T>
    PolyMany(std::initializer_list<T> l) {
        for (auto&& x : l) {
            data.container.push_back(IAny(x));
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
    static IMacroList Newlist() { return IMacroList(); }
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
    IMany BuildIMany() {
        ASSUME(_lock, "Prepare from an unlocked IMacroList");
        return prepared;
    }

private:
    IMany prepared;
    bool _lock = false;
};





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
            //Same height, same lead digits
            //Iteratively compare smaller digits
            number index = startTa.GetNumberConst_Index(0);
            //Compare upto the lowest tail
            number end_index = std::min(endTa.GetNumberConst_Index(0), endTb.GetNumberConst_Index(0));
            while (index -->= end_index) {
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
        if (+hasSingleLimb(*this, 0)) {
            return to_string(GetNumberConst_Index(0));
        }
        for (const std::pair<number, number>& entry : data) {
            if (entry.second != 0) {
                s += to_string(entry.first);
                s += ": ";
                s += to_string(entry.second);
                s += ", ";
            }
        }
        if (s.size() > 1) {
            s.resize(s.size() - 2);
            s += "]";
            return s;
        }
        else if (s.size() == 1) {
            return "0";
        }
        else {
            UNREACHABLE("Shouldn't be < 1");
        }
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

    //Does it have only 1 non-zero value at the specified index?
    static IComparisonResult hasSingleLimb(const Tuple& a, const number at = 0) {
        for (const std::pair<number, number>& x : a.data) {
            if (x.first != at and x.second != 0) {
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
                if (not found) {
                    max = entry.first;
                    found = true;
                }
                else {
                    max = std::max(max, entry.first);
                }
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
        if (choices.SizeVector() < to_size_t(choice))
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
            if (selectionNodes[before].SizeVector() < to_size_t(choice))
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
//Only supports total, pure functions that return in finite time
//All functions are fully specified (x /0 == 0 and x %0 == x)
///Operations: Lexicographically bounded recursion
///Identity function, pointwise add/subtract/multiply/divide/mod etc, index get/set
///Fully Immutable once built
///Atoms := (Finite) Constant values, Pointwise Identity function
///Describe function type-selection and dispatch explicitly, along with allocation etc
class Function {
    //Nodes:
    //Constant values
    //Pointwise Identity, pointwise +-*/, get/set single
    //Total piecewise based on < or > or == (ie if-else)
    //Borrow: Bounded recursion calls
    enum class NodeType {
        Uninitialized,      //Panic if called
        Constant,           //Return a constant value
        PointwiseIdentity,  //Return the input unchanged
        PointwisePlus,      //Return R[i] := a[i] + b[i]
        PointwiseMinus,     //Return R[i] := a[i] - b[i]
        PointwiseProduct,   //Return R[i] := a[i] * b[i]
        PointwiseQuotient,  //Return R[i] := a[i] / b[i]    (x /0 = 0)
        PointwiseRemainder, //Return R[i] := a[i] % b[i]    (x %0 = x)
        PointwiseLess,      //Return R[i] := a[i] < b[i] ? c[i] : d[i]
        PiecewiseLess,      //Return R[i] := i < c[0] ? a[i] : b[i]
        LexicographicLess,  //Return R    := a L< b ? c : d
        GetIndex,           //Return R[0] = b[a[0]]
        SetIndex,           //Return R[a[0]] = b[0]
        Borrow,             //Return the value of calling a "higher" function, with an input strictly between current input and zero
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

    //Sets the recursive call - not the recursive definition
    void SetBorrow(Function shrink, Tuple basecase, Function top) {
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
        FunctionNodes().InsertTypeEnd(threshold);
        FunctionNodes().InsertTypeEnd(ifLess);
        FunctionNodes().InsertTypeEnd(ifEqualOrMore);
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
    Tuple dispatchThis(Tuple input_) {
        Tuple input = normalize(input_);
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
            Tuple target = FunctionNodes().pickLast().GetCppType<Tuple>("FunctionNode should store Tuple!");
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
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwisePlus(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseMinus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwiseMinus(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseProduct: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple lhs_t    = lhs_f(input);
            Tuple rhs_t    = rhs_f(input);
            output = EvaluatePointwiseProduct(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseQuotient: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple    lhs_t = lhs_f(input);
            Tuple    rhs_t = rhs_f(input);
            output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseRemainder: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple    lhs_t = lhs_f(input);
            Tuple    rhs_t = rhs_f(input);
            output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
            break;
        }
        case NodeType::PointwiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
            Function value = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function test =  FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function alt_1 = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            Function alt_2 = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
            Tuple    ahs_t = value(input);
            Tuple    bhs_t = test(input);
            Tuple    chs_t = alt_1(input);
            Tuple    dhs_t = alt_2(input);
            output = EvaluatePointwiseLess(ahs_t, bhs_t, chs_t, dhs_t);
            break;
        }
        case NodeType::PiecewiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
            Function left_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function right_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function threshold_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            Tuple    left_t = left_f(input);
            Tuple    right_t = right_f(input);
            Tuple    threshold = threshold_f(input);
            output = EvaluatePiecewiseLess(left_t, right_t, threshold.GetNumberConst_Index(0));
            break;
        }
        case NodeType::GetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple ahs_t = ahs_f(input);
            Tuple bhs_t = bhs_f(input);
            output = EvaluateGetIndex(ahs_t, bhs_t);
            break;
        }
        case NodeType::SetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Tuple ahs_t    = ahs_f(input);
            Tuple bhs_t    = bhs_f(input);
            output = EvaluateSetIndex(ahs_t, bhs_t);
            break;
        }
        case NodeType::Borrow: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode Borrow must store 3 elements!");
            Function top        = FunctionNodes().nthElementVector(0).GetCppType<Function>("[0] element must be a Function Pointer! (Top)");
            Function shrink     = FunctionNodes().nthElementVector(1).GetCppType<Function>("[1] element must be a Function! (shrink)");
            Tuple basecase      = FunctionNodes().nthElementVector(2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
            output              = EvaluateBorrow(input, shrink, basecase, top);
            break;
        }
        case NodeType::LexicographicLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
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
    
    //R[i] := a[i] < b[i] ? c[i] : d[i]
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
            if (a_val < b_val) {
                result.data[index] = c_val;
            }
            else if (a_val >= b_val) {
                result.data[index] = d_val;
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
                if(a_val != 0)
                    result.data[index] = a_val;
            }
            else if (index >= threshold) {
                if(b_val != 0)
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

    static Tuple normalize(Tuple input) {
        std::erase_if(input.data, [](auto x)->bool { return x.second == 0; });
        if (input.data.size() == 0)
            input = Tuple();
        return input;
    }

    //Takes four parameters: Input, Recursive call argument generator, basecase, and full-function
    //Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
    //If shrunk operand is L<= 0 or has any negative element at positive index; return the basecase
    //Return the result of applying the "top" function to the shrunk value
    //If top returns the "true" function, can be used to create recursive calls
    //eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
    static Tuple EvaluateBorrow(Tuple Op1, Function shrinker, Tuple basecase, Function top) {
        static Tuple ZeroT = Tuple(0);
        if (+Tuple::isZero(Op1)) {
            return basecase;
        }
        Tuple shrunk1 = shrinker(Op1);
        Tuple shrunk1Left = EvaluatePiecewiseLess(shrunk1, ZeroT, number(0));
        Tuple shrunk1Right = EvaluatePiecewiseLess(ZeroT, shrunk1, number(0));
        //If the left half is not empty
        bool isLeftNotEmpty = not Tuple::isZero(shrunk1Left);
        //If the right half is empty
        bool isRightEmpty = +Tuple::isZero(shrunk1Right);
        //If Input << 0
        bool isNegative = +Tuple::isLess(shrunk1, ZeroT);
        //If not Input <<= Shrunk
        bool isNotSmaller = not Tuple::isLess(shrunk1, Op1);
        //If Input < 0 != 0
        bool hasAnyNegative = not Tuple::isZero(Function::EvaluatePointwiseLess(shrunk1, ZeroT, shrunk1, ZeroT));
        //If Input > 0 == 0
        bool hasAnyNegativeRight = not Tuple::isZero(Function::EvaluatePointwiseLess(shrunk1Right, ZeroT, shrunk1Right, ZeroT));

        if (isRightEmpty or isNegative or isNotSmaller or hasAnyNegativeRight) {
            return basecase;
        }
        return top(shrunk1);
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

    std::string print_Function(bool is_sub = false) {
        if (name() != "" and is_sub)
            return name();
//        std::string id_ = to_string(id);
        std::string id_ = "";
        std::string return_val = "";
        NodeType NodeTypeClass = NodeClass();
        switch (NodeTypeClass)
        {
        case NodeType::Uninitialized: {
            PANIC("Print uninitialized function!");
            UNREACHABLE("Panic");
            return_val += "";
            break;
        }
        case NodeType::Constant: {
            ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
            Tuple target = FunctionNodes().pickLast().GetCppType<Tuple>("FunctionNode should store Tuple!");
            if (+Tuple::isZero(target)) {
                return_val += "0";
            }
            else if (+Tuple::hasSingleLimb(target, 0)) {
                return_val += to_string(target.GetNumberConst_Index(0));
            }
            else {
                return_val += target.to_str();
            }
            break;
        }
        case NodeType::PointwiseIdentity: {
            ASSUME(FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
            return_val += "x";
            break;
        }
        case NodeType::PointwisePlus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            return_val += std::string("(") + lhs_f.print_Function(true) + " + " + rhs_f.print_Function(true) + ")";
            break;
        }
        case NodeType::PointwiseMinus: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            return_val += std::string("(") + lhs_f.print_Function(true) + " - " + rhs_f.print_Function(true) + ")";
            break;
        }
        case NodeType::PointwiseProduct: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            return_val += std::string("(") + lhs_f.print_Function(true) + " * " + rhs_f.print_Function(true) + ")";
            break;
        }
        case NodeType::PointwiseQuotient: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            return_val += std::string("(") + lhs_f.print_Function(true) + " / " + rhs_f.print_Function(true) + ")";
            return return_val;
        }
        case NodeType::PointwiseRemainder: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
            Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            return_val += std::string("(") + lhs_f.print_Function(true) + " % " + rhs_f.print_Function(true) + ")";
            return return_val;
        }
        case NodeType::PointwiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
            return std::string("(") + ahs_f.print_Function(true) + " < " + bhs_f.print_Function(true) + " ? " + chs_f.print_Function(true) + " : " + dhs_f.print_Function(true) + ")" + id_;
        }
        case NodeType::PiecewiseLess: {
            ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
            Function left = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function right = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function threshold = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            return std::string("(") + left.print_Function(true) + " | " + threshold.print_Function(true) + " | " + right.print_Function(true) + ")" + id_;
        }
        case NodeType::GetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            if (ahs_f.NodeClass() == NodeType::Constant) {
                Tuple indexTuple = ahs_f.FunctionNodes().nthElementVector(0).GetCppType<Tuple>();
                return_val = bhs_f.print_Function(true) + "[" + to_string(indexTuple.GetNumberConst_Index(0)) + "]";
            }
            else {
                return_val += bhs_f.print_Function(true) + "[" + ahs_f.print_Function(true) + "]";
            }
            break;
        }
        case NodeType::SetIndex: {
            ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
			if (ahs_f.NodeClass() == NodeType::Constant) {
				Tuple t = ahs_f.FunctionNodes().nthElementVector(0).GetCppType<Tuple>();
				number n = t.GetNumberConst_Index(0);
				if (n == 0) {
					return_val = bhs_f.print_Function(true);
				}
				else {
					return_val = std::string("[") + to_string(n) + ": " + bhs_f.print_Function(true) + "]";
				}
			}
            else {
                return_val = std::string("[") + ahs_f.print_Function(true) + "[0]: " + bhs_f.print_Function(true) + "]";
            }
            break;
        }
        case NodeType::Borrow: {
            //Change, replace with "independent improper functions"
            //PANIC("Disimplement!");
            Function top = FunctionNodes().nthElementVector(0).GetCppType<Function>("[0] element must be a Function! (Top)");
            Function shrink = FunctionNodes().nthElementVector(1).GetCppType<Function>("[1] element must be a Function! (shrink)");
            Tuple basecase = FunctionNodes().nthElementVector(2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
            std::string top_name = (top.name() == "" ? "top" + to_string(top.id) : top.name());
            return top_name + "(" + basecase.to_str() + ", " + shrink.print_Function(true) + ")" + id_;
        }
        case NodeType::LexicographicLess: {
            ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
            Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
            Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
            Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
            Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
            return std::string("(") + ahs_f.print_Function(true) + " << " + bhs_f.print_Function(true) + " ? " + chs_f.print_Function(true) + " : " + dhs_f.print_Function(true) + ")" + id_;
        }
        default:
            UNREACHABLE("All cases should be covered.");
            PANIC("Default Case!");
        }
        return return_val + id_;
    }

    //Structural equality
    //identity == identity
    //constant == constant iff a.value == b.value
    //+-*/% [] < << | are equal if the values are recursively equal (as checked by the operator)
    //If any borrow occurs, the value is unknown (not structurally checked)
    static IComparisonResult StructuralEquality(const Function& a, const Function& b) {
        if (a.id == b.id) {
            return true;
        }
        if (a.NodeClass() == NodeType::Borrow or b.NodeClass() == NodeType::Borrow) {
            return IComparisonResult::ConstructUnknown();
        }
        if (a.NodeClass() != b.NodeClass()) {
            return false;
        }
        if (a.NodeClass() == NodeType::Uninitialized) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            return true;
        }
        if (a.NodeClass() == NodeType::PointwiseIdentity) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            return true;
        }
        if (a.NodeClass() == NodeType::Constant) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            Tuple a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Tuple>();
            Tuple b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Tuple>();
            if (Tuple::isEqual(a0, b0)) {
                return true;
            }
            else {
                return false;
            }
        }
        if (std::set{NodeType::PointwisePlus, NodeType::PointwiseMinus, NodeType::PointwiseProduct, NodeType::PointwiseQuotient, NodeType::PointwiseRemainder, NodeType::GetIndex, NodeType::SetIndex}.contains(a.NodeClass())) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            if (a0 == a1 and b0 == b1) {
                return true;
            }
            else {
                return false;
            }
        }
        if (std::set{NodeType::PiecewiseLess}.contains(a.NodeClass())) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            Function a2 = (a.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
            Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            Function b2 = (b.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
            if (a0 == a1 and b0 == b1 and a2 == b2) {
                return true;
            }
            else {
                return false;
            }
        }
        if (std::set{ NodeType::PointwiseLess, NodeType::LexicographicLess }.contains(a.NodeClass())) {
            ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
            Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            Function a2 = (a.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
            Function a3 = (a.FunctionNodes().nthElementVector(3)).GetCppType<Function>();
            Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
            Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
            Function b2 = (b.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
            Function b3 = (b.FunctionNodes().nthElementVector(3)).GetCppType<Function>();
            if (a0 == a1 and b0 == b1 and a2 == b2 and a3 == b3) {
                return true;
            }
            else {
                return false;
            }
        }
        UNREACHABLE("Checked all cases!");
    }

    //Nominal if has borrows, structural if not
    friend bool operator==(const Function& a, const Function& b) {
        IComparisonResult x = StructuralEquality(a, b);
        if (x.isKnown()) {
            return (x.getBoolOrAssert("If-guard"));
        }
        return (a.id == b.id);
    }

    number id;
    static number max_id;
};

namespace std {
    template <> struct hash<Tuple>
    {
        size_t operator()(const Tuple& x) const
        {
            using namespace boost::multiprecision;
            using namespace std;
            return (size_t)abs(((x.GetNumberConst_Index(0)) ^ x.GetNumberConst_Index(1)) % (1LL << 31));
        }
    };

    template <> struct hash<Function>
    {
        size_t operator()(const Function& x) const
        {
            using namespace boost::multiprecision;
            using namespace std;
            return (size_t)abs((x.id) % (1LL << 31));
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

//Optimizations:

//"Free" optimizations: (Can be performed without non-trivial search optimizations)
//Propagate constants fully
//Monomorphize all function calls on the call-site  f(int a, int b) -> f(3, b) etc
//Monomorphize all theories (eg int a, b; -> int1 a; int2 b;)
//Replace all variables with (function+state) such that it evaluates equal to the variable at that value
//Strengthen all theories from the end  int c := a + b -> int(a + b) c := a + b
// Reduce function-types of outputs' codomain to match function-types of consumption' domain
//Delay all computation "production" until "consumption" in the sequence
//Weaken all theories to only match requirements for all "consumption"

//Purify pass:
//The "full type" of a function is the equivalence class of all functions that produce the same output given the same inputs
// Types over partial inputs can be handled by slicing the non-input to zero
//Strength reduction of bounded loops to for(i = 0; i != ...; i++) { /*No writes to i*/; }
//Eliminate all bounded recursion with monomorphization (ie f(3, a) = f(2, b) + c becomes f_3(a) = f_2(b) + c)
//  Transform to non-recursive functions with basecases
//Sequence-Invariant code motion: Useful for loops and branches
//  Moves code representing constraints "outside" sequences that retain those constraints
//Replace all variables with "const shadowing CoWs"
//  ie a = a+1
//   to
//  a2 = a1 + 1
//Merge all unbounded loops program-wide into a single unbounded loop with a single inductor+local variables
//  eg convert 
//      generate(x):
//          co_return next x;
//      generate(y):
//          co_return next y;
//      generate(z):
//          generate(x);
//          generate(y);
//          co_return next z;
//      to
//      generate(z):
//          next x;
//          next y;
//          co_return next z;

//Search pass: (Opposites)
//Monomorphize functions <-> deduplicate ("template-ize") functions
//Inline function calls <-> extract function calls
//Cache vs "functionize" pure variables
//eg
//  a = b + c
//  f(a)
// to/from
//  f(b + c)
//Complete "conditional" theories by solving for the unconditional result, to facilitate "Sequence-invariant code motion"

//Representation pass:
//Represent theories with restrictions on Tuples (ie f[input] = fixed)
//Solve for theory representations by solving (f[input] = fixed)
// Functions with multiple inputs (by tying multiple inputs together)
// Functions that are partially applied are functions with some inputs specified
// 
//Describe "infinities" with generators, switch between different stateless representations
//Brute force equivalence class relations with (f[input, equivalence] = fixed)
//Purify all computation with "pure" and "impure" sections, separating their cones of influence
// 
// Equivalence class relations split the input into two factors; an equivalence member and an equivalence result
// The "equivalence result" is a single unique representation of the equivalence class
// Different members of the equivalence class can be represented with equivalence results, and can be compared (both == and !=) by comparing the equivalence result
// The equivalence result can be found by brute-forcing the solution to "equivalence_class[a] == equivalence_class[b]"
//
//eg can represent infinity with x such that x - 1 = x (the result of x - 1 has been generated by the definition)
//Representing a generator over a different domain needs (possibly hidden) statefulness, but without representation is stateless
//eg Infinity can also be represented like x such that n < x for all n in N.
//  This can be "approximated statefully" by using a number n', which is actually in N, but is bigger than all elements in the discourse.
//  These approximations have to be stateful because eg the "discourse" is stateful (eg as new inputs are received, the "largest" number has to be increased)
//  Generating a "stateful" approximation from a theory description can be accomplished by brute force solving the constraints

//For proving the consistency of axiom sets, assign each derivation with a new number n in N. When the independence of each derivation is proved from n, then the axioms satisfy the theory of term-rewriting systems, and hence are valid axiomatic systems.
//Inconsistent models can be "salvaged" by a different model that has a dependence on the number n. This is horizontal shift, not a vertical shift.
//If a model is inconsistent, it can still produce useful results, by explicitizing the dependence on n, and weakening its theory from "free term-rewriting systems)


//Builds a Function from C++ code
class FunctionBuilder {
public:
    
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
        FunctionBorrowedNode,   //Build recursive call
        UnsetBorrowFromNode,    //The node that is borrowed from
        SetBorrowFrom,          //A "borrow from" node that is set to another function
        CallFunctionNode,       //Calls a prebuilt function
    };

    number builder_id = 0;
    static number max_builder_id;
    static std::unordered_map<number, IMany> data_static;
    static std::unordered_map<number, FunctionBuilderNodeTypes> node_types;
    static std::unordered_map<number, std::string> names;

    FunctionBuilderNodeTypes& FunctionBuilderNode() {
        if (not node_types.contains(builder_id)) {
            node_types[builder_id] = FunctionBuilderNodeTypes::Uninitialized;
        }

        return node_types[builder_id];
    }

    const FunctionBuilderNodeTypes& FunctionBuilderNode() const {
        if (not node_types.contains(builder_id)) {
            node_types[builder_id] = FunctionBuilderNodeTypes::Uninitialized;
        }

        return node_types[builder_id];
    }

    IMany& data() {
        return data_static[builder_id];
    }

    const IMany& data() const {
        return data_static[builder_id];
    }

    std::string& name() {
        return names[builder_id];
    }

    const std::string& name() const {
        return names[builder_id];
    }

    static bool isNotUninitialized(const FunctionBuilder S) {
        if (S.FunctionBuilderNode() == FunctionBuilderNodeTypes::Uninitialized)
            return false;
        else if (S.FunctionBuilderNode() != FunctionBuilderNodeTypes::Uninitialized)
            return true;
        UNREACHABLE("True or not true");
        return {};
    }

    FunctionBuilder() {
        builder_id = ++max_builder_id;
        FunctionBuilderNode() = FunctionBuilderNodeTypes::Uninitialized;
        data().clear();
        name() = "";
    }

    FunctionBuilder(Tuple T): FunctionBuilder() {
        FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
        data().InsertTypeEnd(T);
    }

    FunctionBuilder(number n): FunctionBuilder() {
        FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
        data().InsertTypeEnd(Tuple(n));
    }

    FunctionBuilder(std::integral auto n) : FunctionBuilder() {
        FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
        data().InsertTypeEnd(Tuple(n));
    }

    FunctionBuilder(Function F) : FunctionBuilder() {
        FunctionBuilderNode() = FunctionBuilderNodeTypes::CallFunctionNode;
        data().InsertTypeEnd(F);
    }

    void setName(std::string s) {
        name() = s;
    }

    static FunctionBuilder Variable(){
        FunctionBuilder SB;
        SB.FunctionBuilderNode() = FunctionBuilderNodeTypes::VariableTerminal;
        SB.data().clear();
        return SB;
    }

    static FunctionBuilder BorrowFrom() {
        FunctionBuilder SB;
        SB.FunctionBuilderNode() = FunctionBuilderNodeTypes::UnsetBorrowFromNode;
        SB.data().clear();
        Function top;
        SB.data().InsertTypeEnd(top);
        return SB;
    }

    FunctionBuilder operator+(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPlus;
        ASSUME(result.data().SizeVector() == 2, "Must have 2 elements!");
        result.data().AssertAllType<FunctionBuilder>("Must have 2 FunctionBuilders!");
        return result;
    }

    FunctionBuilder operator-(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionMinus;
        return result;
    }

    FunctionBuilder operator*(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionProduct;
        return result;
    }

    FunctionBuilder operator/(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionQuotient;
        return result;
    }

    FunctionBuilder operator%(const FunctionBuilder rhs) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(rhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionMod;
        return result;
    }

    FunctionBuilder operator[](const FunctionBuilder index) const {
        return GetIndex(index);
    }

    FunctionBuilder operator-() const {
        const FunctionBuilder& lhs = *this;
        const FunctionBuilder& rhs = Tuple(-1);
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(rhs);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionProduct;
        return result;
    }

    class PolyTuple {
        Tuple T;
    public:
        PolyTuple(Tuple T) : T(T) {}

        PolyTuple(number n): T(n) {}
        template<std::integral Int>
        PolyTuple(Int n) : T(number(n)){}
        operator Tuple() const { return T; }

        PolyTuple(std::initializer_list<number> l) {
            auto l_iterator = l.begin();
            for (int i = 0; l_iterator != l.end(); i++, l_iterator++) {
                T.data[i] = *l_iterator;
            }
        }
    };

    FunctionBuilder(PolyTuple i) : FunctionBuilder() {
        FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
        data().InsertTypeEnd(Tuple(i));
    }

    //Build Expression FunctionBorrowedNode
    FunctionBuilder operator()(const FunctionBuilder shrink, const PolyTuple basecase) const {
        ASSUME(isNotUninitialized(shrink), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(*this);
        result.data().InsertTypeEnd<FunctionBuilder>(shrink);
        result.data().InsertTypeEnd<Tuple>(basecase);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionBorrowedNode;
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
        if (a.builder_id != b.builder_id)
            return false;
        else if (a.builder_id == b.builder_id)
            return true;
        UNREACHABLE("operator==");
    }

    void operator^=(const FunctionBuilder& F) {
        setBorrowed(F);
    }

    //Returns a function that can evaluate the constructed builder
    Function buildFunction() const {
        Function return_value;
        switch (FunctionBuilderNode())
        {
        case FunctionBuilder::FunctionBuilderNodeTypes::Uninitialized: {
            PANIC("Uninitialized!");
            return {};
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::ConstantTerminal: {
            ASSERT(data().SizeVector() == 1, "Expecting 1 element!");
            Tuple ConstantValue = data().pickLast().GetCppType<Tuple>();
            return_value.SetConstant(ConstantValue);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::VariableTerminal: {
            ASSERT(data().SizeVector() == 0, "Expecting no element!");
            return_value.SetIdentity();
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPlus: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            data().AssertAllType<FunctionBuilder>("Expecting 2 FunctionBuilders!");
            IAny a1 = data().nthElementVector(0);
            FunctionBuilder FB1 = a1.GetCppType<FunctionBuilder>();
            Function F1 = FB1.buildFunction();
            Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetAddFunction(F1, F2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMinus: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetSubtractFunction(F1, F2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionProduct: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetProductFunction(F1, F2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionQuotient: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetQuotientFunction(F1, F2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMod: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetRemainderFunction(F1, F2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPointwiseLess: {
            ASSERT(data().SizeVector() == 4, "Expecting 4 elements!");
            Function value = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function test = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function alt_1 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            Function alt_2 = data().nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetPointwiseLess(value, test, alt_1, alt_2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPiecewiseLess: {
            ASSERT(data().SizeVector() == 3, "Expecting 3 elements!");
            Function alt_1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function threshold = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function alt_2 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetPiecewiseLess(threshold, alt_1, alt_2);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionLexicographicLess: {
            ASSERT(data().SizeVector() == 4, "Expecting 4 elements!");
            Function F0 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function F1 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Function F2 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
            Function F3 = data().nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetLexicographicLess(F0, F1, F2, F3);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionGetIndex: {
            ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
            Function tuple = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function index = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetGetIndex(index, tuple);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionSetIndex: {
            ASSERT(data().SizeVector() == 2, ((std::string)"Expecting 2 elements! Found: " + std::to_string(data().SizeVector())).c_str());
            Function tuple = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
            Function index = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            return_value.SetSetIndex(index, tuple);
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::FunctionBorrowedNode: {
            //Construct the BorrowNode
            ASSERT(data().SizeVector() == 3, "Expecting 3 elements!");
            //Get the stored function object:
            //Get the reference to the recursive call
            FunctionBuilder top_builder = (data().nthElementVector(0).GetCppType<FunctionBuilder>());
            ASSERT(top_builder.FunctionBuilderNode() == FunctionBuilderNodeTypes::SetBorrowFrom, "Must be set before construction");
            //Retrieve the stored function object from the recursive call object
            Function top = top_builder.data().nthElementVector(0).GetCppType<Function>();
            Function shrink = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
            Tuple    basecase = data().nthElementVector(2).GetCppType<Tuple>();
            return_value.SetBorrow(shrink, basecase, top);
            
            //The "set" value, containing the final recursive call
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::UnsetBorrowFromNode: {
            PANIC("Construction of an unset borrow-from node. Must set it first");
            UNREACHABLE("Panic");
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::SetBorrowFrom: {
            //The returned function_id must match the function_id of the stored function
            //Get the target functionbuilder
            FunctionBuilder target = data().nthElementVector(1).GetCppType<FunctionBuilder>();
            //This will call the "build" on the recursive operation
            //This call will call the switch at FunctionBuilder::FunctionBuilderNodeTypes::FunctionBorrowedNode
            // which will receive the zeroth element of this data, which is the function constructed here

            //id of return_value is this
            return_value = data().nthElementVector(0).GetCppType<Function>();
            
            //values associated with return_value is this
            //return_value.copyFunction(target.buildFunction());
            Function target_data = target.buildFunction();

            return_value.NodeClass() = target_data.NodeClass();
            return_value.FunctionNodes() = target_data.FunctionNodes();
            break;
        }
        case FunctionBuilder::FunctionBuilderNodeTypes::CallFunctionNode: {
            ASSERT(data().SizeVector() == 1, "Expecting 1 elements!");
            Function F0 = data().nthElementVector(0).GetCppType<Function>();
            return F0;
        }
        default:
            break;
        }
        if (name() != "")
            return_value.name() = name();
        return return_value;
        UNREACHABLE("Unreachable code!");
    }

    std::string print_dbg() {
        TODO_("Improve function builder");
        std::stringstream s;
        s << "Type: " << (int)FunctionBuilderNode() << " ";
        if (FunctionBuilderNode() == FunctionBuilderNodeTypes::VariableTerminal) {
            s << "x";
            return s.str();
        }
        else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::ConstantTerminal) {
            s << "build: [" + data().pickLast().GetCppType<Tuple>().to_str() + "]";
            return s.str();
        }
        else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::UnsetBorrowFromNode) {
            s << "recurse: [" "]";
            return s.str();
        }
        else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::FunctionBorrowedNode) {
            s << "recurse: [" "]";
            return s.str();
        }
        else {
            s << "Elements: [";
            IMany::MonoVoidFunction PrintRecursive = [&s](IAny element) -> void {
                if (+element.HasCPPType<FunctionBuilder>()) {
                    s << element.GetCppType<FunctionBuilder>("").print_dbg() << ", ";
                    return;
                }
                else
                    return;
            };
            data().OperateElement(PrintRecursive);
            s << "]";
            return s.str();
        }
    }

private:
    FunctionBuilder ReplaceIdentity(const FunctionBuilder& F) const {
        if (FunctionBuilderNode() == FunctionBuilderNodeTypes::VariableTerminal) {
            return F;
        }
        else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::ConstantTerminal) {
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
            FB.data() = data().TransformMap(ReplaceRecursive);
            //Everything else is unchanged, only data() is recursively transformed
            return FB;
        }
        UNREACHABLE("If-else");
    }

    //Sets a borrowed-from node to its value
    void setBorrowed(const FunctionBuilder& F) {
        number x = builder_id;
        ASSUME(FunctionBuilderNode() == FunctionBuilderNodeTypes::UnsetBorrowFromNode, "Can only set Borrowed-from nodes!");
        ASSUME(F.FunctionBuilderNode() != FunctionBuilderNodeTypes::UnsetBorrowFromNode, "Cannot set to Borrowed-from nodes!");
        //When building it, return this function
        //When building recursive calls to this, use this function for "top"
        data().clear();
        data().InsertTypeEnd<Function>(Function());
        data().InsertTypeEnd(F);
        FunctionBuilderNode() = FunctionBuilderNodeTypes::SetBorrowFrom;
        ASSUME(FunctionBuilderNode() == FunctionBuilderNodeTypes::SetBorrowFrom, "It remains a recursive call!");
        ASSUME(builder_id == x, "builder_id should not change!");
    }

    struct SetTypeOP;
    friend class LexicographicCase2;
    friend class PointwiseCase2;
    friend class PiecewiseCase1;
    friend class SetType;
    friend FunctionBuilder operator||(LexicographicCase2 PC, FunctionBuilder alt_2);
    friend FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2);
    friend FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_2);

    //Build expression a[i] < b[i] ? c[i] : d[i]
    static FunctionBuilder BuildPointwiseLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt1, const FunctionBuilder alt2) {
        ASSUME(isNotUninitialized(value), "Must be initialized");
        ASSUME(isNotUninitialized(test), "Must be initialized");
        ASSUME(isNotUninitialized(alt1), "Must be initialized");
        ASSUME(isNotUninitialized(alt2), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(value);
        result.data().InsertTypeEnd<FunctionBuilder>(test);
        result.data().InsertTypeEnd<FunctionBuilder>(alt1);
        result.data().InsertTypeEnd<FunctionBuilder>(alt2);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPointwiseLess;
        return result;
    }

    //Build expression i < index ? alt_1[i] : alt_2[i]
    static FunctionBuilder BuildPiecewiseLess(const FunctionBuilder alt_1, const FunctionBuilder index, const FunctionBuilder alt_2) {
        ASSUME(isNotUninitialized(alt_1), "Must be initialized");
        ASSUME(isNotUninitialized(index), "Must be initialized");
        ASSUME(isNotUninitialized(alt_2), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(alt_1);
        result.data().InsertTypeEnd<FunctionBuilder>(index);
        result.data().InsertTypeEnd<FunctionBuilder>(alt_2);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPiecewiseLess;
        return result;
    }

    //Build expression a < b ? c : d
    static FunctionBuilder BuildLexicographicLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt_1, const FunctionBuilder alt_2) {
        ASSUME(isNotUninitialized(value), "Must be initialized");
        ASSUME(isNotUninitialized(test), "Must be initialized");
        ASSUME(isNotUninitialized(alt_1), "Must be initialized");
        ASSUME(isNotUninitialized(alt_2), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(value);
        result.data().InsertTypeEnd<FunctionBuilder>(test);
        result.data().InsertTypeEnd<FunctionBuilder>(alt_1);
        result.data().InsertTypeEnd<FunctionBuilder>(alt_2);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionLexicographicLess;
        return result;
    }

    //Build Expression result[0] = lhs[index[0]]
    FunctionBuilder GetIndex(const FunctionBuilder index) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(index), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(index);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionGetIndex;
        return result;
    }

    //Build Expression result[index[0]] = lhs[0]
    FunctionBuilder SetIndex(const FunctionBuilder index) const {
        const FunctionBuilder& lhs = *this;
        ASSUME(isNotUninitialized(lhs), "Must be initialized");
        ASSUME(isNotUninitialized(index), "Must be initialized");
        FunctionBuilder result;
        result.data().InsertTypeEnd<FunctionBuilder>(lhs);
        result.data().InsertTypeEnd<FunctionBuilder>(index);
        result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionSetIndex;
        return result;
    }

public:
    FunctionBuilder operator[](SetTypeOP S) const;
};

std::unordered_map<number, IMany> FunctionBuilder::data_static{};
number FunctionBuilder::max_builder_id = 0;
std::unordered_map<number, FunctionBuilder::FunctionBuilderNodeTypes> FunctionBuilder::node_types;
std::unordered_map<number, std::string> FunctionBuilder::names;

FunctionBuilder const_(FunctionBuilder::PolyTuple i) {
    return i;
}

const FunctionBuilder var_ = FunctionBuilder::Variable();

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
    PointwiseCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1);
    friend FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2);
};

PointwiseCase1::PointwiseCase1(FunctionBuilder value, FunctionBuilder test) : value(value), test(test) {}
PointwiseCase2::PointwiseCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1) : test(test), value(value), alt_1(alt_1) {}

PointwiseCase2 operator||(PointwiseCase1 PC, FunctionBuilder alt_1) { return PointwiseCase2(PC.value, PC.test, alt_1); }
FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPointwiseLess(PC.value, PC.test, PC.alt_1, alt_2); }
PointwiseCase1 operator<(FunctionBuilder value, FunctionBuilder test) { return PointwiseCase1(value, test); }
PointwiseCase1 operator>(FunctionBuilder value, FunctionBuilder test) { return PointwiseCase1(test, value); }




//alt_1 | test | alt_2
class PiecewiseCase1;

class PiecewiseCase1 {
public:
    FunctionBuilder index;
    FunctionBuilder alt_1;

    PiecewiseCase1(FunctionBuilder value, FunctionBuilder test);
    friend FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_1);
};

PiecewiseCase1::PiecewiseCase1(FunctionBuilder alt_1, FunctionBuilder index) : alt_1(alt_1), index(index) {}

FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPiecewiseLess(PC.alt_1, PC.index, alt_2); }
PiecewiseCase1 operator|(FunctionBuilder alt_1, FunctionBuilder index) { return PiecewiseCase1(alt_1, index); }


class LexicographicCase1;
class LexicographicCase2;

class LexicographicCase1 {
public:
    FunctionBuilder test;
    FunctionBuilder value;

    LexicographicCase1(FunctionBuilder value, FunctionBuilder test);
    friend LexicographicCase2 operator||(LexicographicCase1 PC, FunctionBuilder alt_1);
};

class LexicographicCase2 {
    FunctionBuilder test;
    FunctionBuilder value;
    FunctionBuilder alt_1;
public:
    LexicographicCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1);
    friend FunctionBuilder operator||(LexicographicCase2 PC, FunctionBuilder alt_2);
};

LexicographicCase1::LexicographicCase1(const FunctionBuilder value, const FunctionBuilder test) : value(value), test(test) {}
LexicographicCase2::LexicographicCase2(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt_1) : test(test), value(value), alt_1(alt_1) {}

LexicographicCase2 operator||(const LexicographicCase1 PC, const FunctionBuilder alt_1) { return LexicographicCase2(PC.value, PC.test, alt_1); }
FunctionBuilder operator||(const LexicographicCase2 PC, const FunctionBuilder alt_2) { return FunctionBuilder::BuildLexicographicLess(PC.value, PC.test, PC.alt_1, alt_2); }
LexicographicCase1 operator<<(const FunctionBuilder value, const FunctionBuilder test) { return LexicographicCase1(value, test); }
LexicographicCase1 operator>>(const FunctionBuilder value, const FunctionBuilder test) { return LexicographicCase1(test, value); }

struct FunctionBuilder::SetTypeOP {
    FunctionBuilder F;
    SetTypeOP(FunctionBuilder F) : F(F) {}
};

struct SetType {
    FunctionBuilder::SetTypeOP operator>>(const FunctionBuilder F) const {
        return FunctionBuilder::SetTypeOP(F);
    }
} set;

FunctionBuilder FunctionBuilder::operator[](const SetTypeOP S) const {
    return FunctionBuilder::SetIndex(S.F);
}

//If Test(x) == case_, return Value(x), else return Main(x);
static FunctionBuilder CreateCase(Tuple case_, FunctionBuilder Value, FunctionBuilder Main, FunctionBuilder Test) {
    FunctionBuilder Identity = var_;
    FunctionBuilder Case = case_;
    FunctionBuilder IsLess = Test << Case || Main || Value;
    FunctionBuilder IsMore = Case << Test || Main || IsLess;
    return IsMore;
}

//base[index] = value, base[!index] = base
static FunctionBuilder CreatePiecewiseAt(FunctionBuilder base, FunctionBuilder index, FunctionBuilder value) {
    //F[-Inf:Index -1] = base; F[Index:Index] = value; F[Index +1:Inf] = base;
    FunctionBuilder FinalResult = base | index | value | index + 1 | base;
    return FinalResult;
}

//Return a function that performs pointwise x < 0 ? -x : x
static FunctionBuilder PointwiseAbs(FunctionBuilder base) {
    FunctionBuilder abs = var_ < 0 || -base || base;
    return abs;
}

//retrieve an overestimate for the maxHeight for a tuple
static FunctionBuilder MaxHeight() {
    //C++ algorithm:
    number i = 0;
    // guard against var_[0:Inf] = 0 -> Return 0
    // y = Abs var_
    // z = set y[0] to 0
    //While shrinking z: increment z[0] and check if 0 | z[0] | z << var_
    //  If so,  return z[0] from the borrow
    //  If not, continue shrinking

    //[0: 5, 1: 1, 2: 1, 3: 1, 4: 1, 5: 1, 6: 1, 7: 1, 8: 1]
    //[0: 0, 1: 1, 2: 1, 3: 1, 4: 1, 5: 1, 6: 1, 7: 1, 8: 1]
    //[0: 1, 1: 1, 2: 1, 3: 1, 4: 1, 5: 1, 6: 1, 7: 1, 8: 1]
    //[0: 2, 2: 1, 3: 1, 4: 1, 5: 1, 6: 1, 7: 1, 8: 1]
    //[0: 3, 3: 1, 4: 1, 5: 1, 6: 1, 7: 1, 8: 1]

    FunctionBuilder top = FunctionBuilder::BorrowFrom();
    FunctionBuilder y = PointwiseAbs(var_);
    FunctionBuilder z = CreatePiecewiseAt(y, 0, 0);
    z.setName("z");

    //Increment the base
    FunctionBuilder Shrink1 = z + 1;
    //Splice zeroes from 1 to z[0]
    FunctionBuilder Shrink2 = Shrink1 | const_(1) | const_(0);
    FunctionBuilder Shrink3 = Shrink2 | z[0] | z;
    //If Shrink2 << (0, 1), return Shrink2[0]; else return top(Shrink2)
    top ^= z << const_({ 0, 1 }) || 1 || top(Shrink3, 1) + 1;



    return top;
}

//A tuple variable that always evaluates f[input_] = value
//Can be used to represent types
class ConstrainedTuple {
    Function Process;
    Tuple Target;
public:
    ConstrainedTuple(Function Process, Tuple Target) : Process(Process), Target(Target) {}
    void AssertTuple(Tuple T) {
        Tuple test = Process(T);
        IComparisonResult x = Tuple::isEqual(test, Target);
        if (+x) {
            return;
        }
        else if (!x) {
            std::string message = "";
            message += "Assertion failure!";
            message += "\nThe tested Tuple is: ";
            message += T.to_str();
            message += "\nThe function evaluation is: ";
            message += test.to_str();
            message += "\nThe target is: ";
            message += Target.to_str();
            PANIC(message.c_str());
            return;
        }
        else {
            UNREACHABLE("Is known");
        }
    }
};

//Mini-class to aid inline construction
class ConstrainedTupleTarget {
    friend class ConstrainedTuple;
public:
    Tuple target;
    ConstrainedTupleTarget(FunctionBuilder::PolyTuple input) : target(input) {}
};

ConstrainedTuple operator&=(FunctionBuilder fB, ConstrainedTupleTarget ctt) {
    return ConstrainedTuple{ fB.buildFunction(), ctt.target };
}

ConstrainedTupleTarget ctt_(FunctionBuilder::PolyTuple input) { return ConstrainedTupleTarget(input); }

//Convert a string to a tuple
Tuple parse_tuple(const std::string& is)
{
    TODO;
    Tuple T;
    using boost::spirit::qi::stream;
    using boost::spirit::qi::stream_parser;
    using boost::spirit::qi::parse;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::space;
    using boost::phoenix::push_back;
    using boost::phoenix::ref;
    using boost::phoenix::placeholders::_1;
    //stream_parser<char, number> num_parser;
    auto num_parser = char_;

    std::vector<number> temp_num;
    auto parse_action = [&](auto& ctx)->void {
        temp_num.push_back(_attr(ctx));
    };

    //parse(is.begin(), is.end(), char_('[') >> *((+(char_ >> char_(':'))>> char_) % char_(',')) >> char_(']'));



    return T;
}

class SequencedCall;

//Tie multiple tuples into 1
class SequenceTie {
    IMany Tuples;
public:
    //Retrieve the tied tuple for evaluation
    Tuple ExecuteTie() {
        //Convert a, b, c, into:
        //-3: a[-1], -2: b[-1], -1: c[-1], 0: a[0], 1: b[0], 2: c[0], 3: a[1], ...
        //Map index i of Tuple j to j + (i * n)
        Tuples.AssertAllType<Tuple>("Must store tuples!");
        const number n = Tuples.SizeVector();
        if (n == 0) {
            return Tuple{};
        }
        auto map_target_index = [n](number in_index, number in_tuple)->number { return (in_index * n) + in_tuple; };
        auto map_get_index = [n](number out_index)->std::pair<number, number> { return { out_index / n, out_index % n }; };
        number maxHeight = 0;
        number minHeight = 0;
        for (const IAny& Tx : Tuples.VectorIterate()) {
            const Tuple& T = Tx.GetCppType<Tuple>("Must only store Tuples!");
            using namespace std;
            maxHeight = max(maxHeight, Tuple::getMaxHeight(T).GetNumberConst_Index(0));
            minHeight = min(minHeight, Tuple::getMinHeight(T).GetNumberConst_Index(0));
        }
        Tuple ret;
        for (number i = (minHeight - 1) * n; i <= maxHeight * n; i++) {
            number a = 0, b = 0;
            auto z = map_get_index(i);
            a = z.first; b = z.second;
            ASSERT(map_target_index(a, b) == i, "Must be reversible functions");
            Tuple get = Tuples.nthElementVector(b).GetCppType<Tuple>("Must store Tuples!");
            ret.GetNumber_Index(i) = get.GetNumber_Index(a);
        }
        return ret;
    }
    //A function for retrieving the nth Tuple when max are stored
    static FunctionBuilder FunctionBuilderUntie(number n, number max) { 
        TODO;
    }
    SequenceTie(std::initializer_list<Tuple> l) : Tuples(PolyMany(l)) {}  //Construct a collection of variables
};

//a + b = (Untie(0) + Untie(1))(SequenceTie(a, b))

//Represent variables, tuples, streams etc
//Allocate within the "function tuple"
//Each terminal is a name for a piece of a Tuple; dehardcode the allocator
//Have explicit "tie" and "get" functions for Tuples -> Tuple and Tuple -> Tuples
class SequencedTerminal {
public:
    auto operator=(SequencedTerminal) = delete;
    static number max_id;
    number id;

    SequencedTerminal() { id = ++max_id; }
    friend bool operator==(const SequencedTerminal& lhs, const SequencedTerminal& rhs) { return lhs.id == rhs.id; }
};
number SequencedTerminal::max_id = 0;

//A sequence with associated code
//Has simple sentences (SequenceTerminal = Function(SequenceTerminal))
//If-else statements (if F[a] == b) {} else {};
//Strong loops (for(i = 0; i != n; i++) {no writes to i};)
//Weak loops (while(...) {}) (Maybe runs forever)
class SequencedExecution {
public:
    Sequence Flow;
    enum class SequencedExecutionTypes: int {
        Uninitialized,  //Unitialized variable (not to be used)
        SimpleAssign,   //Terminal = Function(terminals) (void)
        Stream,         //Read from a stream into a terminal
        Chain,          //A list of SequencedExecutions (void)
        If,             //A forward branch (has a mandatory else) (void)
        While,          //A while loop, arbitrary strength (void)
        For,            //A strong for loop (i = 0 to n, i++) { no writes to i; } (void)
    } ExecutionType;
    IMany data;
    static number max_id;
    number id;

public:
    SequencedExecution() { id = ++max_id; ExecutionType = SequencedExecutionTypes::Uninitialized; }
    SequencedExecution(std::initializer_list<SequencedExecution>) : SequencedExecution() {};
    SequencedCall operator()(std::initializer_list<SequencedTerminal>);
    //Identity
    friend bool operator==(const SequencedExecution& lhs, const SequencedExecution& rhs) { return lhs.id == rhs.id; }
};
number SequencedExecution::max_id = 0;

//Represent an expression, involving (possibly) multiple SequencedTerminals
class SequencedFunctionCall {
    IMany vars;                     //Store a list of SequencedFunctionCalls
    FunctionBuilder var_Function;   //The function over the tied together variables

public:
    SequencedFunctionCall() {}
    //Build a functioncall of two functioncalls
    SequencedFunctionCall(SequencedTerminal st) : vars(PolyMany({ st })) { var_Function = var_; }
    friend SequencedFunctionCall operator+(SequencedFunctionCall lhs, SequencedFunctionCall rhs) {
        SequencedFunctionCall SFC;
        SFC.vars.clear();
        SFC.vars.InsertTypeEnd(lhs);
        SFC.vars.InsertTypeEnd(rhs);
        SFC.var_Function = SequenceTie::FunctionBuilderUntie(0, 2) + SequenceTie::FunctionBuilderUntie(1, 2);
        //TODO_("Fix");
        return SFC;
    }
    friend SequencedFunctionCall operator+(SequencedFunctionCall lhs, SequencedTerminal rhs) { return operator+(lhs, SequencedFunctionCall(rhs)); }
    friend SequencedFunctionCall operator+(SequencedTerminal lhs, SequencedFunctionCall rhs) { return operator+(SequencedFunctionCall(lhs), rhs); }
    friend bool operator==(const SequencedFunctionCall& lhs, const SequencedFunctionCall& rhs) { return +IMany::CompareVector(lhs.vars, rhs.vars) and +(lhs.var_Function == rhs.var_Function); }
};

SequencedFunctionCall operator+(SequencedTerminal lhs, SequencedTerminal rhs) { return operator+(SequencedFunctionCall(lhs), SequencedFunctionCall(rhs)); }

//An execution-context defined manner of collapsing into a single value
//eg console input, network management etc
//Use sequences to lock its value as an optimization barrier
//Also use state-machine-esque types to lock the sequence
class SequencedStream {
    enum class StreamKinds {
        uninitialized,
        std_cin,
        std_cout,
    } StreamType = StreamKinds::uninitialized;
public:
    SequencedStream(PolyAny t) {
        if (t == &std::cin) {
            StreamType = StreamKinds::std_cin;
            return;
        }
        else if (t == &std::cout) {
            StreamType = StreamKinds::std_cout;
            return;
        }
        PANIC("Incorrect stream constructor");
    }
    SequencedStream(const SequencedStream&) = default;

    //Read the next tuple from cin
    Tuple ReadNextTuple() {
        CASES(StreamType,
            StreamKinds::std_cin, 
            StreamKinds::std_cout);
        if (StreamType == StreamKinds::std_cin) {
            std::string z;
            return parse_tuple(z);
        }
        else if (StreamType == StreamKinds::std_cout) {
            PANIC("Read from stdout");
            return Tuple{};
        }
        PANIC("UnknownStreamType!");
    }
    void WriteNextTuple(Tuple T) {
        CASES(StreamType,
            StreamKinds::std_cin,
            StreamKinds::std_cout);
        if (StreamType == StreamKinds::std_cin) {
            PANIC("Write to stdin");
            return;
        }
        else if (StreamType == StreamKinds::std_cout) {
            std::cout << T.to_str();
            return;
        }
        PANIC("UnknownStreamType!");
    }
    friend bool operator==(SequencedStream a, SequencedStream b) { return a.StreamType == b.StreamType; }
};

//Set a mutable variable, inside a Sequence
//LHS accepts only terminals
//RHS accepts expressions ie function calls
SequencedExecution operator<=(SequencedTerminal& lhs, SequencedFunctionCall rhs) {
    SequencedExecution ret;
    ret.ExecutionType = SequencedExecution::SequencedExecutionTypes::SimpleAssign;
    ret.data.clear();
    ret.data.InsertTypeEnd(lhs);
    ret.data.InsertTypeEnd(rhs);
    return ret;
}

//Represent the result of a SequenceExecution() - used as part of the definition
class SequencedCall {
    IMany terminal;
public:
    SequencedExecution operator()(std::initializer_list<SequencedExecution> l);
    SequencedCall(std::initializer_list<SequencedTerminal> l): terminal(PolyMany(l)) {}
};
//The function definition object resulting from a call f({parameters})
//Use by attaching ({definition...}) to it
SequencedCall SequencedExecution::operator()(std::initializer_list<SequencedTerminal> l) { return SequencedCall(l); }
//The function code resulting from f({parameters})({definition})
SequencedExecution SequencedCall::operator()(std::initializer_list<SequencedExecution> l) {
    SequencedExecution ret;
    ret.ExecutionType = SequencedExecution::SequencedExecutionTypes::Chain;
    ret.data.clear();
    for (const SequencedExecution& x : l) {
        ret.data.InsertTypeEnd(x);
    }
    return ret;
}

//Assign a constant to a terminal
SequencedExecution operator<=(SequencedTerminal Terminal, FunctionBuilder::PolyTuple Tuple_) {
    SequencedExecution SE;
    SE.ExecutionType = SequencedExecution::SequencedExecutionTypes::SimpleAssign;
    SE.data.clear();
    SE.data.InsertTypeEnd(Terminal);
    SE.data.InsertTypeEnd<Tuple>(Tuple_);
    return SE;
}

//Read from a stream to a terminal
SequencedExecution operator<<(SequencedTerminal ST, SequencedStream SS) {
    SequencedExecution SE;
    SE.data.clear();
    SE.ExecutionType = SequencedExecution::SequencedExecutionTypes::Stream;
    SE.data.InsertTypeEnd(ST);
    SE.data.InsertTypeEnd(SS);
    return SE;
}

//Used for return_ <= ...
class ReturnType{} return_;
//Set the return terminal
SequencedExecution operator<=(ReturnType, SequencedTerminal) { TODO; };

//Use operator() to get the condition: Function(Tuple) = Value
//Use another operator() to get the first alternative: if(cond) then alt_1
//Use another operator() to get the second alternative: if(cond) then alt_1 else alt_2
class SequenceIfAlt2 {
    Function F;
    SequencedTerminal input;
    Tuple target;
    SequencedExecution if_1;
public:
    SequencedExecution operator()(SequencedExecution if_2) {}
    SequenceIfAlt2(Function F, SequencedTerminal input, Tuple target, SequencedExecution if_1) :
        F(F), input(input), target(target), if_1(if_1) {}
};

class SequenceIfAlt1 {
    Function F;
    SequencedTerminal input;
    Tuple target;
public:
    SequenceIfAlt2 operator()(SequencedExecution if_1) { return SequenceIfAlt2(F, input, target, if_1); }
    SequenceIfAlt1(Function F, SequencedTerminal input, Tuple target): F(F), input(input), target(target) {}
};

SequenceIfAlt1 if_(Function F, SequencedTerminal input, Tuple target) { return SequenceIfAlt1(F, input, target); }

class SequenceWhile {
    Function F; SequencedTerminal input; Tuple target;
public:
    SequencedExecution operator()(SequencedExecution loop) { TODO; }
    SequenceWhile(Function F, SequencedTerminal input, Tuple target): F(F), input(input), target(target) {}
};

SequenceWhile while_(Function F, SequencedTerminal input, Tuple target) { return SequenceWhile(F, input, target); }


class SequenceFor {
    SequencedTerminal input, target;
public:
    SequencedExecution operator()(SequencedExecution loop) { TODO; }
    SequenceFor(SequencedTerminal input, SequencedTerminal target) : input(input), target(target) {}
};

SequenceFor for_(SequencedTerminal input, SequencedTerminal target) { return SequenceFor(input, target); }

int main() {

    using namespace c_star_star;


    return 0;


    SequencedTerminal a, b, c, n, i;
    SequencedStream cin_(&std::cin);
    SequencedExecution Fibonacci_f;
    Fibonacci_f({})({
        a <= 0,
        b <= 1,
        n << cin_,
        for_(i, n)({
            c <= a + b,
            a <= b,
            b <= c,
        }),
        return_ <= c,
	});

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
    //Function AckermannBorrowCall = Function::C*reateBorrowNode(AckermannBorrowValue, number(1), &Ackermann);
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

    //FunctionBuilder Ackermann = FunctionBuilder::BorrowFrom();
    //FunctionBuilder AckermannY0 = Ackermann(var_ - 1, 1);                       //Zeroth element
    //FunctionBuilder AckermannY1 = var_[1] - 1;                                  //First element
    //FunctionBuilder AckermannY = AckermannY0[0] + AckermannY1[set >> 1];        //Compose inner call
    //Ackermann ^= (var_[1] < 1) || var_[0] + 1 || Ackermann(AckermannY, 1);      //if m < 1; n + 1; else ...

    //Function AckermannF = Ackermann.buildFunction();
    //std::cout << AckermannF.print_Function() << '\n';
    //for (int m = 0; m < 4; m++) {
    //    for (int n = 0; n < 5; n++) {
    //        std::cout << AckermannF(FunctionBuilder::PolyTuple({ n, m })).to_str() << ' ';
    //    }
    //    std::cout << '\n';
    //}

    //std::cout << AckermannF(FunctionBuilder::PolyTuple({ 1, 4 })).to_str() << ' ';


    return 0;
}

#undef ASSERT
#undef ASSUME
#undef UNREACHABLE