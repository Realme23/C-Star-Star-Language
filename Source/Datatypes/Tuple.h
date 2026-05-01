#pragma once
#include "Number.h"
#include "../Utils/IComparison.h"

namespace c_star_star {
    namespace function {
        namespace core {
            class Function;
        }
    };
};

namespace c_star_star {
	namespace data_types {

		//A list of bigints
		//Set/get
		//BigInts: Math ops, comparison ops
		//Lexicographic comparisons but no "pointwise" comparisons
		//No "procedural" values (ie no infinite a[x] = x)
		//Allow variable indexing (ie a[x] = y) (bc it can be implemented with switch-cases) (Indexing by a tuple is a[b] := a[b[0]] only)
		class Tuple {
			using BinaryMathFunction = auto(*)(number_, number_)->number_;
			using UnaryMathFunction = auto(*)(number_)->number_;

			//Are they equal?
			friend bool operator==(const Tuple& a, const Tuple& b);
			friend bool operator!=(const Tuple& a, const Tuple& b);

			friend class c_star_star::function::core::Function;

		public:
            std::map<number_, number_> data;

			//Construct from a single number_
			//Provided number_ is stored at the 0 index
			Tuple(number_ a);

			//Construct from another Tuple
			Tuple();
			Tuple(const Tuple&);
			Tuple(Tuple&&);
			Tuple& operator=(const Tuple&);
			Tuple& operator=(Tuple&&);

			//Set a tuple to a constant number_
			static void setIndividualNumber(Tuple& a, number_ value);

			std::string to_str();

			//Is it zero?
			static c_star_star::interpreter::IComparisonResult isZero(const Tuple& a);

			//Does it have only 1 non-zero value at the specified index?
			static c_star_star::interpreter::IComparisonResult hasSingleLimb(const Tuple& a, const number_ at = 0);

			//Are they equal?
			static bool isEqual(const Tuple& a, const Tuple& b);

			//T[0], T[1]
			static Tuple ConstructPair(number_ a, number_ b);

			//Get a reference to the number_ at an index
			number_& GetNumber_Index(const number_& index);

			//Get the number_ at an index (no reference)
			number_ GetNumberConst_Index(const number_& index) const;

			//Return the maximum height for the tuple (The highest index with a non-zero value)
			//Returns (max, 0) as the max height if found, (0, 1) if not
			static Tuple getMaxHeight(const Tuple& a);

			//Return the minimum height for the tuple (The lowest index with a non-zero value)
			//Returns (0, min) as the min height if found, (1, _) if not
			static Tuple getMinHeight(const Tuple& a);

			//Remove all zero elements
			void normalize() {
				std::erase_if(data, [](auto x)->bool { return x.second == 0; });
				if (data.size() == 0)
					*this = Tuple();
				return;
			}
		};

        //A convenience class for constructing a tuple from a std::initializer_list
        class PolyTuple {
            Tuple T;
        public:
            PolyTuple(Tuple T);

            PolyTuple(number_ n);
            template<std::integral Int>
            PolyTuple(Int n) : T(number_(n)) {}
            operator Tuple() const;

            PolyTuple(std::initializer_list<number_> l);
        };
	}
}

namespace std {
    template <> struct hash<c_star_star::data_types::Tuple>
    {
        size_t operator()(const c_star_star::data_types::Tuple& x) const;
    };
}