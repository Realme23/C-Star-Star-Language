#pragma once
#include "../Main.h"
#include "../Utils/IComparison.h"

namespace c_star_star {
	namespace polymorphic {

		using IComparisonResult = interpreter::IComparisonResult;
		class IEmptyValue;
		class IAny;
		struct PolyAny;

		//Is the fallback class for IAny and is in default constructed IAny
		//Always compares equal to and not-less-than itself
		class IEmptyValue {};
		constexpr bool operator==(const IEmptyValue, const IEmptyValue);
		constexpr bool operator<(const IEmptyValue, const IEmptyValue);

		//A class for storing any CPP type with runtime polymorphism
		//Requires copy constructible, typeid, equality comparison and less-than comparison
		class IAny {
		public:
			//Get the type_index of the value stored
			type_index GetStoredType() const;

			//Check if a type is stored by type_info object
			IComparisonResult IsCPPType(const type_index& T) const;

			//Check if a type is stored (template parameter version)
			template<typename T>
			IComparisonResult HasCPPType() const;

			//Extract either a stored value or an empty IAny
			any GetBoostAny() const;

			//Return a T if it is stored, or assert otherwise
			//Return type needs to be known at compile-time
			template<typename T>
			T GetCppType(std::string assert_message = "") const;

			//If it stores data of the same C++ Type
			static IComparisonResult isComparable(const IAny& lhs, const IAny& rhs);

			//Compare with another any if the same C++Type is stored, return false otherwise
			static IComparisonResult CompareOrFalse(const IAny& lhs, const IAny& rhs);

			//If it has the IEmptyValue value
			IComparisonResult isEmpty() const;

			//Store a value into the IAny
			template<typename T>
			void storeAny(T t);

			//Default Constructor
			IAny();

			//Construct from any object, explicitly
			template<typename T>
			explicit IAny(const T& t);

			//Construct an empty IAny
			static IAny ConstructEmpty();

		private:
			//Stores the data of the IAny
			any data;
			friend struct std::hash<IAny>;
		};
		//Compare two IAnys for equality
		bool operator==(const IAny& lhs, const IAny& rhs);

		//Any with implicit casts from all const& t, used for accepting function parameters etc
		struct PolyAny {
			IAny a;
			template<typename T>
			PolyAny(const T& t) : a(t) {};
			operator IAny();
		};

		//Compare an IAny with another type
		template<typename T>
		bool operator==(const IAny& lhs, const T& rhs) {
			if (not lhs.HasCPPType<T>()) {
				return false;
			}
			return lhs.GetCppType<T>("Already tested") == rhs;
		}

		template<typename T>
		IComparisonResult IAny::HasCPPType() const {
			IComparisonResult return_value = IsCPPType(typeid(T));

			if (return_value == true)
				ASSUME(typeid_of(data) == typeid(T), "Return value true must imply stored type is correct type.");
			else if (return_value == false)
				ASSUME(typeid_of(data) != typeid(T), "Return value false must imply stored type is not correct type.");
			else
				UNREACHABLE("Either matches or doesn't. Total, no unknowns expected.");

			return return_value;
		}

		template<typename T>
		T IAny::GetCppType(std::string assert_message) const {
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

		template<typename T>
		IAny::IAny(const T& t) : data(t) {}
	}
}

template<>
struct std::hash<c_star_star::polymorphic::IAny> {
public:
	size_t operator()(const c_star_star::polymorphic::IAny& operand) const;
};