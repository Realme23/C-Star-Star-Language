#pragma once
#include "IMany.h"

namespace c_star_star {
	namespace polymorphic {
		//A polymorphic container of IAnys
		//Implemented as a vector
		//Supports more advanced compares (ignore ordering, ignore repeats etc)
		//Support Map operations (Add an associative container)

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
		IComparisonResult IMany::CompareMany(const IMany& lhs, const IMany& rhs, bool ordered, bool no_lhs_repeats, bool no_rhs_repeats, bool not_subset, bool not_superset, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
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

		std::vector<IAny>::iterator IMany::VectorIterateClass::begin() {
			return target->container.begin();
		}
		std::vector<IAny>::const_iterator IMany::VectorIterateClass::begin() const {
			return target->container.cbegin();
		}
		std::vector<IAny>::iterator IMany::VectorIterateClass::end() {
			return target->container.end();
		}
		std::vector<IAny>::const_iterator IMany::VectorIterateClass::end() const {
			return target->container.cend();
		}

		IMany::VectorIterateClass IMany::VectorIterate() {
			VectorIterateClass return_value;
			return_value.target = this;
			return return_value;
		}

		//true, true, true, true, true
		//Check if the two Manys have the same elements in the same order (1,2,3,4 == 1,2,3,4)
		//Check as if lhs and rhs are vectors
		IComparisonResult IMany::CompareVector(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			return CompareMany(lhs, rhs, true, true, true, true, true, lhs_index, rhs_index, lhs_transform, rhs_transform);
		}
		//Check if the two Manys have the same elements, with arbitrary repeats in both sets, in the same order, rhs may have more elements after the end (1,2,3,4 == 1,2,3,4,5)
		//Check as if rhs is an extension of lhs, as vectors
		IComparisonResult IMany::CompareVectorExtension(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			return CompareMany(lhs, rhs, true, false, false, true, true, lhs_index, rhs_index, lhs_transform, rhs_transform);
		}
		//Check if the two Manys have a (non-zero) common prefix
		IComparisonResult IMany::CompareVectorPrefix(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			TODO;
		}
		//Check if the two Manys have a (non-zero) single common substring
		IComparisonResult IMany::CompareVectorOverlap(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			TODO;
		}
		//Check if the two Manys have the same elements (with the same counts) in a possibly different order (1,2,3,4 == 1,3,2,4)
		//Check as if lhs and rhs are multisets
		IComparisonResult IMany::CompareMultiset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
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
		IComparisonResult IMany::CompareSet(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
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
		IComparisonResult IMany::CompareSubset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			TODO;
		}
		//false, true, true, false, false
		//Check if the lhs set_intersection rhs has atleast 1 element (0,1,2,5 == 3,4,5,6) (1,2,3,4 != 5,6,7,8)
		IComparisonResult IMany::CompareSetIntersection(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform) {
			TODO;
		}

		//Check if the rhs is a member of the lhs, if each member of the lhs was transformed
		IComparisonResult IMany::IsMember(const IMany& lhs_container, const IAny& rhs_member, const MonoAnyFunction lhs_transform) {
			for (const IAny& element : lhs_container.container) {
				const auto element_transformed = lhs_transform ? lhs_transform(element) : element;
				if (IAny::CompareOrFalse(element_transformed, rhs_member).getBoolOrAssert("Fallback called."))
					return IComparisonResult::ConstructBool(true);
			}
			return IComparisonResult::ConstructBool(false);
		}

		//Look for a member in a transformed range and return the index it was found at first
		//Return -1 if not found
		size_t IMany::WhereMember(const IMany& lhs, const IAny& rhs_member, const MonoIndexFunction lhs_index, const MonoAnyFunction lhs_transform) {
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
		IMany IMany::RemoveDuplicates(const IMany& container, const MonoAnyFunction transform) {
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
		IMany IMany::SortElements(const IMany& container, const CompareAnyFunction is_less) {
			IMany return_value;
			return_value.container = container.container;
			auto compareAny = [](IAny& a, IAny& b, const CompareAnyFunction& is_less) -> bool {
				TODO_("Implement IAny comparison or remove sorting");
				return true;
				};
			std::sort(return_value.container.begin(), return_value.container.end(), is_less);
			return return_value;
		}

		//Lexicographically compare IManys:
		//Compare the elements of lhs and rhs pairwise, until lhs_element != rhs_element, return true if lhs_element < rhs_element
		//isSmallerLess describes which result will be returned if 1 is a strict subset of the other
		//isEqualOrStrictlyLess describes which result will be returned if lhs is equal to the rhs
		IComparisonResult IMany::IsLesserIMany(IMany& lhs, IMany& rhs, bool isSmallerLess, bool isEqualOrStrictlyLess, const MonoIndexFunction lhs_index, const MonoIndexFunction rhs_index, const MonoAnyFunction lhs_transform, const MonoAnyFunction rhs_transform, const CompareAnyFunction is_less) {
			int i = 0, j = 0;
			for (i = 0, j = 0; i < lhs.container.size() and j < rhs.container.size(); i++, j++) {
				IAny l = get_i_element(lhs, i, lhs_index, lhs_transform);
				IAny r = get_i_element(rhs, j, rhs_index, rhs_transform);
				IComparisonResult l_equal = (l == r);
				if (not l_equal.isTrue()) {
					TODO_("Implement IAny comparison or remove sorting");
					IComparisonResult l_less = is_less ? is_less(l, r) : true;
					if (+l_less)
						return IComparisonResult::ConstructBool(true);
					else if (not l_less)
						return IComparisonResult::ConstructBool(false);
					else
						UNREACHABLE("Total Comparison of lesser or not. No unknowns expected.");
				}
			}
			if (i < lhs.container.size() or j < rhs.container.size())
				return IComparisonResult::ConstructBool(isSmallerLess);
			HEAVY_ASSERT(2, +CompareVector(lhs, rhs, lhs_index, rhs_index, lhs_transform, rhs_transform), "Must be same to reach this point!");
			return IComparisonResult::ConstructBool(isEqualOrStrictlyLess);
		}

		//Insert an IAny into the end of the vector
		void IMany::InsertIAnyEnd(IAny element) {
			size_t size = container.size();

			container.push_back(element);

			ASSUME(container.size() == size + 1, "Must have added an element!");
			ASSUME(container.back() == element, "Must have pushed element at the end!");
		}

		//Insert an element-wise transformation of another IMany at the end of this one (nullptr for no transform)
		void IMany::InsertManyEnd(const IMany& container_rhs, const MonoAnyFunction rhs_transform) {
			for (const IAny& element : container_rhs.container) {
				const auto element_transformed = rhs_transform ? rhs_transform(element) : element;
				InsertIAnyEnd(element_transformed);
			}
		}

		//Transform each element of an IMany elementwise (nullptr for no transform)
		//Returns a copy
		IMany IMany::TransformMap(const MonoAnyFunction transform) const {
			IMany result;
			for (const IAny& element : container) {
				const auto element_transformed = transform ? transform(element) : element;
				result.InsertIAnyEnd(element_transformed);
			}
			return result;
		}

		//Run a nullary transformation on each element of an IMany (nullptr for no transform)
		void IMany::OperateElement(const MonoVoidFunction operate) const {
			if (operate) {
				for (const IAny& element : container) {
					operate(element);
				}
			}
		}

		void IMany::clear() {
			container.clear();
		}

		//Ordered MultiSet Union (lhs first unmodified, rhs elements after if not in lhs)
		IMany IMany::Union(const IMany& lhs, const IMany& rhs) {
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
		IMany IMany::Intersection(const IMany& lhs, const IMany& rhs) {
			TODO;
		}
		//Ordered multiSet Difference (Preserves order)
		IMany IMany::Difference(const IMany& lhs, const IMany& rhs) {
			TODO;
		}
		//Set size = number_ of unique elements
		size_t IMany::SizeSet() const {
			std::unordered_set<IAny> z;
			for (const auto& element : container) {
				z.insert(element);
			}
			return z.size();
		}
		//Vector size = number_ of inserted elements
		size_t IMany::SizeVector() const {
			return container.size();
		}
		//Removes all IAny that satisfy a predicate
		void IMany::RemoveIf(const MonoBoolFunction predicate) {
			//Use std::erase_if
			std::erase_if(container, predicate);
			return;
		}
		//Return an unspecified element; no guarantees on randomness
		IAny IMany::pickLast() const {
			if (container.size() == 0) {
				return IAny{};
			}
			return container.back();
		}
		//Selects a random element; as a Set (affects weighting)
		IAny IMany::pickRandomSet() const {
			TODO;
		}
		//Return an unspecified element and remove it (and all copies of it)
		IAny IMany::popSet() {
			TODO;
		}
		//Return an unspecified element and remove it (retain other copies)
		IAny IMany::popVector() {
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
		IAny IMany::nthElementVector(const number_ index, const MonoIndexFunction indexing, const MonoAnyFunction transform) const {
			number_ k = indexing ? indexing(index) : index;
			ASSERT(k < (1ULL << 63), "Must be small!");
			size_t k_conv = boost::multiprecision::integer_modulus(k, 1ULL << 63);
			ASSERT((number_)k_conv == k, "Must convert better!");
			if (k_conv >= container.size())
				return IAny::ConstructEmpty();
			IAny result = container[k_conv];
			IAny return_value = transform ? transform(result) : result;
			return return_value;
		}
		//Retrieves ith element from set (ignore previously seen transformed values)
		// eg ([1, 2, 3, 4, 5] mod 3 = [1, 2, 0]) index 2 = 0, index 3 = not found
		//Index according to indexing function and transform the returned result
		IAny IMany::nthElementSet(const number_ index, const MonoIndexFunction indexing, const MonoAnyFunction transform) const {
			std::unordered_set<IAny> seen;
			int index2 = 0;
			for (int i = 0; i < container.size() && index2 < index; i++) {
				size_t k = c_star_star::number::to_size_t(indexing ? indexing(i) : i);
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
			size_t k = c_star_star::number::to_size_t(indexing ? indexing(index2) : index2);
			if (k >= container.size())
				return IAny::ConstructEmpty();
			IAny result = container[k];
			IAny return_value = transform ? transform(result) : result;
			return return_value;
		}


		//Retrieve the ith element, according to index and transform
		IAny IMany::get_i_element(const IMany& hs, number_ index, const MonoIndexFunction hs_index, const MonoAnyFunction hs_transform)
		{
			size_t new_index = c_star_star::number::to_size_t(hs_index ? hs_index(index) : index);
			return hs_transform ? hs_transform(hs.container[new_index]) : hs.container[new_index];
		};

		//Construct IManys from heterogeneous lists
		//Use as the argument of a function to automatically receive an IMany from an initializer list
		PolyMany::PolyMany() = default;

		//Construct from a heterogeneous list
		PolyMany::PolyMany(std::initializer_list<IAny> l) {
			for (auto&& x : l) {
				data.container.push_back(x);
			}
		}

		PolyMany::operator IMany() {
			return data;
		}
	}
}