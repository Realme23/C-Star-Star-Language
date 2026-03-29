#include "Tuple.h"

namespace std {
	size_t hash<c_star_star::tuples::Tuple>::operator()(const c_star_star::tuples::Tuple& x) const
	{
		using namespace boost::multiprecision;
		using namespace std;
		return (size_t)(abs(((x.GetNumberConst_Index(0)) ^ x.GetNumberConst_Index(1))) % (1ULL << 31));
	}
}

size_t hash_value(c_star_star::tuples::Tuple x) {
	return std::hash<c_star_star::tuples::Tuple>()(x);
}

namespace c_star_star {
	namespace tuples {

		Tuple::Tuple(number_ a) { GetNumber_Index(0) = a; }

		//Construct from another Tuple
		Tuple::Tuple() { data[0] = 0; };
		Tuple::Tuple(const Tuple&) = default;
		Tuple::Tuple(Tuple&&) = default;
		Tuple& Tuple::operator=(const Tuple&) = default;
		Tuple& Tuple::operator=(Tuple&&) = default;

		//Set a tuple to a constant number_
		void Tuple::setIndividualNumber(Tuple& a, number_ value) {
			a = Tuple(value);
		}

		std::string Tuple::to_str() {
			std::string s = "[";
			if (+hasSingleLimb(*this, 0)) {
				return to_string(GetNumberConst_Index(0));
			}
			for (const std::pair<number_, number_>& entry : data) {
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
		bool operator==(const Tuple& a, const Tuple& b) {
			return +Tuple::isEqual(a, b);
		}

		//Is a Lexicographically< b? 
		bool operator<(const Tuple& a, const Tuple& b) {
			return +Tuple::isLess<false>(a, b);
		}

		//Is it zero?
		c_star_star::tribool::IComparisonResult Tuple::isZero(const Tuple& a) {
			for (const std::pair<number_, number_>& x : a.data) {
				if (x.second != 0) {
					return false;
				}
			}
			return true;
		}

		//Does it have only 1 non-zero value at the specified index?
		c_star_star::tribool::IComparisonResult Tuple::hasSingleLimb(const Tuple& a, const number_ at) {
			for (const std::pair<number_, number_>& x : a.data) {
				if (x.first != at and x.second != 0) {
					return false;
				}
			}
			return true;
		}

		//Are they fully equal?
		bool Tuple::isEqual(const Tuple& a, const Tuple& b) {
			for (const std::pair<number_, number_>& x : a.data) {
				if (b.GetNumberConst_Index(x.first) != x.second) {
					return false;
				}
			}
			for (const std::pair<number_, number_>& x : b.data) {
				if (a.GetNumberConst_Index(x.first) != x.second) {
					return false;
				}
			}
			return true;
		}

		//T[0], T[1]
		Tuple Tuple::ConstructPair(number_ a, number_ b) {
			Tuple T;
			T.GetNumber_Index(0) = a;
			T.GetNumber_Index(1) = b;
			return T;
		}

		//Get a reference to the number_ at an index
		number_& Tuple::GetNumber_Index(const number_& index) {
			if (data.find(index) == data.end()) {
				data.insert_or_assign(index, 0);
			}
			return data.at(index);
		}

		//Get the number_ at an index (no reference)
		number_ Tuple::GetNumberConst_Index(const number_& index) const {
			if (data.find(index) == data.end()) {
				return 0;
			}
			else {
				return data.at(index);
			}
		}

		//Return the maximum height for the tuple (The highest index with a non-zero value)
		//Returns (max, 0) as the max height if found, (0, 1) if not
		Tuple Tuple::getMaxHeight(const Tuple& a) {
			number_ max = 0;
			bool found = false;
			for (const std::pair<number_, number_>& entry : a.data) {
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
		//Returns (min, 0) as the min height if found, (0, 1) if not
		Tuple Tuple::getMinHeight(const Tuple& a) {
			number_ min = 0;
			bool found = false;
			for (const std::pair<number_, number_>& entry : a.data) {
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

		bool operator!=(const Tuple& a, const Tuple& b) {
			return not Tuple::isEqual(a, b);
		}
	}
}
