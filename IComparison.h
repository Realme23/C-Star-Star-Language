#pragma once
#include "Main.h"

//A tribool type
//Supports true, false and unknown
//Supports asserting conversions to bool (asserts on Unknown)
//Default construction to unknown
class IComparisonResult;
//Utility class: Converts to and from IComparisonResult and bool
class IComparisonToBool;

class IComparisonResult {
public:
    //Default construct: set to unknown
    constexpr IComparisonResult();

    //Value construct: set to a bool
    constexpr IComparisonResult(const bool value);

    //Set to a bool value, is not unknown after being set
    constexpr void setBool(const bool value);

    //Set the value to true
    constexpr void setTrue();

    //Set the value to false
    constexpr void setFalse();

    //Set to unknown
    constexpr void setUnknown();

    //Check if it is not unknown
    constexpr bool isKnown() const;

    //Check if it is a specific bool (Return false if unknown)
    constexpr bool isBool(bool b) const;

    //Check if it is true (Return false if unknown)
    constexpr bool isTrue() const;

    //Check if it is false (Return false if unknown)
    constexpr bool isFalse() const;

    //Check if it is unknown
    constexpr bool isUnknown() const;

    //Get either the stored boolean, or a fallback value
    constexpr bool getBoolOr(const bool b) const;

    //Get the stored boolean, or throw an error
    constexpr bool getBoolOrAssert(const std::string& error) const;

    //Get bool or assert
    constexpr bool operator+() const;

    //Compare with another IComparisonResult variable (propagate unknown)
    constexpr static IComparisonResult Compare(const IComparisonResult& lhs, const IComparisonResult& rhs);

    //Construct a IComparisonResult from a bool
    constexpr static IComparisonResult ConstructBool(const bool b);

    //Construct a IComparisonResult to "Unknown"
    constexpr static IComparisonResult ConstructUnknown();

    //The "negative" of the Comparison Result; true <-> false, unknown -> unknown
    constexpr IComparisonResult Negation() const;

    //Call a generic bool + bool -> bool function on the stored types
    constexpr static IComparisonResult WrapUnknownsOP(const IComparisonResult& lhs, const IComparisonResult& rhs, bool function(bool, bool));

    friend std::ostream& operator<<(std::ostream&, const IComparisonResult&);

private:
    bool _isTrue;
    bool _isFilled;
};

//IComparisonResult that autoconverts to bool and from IComparisonResult
//Asserts if unknown is implicitly converted
class IComparisonToBool {
public:
    constexpr operator bool() const;
    constexpr operator IComparisonResult() const;
    constexpr IComparisonToBool(const bool value);
    constexpr IComparisonToBool(const IComparisonResult value);

    constexpr IComparisonResult getIComparisonResult() const;
private:
    IComparisonResult Result;
};


constexpr bool operator==(const IComparisonResult& lhs, const IComparisonResult& rhs);



//Boolean operators, unknown-propagating
//Boolean negation
constexpr IComparisonToBool operator!(const IComparisonResult& operand);
constexpr IComparisonToBool operator!(const IComparisonToBool& operand);
//Boolean and
constexpr IComparisonToBool operator&&(const IComparisonResult& lhs, const IComparisonResult& rhs);
constexpr IComparisonToBool operator&&(const IComparisonToBool& lhs, const IComparisonToBool& rhs);

//Boolean or
constexpr IComparisonToBool operator||(const IComparisonResult& lhs, const IComparisonResult& rhs);
constexpr IComparisonToBool operator||(const IComparisonToBool& lhs, const IComparisonToBool& rhs);
//Boolean xor
constexpr IComparisonToBool operator^(const IComparisonResult& lhs, const IComparisonResult& rhs);
constexpr IComparisonToBool operator^(const IComparisonToBool& lhs, const IComparisonToBool& rhs);
//Boolean equals comparison
constexpr IComparisonToBool operator==(const IComparisonToBool& lhs, const IComparisonToBool& rhs);

//Stream IComparisonResult to an ostream
std::ostream& operator<<(std::ostream& ostream, const IComparisonResult& result);
