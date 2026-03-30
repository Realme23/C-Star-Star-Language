#pragma once
#include "Number.h"
#include "../Utils/IComparison.h"

namespace c_star_star {
    namespace functions {
        class Function;
    };
};

namespace c_star_star {
	namespace tuples {

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

			//Is a Lexicographically< b? 
			friend bool operator<(const Tuple& a, const Tuple& b);

			friend bool operator!=(const Tuple& a, const Tuple& b);

			friend class c_star_star::functions::Function;

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

			//Lexicographically compare two Tuples
			//Returns true for a < b; false for a > b, weak for a == b
			template<bool weak = false>
			static c_star_star::tribool::IComparisonResult isLess(const Tuple& a, const Tuple& b);

			//Set a tuple to a constant number_
			static void setIndividualNumber(Tuple& a, number_ value);

			std::string to_str();

			//Is it zero?
			static c_star_star::tribool::IComparisonResult isZero(const Tuple& a);

			//Does it have only 1 non-zero value at the specified index?
			static c_star_star::tribool::IComparisonResult hasSingleLimb(const Tuple& a, const number_ at = 0);

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
		};

        //Lexicographically compare two Tuples
        //Returns true for a < b; false for a > b, weak for a == b
        template<bool weak>
        c_star_star::tribool::IComparisonResult Tuple::isLess(const Tuple& a, const Tuple& b) {
            if (+isEqual(a, b))
                return weak;

            Tuple startTa = getMaxHeight(a);
            Tuple startTb = getMaxHeight(b);
            Tuple endTa = getMinHeight(a);
            Tuple endTb = getMinHeight(b);

            //If a is zero, check the lead digit of b
            if (+isZero(a)) {
                number_ base = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
                if (base > 0)
                    return true;
                else if (base < 0)
                    return false;
                else if (base == 0)
                    UNREACHABLE("Lead digit should not be zero!");
            }

            //If b is zero, check the lead digit of a
            if (+isZero(b)) {
                number_ base = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
                if (base > 0)
                    return false;
                else if (base < 0)
                    return true;
                else if (base == 0)
                    UNREACHABLE("Lead digit should not be zero!");
            }
            ASSERT((not isZero(a)) and (not isZero(b)), "a and b should not be zero by this point!");

            //Get the number_ with the bigger "height"
            if (startTa.GetNumberConst_Index(0) > startTb.GetNumberConst_Index(0))
                return false;
            else if (startTa.GetNumberConst_Index(0) < startTb.GetNumberConst_Index(0))
                return true;

            //If they have the same height, compare the lead digit
            number_ a_lead = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
            number_ b_lead = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
            number_ a_tail = a.GetNumberConst_Index(endTa.GetNumberConst_Index(0));
            number_ b_tail = a.GetNumberConst_Index(endTb.GetNumberConst_Index(0));

            if (a_lead > b_lead) {
                return false;
            }
            else if (b_lead > a_lead) {
                return true;
            }
            else if (a_lead == b_lead) {
                //Same height, same lead digits
                //Iteratively compare smaller digits
                number_ index = startTa.GetNumberConst_Index(0);
                //Compare upto the lowest tail
                number_ end_index = std::min(endTa.GetNumberConst_Index(0), endTb.GetNumberConst_Index(0));
                while (index-- >= end_index) {
                    number_ a_indexed = a.GetNumberConst_Index(index);
                    number_ b_indexed = b.GetNumberConst_Index(index);
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
    template <> struct hash<c_star_star::tuples::Tuple>
    {
        size_t operator()(const c_star_star::tuples::Tuple& x) const;
    };
}

size_t hash_value(std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple> x);