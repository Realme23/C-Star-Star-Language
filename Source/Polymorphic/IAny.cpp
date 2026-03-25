#include "IAny.h"

namespace c_star_star {
	namespace polymorphic {

		//Is the fallback class for IAny and is in default constructed IAny
		//Always compares equal to and not-less-than itself
		class IEmptyValue;
		constexpr bool operator==(const IEmptyValue, const IEmptyValue) { return true; }
		constexpr bool operator<(const IEmptyValue, const IEmptyValue) { return false; }

		//Get the type_index of the value stored
		type_index IAny::GetStoredType() const {
			return typeid_of(data);
		}

		//Check if a type is stored by type_info object
		IComparisonResult IAny::IsCPPType(const type_index& T) const {
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

		//Extract either a stored value or an empty IAny
		any IAny::GetBoostAny() const {
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

		//If it stores data of the same C++ Type
		IComparisonResult IAny::isComparable(const IAny& lhs, const IAny& rhs) {
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
		IComparisonResult IAny::CompareOrFalse(const IAny& lhs, const IAny& rhs) {
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
				ASSUME((not (lhs.data == rhs.data)), "Only return false when unequal!");
			else
				UNREACHABLE("Unknown, true and false all checked.");

			return return_value;
		}

		//If it has the IEmptyValue value
		IComparisonResult IAny::isEmpty() const {
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

		//Default Constructor
		IAny::IAny() : data(IEmptyValue()) {};

		//Construct an empty IAny
		IAny IAny::ConstructEmpty() {
			return IAny(IEmptyValue());
		}

		//Compare two IAnys for equality
		bool operator==(const IAny& lhs, const IAny& rhs) { return +IAny::CompareOrFalse(lhs, rhs); }

		PolyAny::operator IAny() { return a; }
	}
}

size_t std::hash<c_star_star::polymorphic::IAny>::operator()(const c_star_star::polymorphic::IAny& operand) const {
	if (operand.GetStoredType() == typeid(int)) {
		return operand.GetCppType<size_t>();
	}
	if (operand.GetStoredType() == typeid(number_)) {
		return (size_t)(operand.GetCppType<number_>() % (1U << 31));
	}
	return operand.GetStoredType().hash_code();
}