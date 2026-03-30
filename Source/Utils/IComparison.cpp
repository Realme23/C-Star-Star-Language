#include "IComparison.h"

namespace c_star_star {
    namespace interpreter {
        //Start:
        //A interpreter type
        //Supports true, false and unknown
        //Supports asserting conversions to bool (asserts on Unknown)

        //Default construct: set to unknown
        constexpr IComparisonResult::IComparisonResult() : _isTrue(false), _isFilled(false) {
            ASSUME(isUnknown(), "Default constructed IComparisonResults must be unknown.");
        }
        //Value construct: set to a bool
        constexpr IComparisonResult::IComparisonResult(const bool value) : _isTrue(value), _isFilled(true) {
            ASSUME(_isFilled == true, "Value constructed IComparisonResults must be known.");
            ASSUME(_isTrue == value, "Value constructed IComparisonResults must be constructed to correct value.");
        }
        //Set to a bool value, is not unknown after being set
        constexpr void IComparisonResult::setBool(const bool value) {
            _isTrue = value;
            _isFilled = true;

            ASSUME(_isFilled == true, "Must construct to known value.");
            ASSUME(_isTrue == value, "Must construct to specified value.");
        }
        //Set the value to true
        constexpr void IComparisonResult::setTrue() {
            setBool(true);

            ASSUME(_isFilled == true, "Must construct to known value.");
            ASSUME(_isTrue == true, "Must construct to known true.");
        }
        //Set the value to false
        constexpr void IComparisonResult::setFalse() {
            setBool(false);

            ASSUME(_isFilled == true, "Must construct to known value.");
            ASSUME(_isTrue == false, "Must construct to false.");
        }
        //Set to unknown
        constexpr void IComparisonResult::setUnknown() {
            _isFilled = false;

            ASSUME(_isFilled == false, "Must construct to unknown.");
        }
        //Check if it is not unknown
        constexpr bool IComparisonResult::isKnown() const {
            bool return_value = _isFilled;
            if (return_value == true) {
                ASSUME(_isFilled == true, "Must be a filled value.");
            }
            return return_value;
        }
        //Check if it is a specific bool (Return false if unknown)
        constexpr bool IComparisonResult::isBool(bool b) const {
            bool return_value = isKnown() && (b == _isTrue);
            if (return_value == true) {
                ASSUME(_isFilled == true, "isBool() should imply filled.");
            }
            return return_value;
        }
        //Check if it is true (Return false if unknown)
        constexpr bool IComparisonResult::isTrue() const {
            bool return_value = isKnown() && isBool(true);
            if (return_value == true) {
                ASSUME(_isFilled == true, "True should imply filled.");
            }
            return return_value;
        }
        //Check if it is false (Return false if unknown)
        constexpr bool IComparisonResult::isFalse() const {
            bool return_value = isKnown() && isBool(false);
            if (return_value == true) {
                ASSUME(_isFilled == true, "False should imply filled.");
            }
            return return_value;
        }
        //Check if it is unknown
        constexpr bool IComparisonResult::isUnknown() const {
            bool return_value = not isKnown();
            if (return_value == true) {
                ASSUME(_isFilled == false, "isUnknown() should not imply isKnown().");
            }
            return return_value;
        }
        //Get either the stored boolean, or a fallback value
        constexpr bool IComparisonResult::getBoolOr(const bool b) const {
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
        constexpr bool IComparisonResult::getBoolOrAssert(const std::string& error) const {
            ASSUME(isKnown(), ("Boolean must be known at this point: " + error).c_str());
            return isTrue();
        }

        //Get bool or assert
        constexpr bool IComparisonResult::operator+() const {
            return getBoolOrAssert("Conversion to bool operator called!");
        }

        //Compare with another IComparisonResult variable (propagate unknown)
        constexpr IComparisonResult IComparisonResult::Compare(const IComparisonResult& lhs, const IComparisonResult& rhs) {
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
        constexpr IComparisonResult IComparisonResult::ConstructBool(const bool b) {
            IComparisonResult return_value;
            return_value.setBool(b);

            ASSUME(return_value.isKnown() == true, "Constructed from a bool should be known!");
            ASSUME(return_value.isBool(b) == true, "Constructed from a bool should be the correct value!");

            return return_value;
        }

        //Construct a IComparisonResult to "Unknown"
        constexpr IComparisonResult IComparisonResult::ConstructUnknown() {
            IComparisonResult return_value;
            return_value.setUnknown();

            ASSUME(return_value.isKnown() == false, "Constructed from unknown should not be known!");
            ASSUME(return_value.isTrue() == false, "Constructed from unknown should not be true!");
            ASSUME(return_value.isFalse() == false, "Constructed from unknown should not be false!");

            return return_value;
        }

        //The "negative" of the Comparison Result; true <-> false, unknown -> unknown
        constexpr IComparisonResult IComparisonResult::Negation() const {
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
        constexpr IComparisonResult IComparisonResult::WrapUnknownsOP(const IComparisonResult& lhs, const IComparisonResult& rhs, bool function(bool, bool)) {
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

        constexpr bool operator==(const IComparisonResult& lhs, const IComparisonResult& rhs) {
            return +IComparisonResult::Compare(lhs, rhs);
        }


        //IComparisonResult that autoconverts to bool
        //Asserts if unknown is implicitly converted

        constexpr IComparisonToBool::operator bool() const { return Result.getBoolOrAssert("IComparisonToBool implicitly converted."); }
        constexpr IComparisonToBool::operator IComparisonResult() const { return Result; }
        constexpr IComparisonToBool::IComparisonToBool(const bool value) : Result(value) {}
        constexpr IComparisonToBool::IComparisonToBool(const IComparisonResult value) : Result(value) {}

        constexpr IComparisonResult IComparisonToBool::getIComparisonResult() const { return Result; }

        //Boolean operators, unknown-propagating
        //Boolean negation
        constexpr IComparisonToBool operator!(const IComparisonResult& operand) {
            return operand.Negation();
        }
        constexpr IComparisonToBool operator!(const IComparisonToBool& operand) {
            return !operand.getIComparisonResult();
        }
        //Boolean and
        constexpr IComparisonToBool operator&&(const IComparisonResult& lhs, const IComparisonResult& rhs) {
            return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a and b; });
        }
        constexpr IComparisonToBool operator&&(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
            return lhs.getIComparisonResult() and rhs.getIComparisonResult();
        }

        //Boolean or
        constexpr IComparisonToBool operator||(const IComparisonResult& lhs, const IComparisonResult& rhs) {
            return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a or b; });
        }
        constexpr IComparisonToBool operator||(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
            return lhs.getIComparisonResult() or rhs.getIComparisonResult();
        }
        //Boolean xor
        constexpr IComparisonToBool operator^(const IComparisonResult& lhs, const IComparisonResult& rhs) {
            return IComparisonResult::WrapUnknownsOP(lhs, rhs, [](bool a, bool b) { return a not_eq b; });
        }
        constexpr IComparisonToBool operator^(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
            return lhs.getIComparisonResult() xor rhs.getIComparisonResult();
        }
        //Boolean equals comparison
        constexpr IComparisonToBool operator==(const IComparisonToBool& lhs, const IComparisonToBool& rhs) {
            return lhs.getIComparisonResult() == rhs.getIComparisonResult();
        }

        //Stream IComparisonResult to an ostream
        std::ostream& operator<<(std::ostream& ostream, const IComparisonResult& result) {
            CASES_TRUE(result.isUnknown(), result.isTrue(), result.isFalse());
            if (result.isUnknown())
                return ostream << "unknown";
            else if (result.isTrue())
                return ostream << "true";
            else if (result.isFalse())
                return ostream << "false";
            UNREACHABLE("Unknown, true or false");
        }
    }
}