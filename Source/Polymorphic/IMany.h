#pragma once
#include "../Main.h"
#include "../Polymorphic/IAny.h"
#include "../Datatypes/Number.h"

namespace c_star_star {
    namespace polymorphic {
        class IMany;
        class PolyMany;

        //A polymorphic container of IAnys
        //Implemented as a vector
        //Supports more advanced compares (ignore ordering, ignore repeats etc)
        //Support Map operations (Add an associative container)
        class IMany {
        public:

            //A class that represents a pair of vector iterators over IMany
            class VectorIterateClass {
                IMany* target;
            public:
                std::vector<IAny>::iterator begin();
                std::vector<IAny>::const_iterator begin() const;
                std::vector<IAny>::iterator end();
                std::vector<IAny>::const_iterator end() const;
                friend class IMany;
            };

            //A friend class that has easy construction; useful for function arguments
            friend class PolyMany;

            //A function from an IAny to an IAny
            using MonoAnyFunction = std::function<auto (IAny)->IAny>;

            //A function from an IAny to a void
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
            using MonoIndexFunction = std::function<auto (number_)->number_>;

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
            static IComparisonResult CompareMany(const IMany& lhs, const IMany& rhs, bool ordered, bool no_lhs_repeats, bool no_rhs_repeats, bool not_subset, bool not_superset, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Iterator pair as a vector
            VectorIterateClass VectorIterate();

            //Check if the two Manys have the same elements in the same order (1,2,3,4 == 1,2,3,4)
            //Check as if lhs and rhs are vectors
            static IComparisonResult CompareVector(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the two Manys have the same elements, with arbitrary repeats in both sets, in the same order, rhs may have more elements after the end (1,2,3,4 == 1,2,3,4,5)
            //Check as if rhs is an extension of lhs, as vectors
            static IComparisonResult CompareVectorExtension(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the two Manys have a (non-zero) common prefix
            static IComparisonResult CompareVectorPrefix(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the two Manys have a (non-zero) single common substring
            static IComparisonResult CompareVectorOverlap(const IMany& lhs, const IMany& rhs, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the two Manys have the same elements (with the same counts) in a possibly different order (1,2,3,4 == 1,3,2,4)
            //Check as if lhs and rhs are multisets
            static IComparisonResult CompareMultiset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the two Manys have the same elements, with arbitrary repeats, in any order (1,2,3,2,4 == 1,2,3,1,4 == 1,2,3,4)
            //Check as if lhs and rhs are sets
            static IComparisonResult CompareSet(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check as if the lhs, as a Set, is contained in rhs (arbitrary repeats on both sides) (1,2,3,4 == 5,4,3,4,3,2,1)
            static IComparisonResult CompareSubset(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the lhs set_intersection rhs has atleast 1 element (0,1,2,5 == 3,4,5,6) (1,2,3,4 != 5,6,7,8)
            static IComparisonResult CompareSetIntersection(const IMany& lhs, const IMany& rhs, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr);

            //Check if the rhs is a member of the lhs, if each member of the lhs was transformed
            static IComparisonResult IsMember(const IMany& lhs_container, const IAny& rhs_member, const MonoAnyFunction lhs_transform = nullptr);

            //Look for a member in a transformed range and return the index it was found at first
            //Return -1 if not found
            static size_t WhereMember(const IMany& lhs, const IAny& rhs_member, const MonoIndexFunction lhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr);

            //Remove duplicate IAnys: Elements are transformed by the transform function (no op) as fallback
            //MonoAnyFunction defines equivalence classes, it does not mutate the final container
            //Eg ([1, 2, 3, 4, 5], is_odd) => ([1, 2])
            //   ([10, 20, 30, 40, 50], (mod 3)) => ([10, 20, 30])
            //The order is retained
            static IMany RemoveDuplicates(const IMany& container, const MonoAnyFunction transform = nullptr);

            static IComparisonResult IsLesserIMany(IMany& lhs, IMany& rhs, bool isSmallerLess, bool isEqualOrStrictlyLess, const MonoIndexFunction lhs_index = nullptr, const MonoIndexFunction rhs_index = nullptr, const MonoAnyFunction lhs_transform = nullptr, const MonoAnyFunction rhs_transform = nullptr, const CompareAnyFunction is_less = nullptr);

            //Sort IManys: Elements are compared by the CompareAnyFunction
            //fallback is (differentiating within types is based on <, differentiating between types is somewhat random but same types compare equal)
            //Returns a copy
            static IMany SortElements(const IMany& container, const CompareAnyFunction is_less = nullptr);

            //Insert an IAny into the end of the vector
            void InsertIAnyEnd(IAny element);

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
            void InsertManyEnd(const IMany& container_rhs, const MonoAnyFunction rhs_transform = nullptr);

            //Transform each element of an IMany elementwise (nullptr for no transform)
            //Returns a copy
            IMany TransformMap(const MonoAnyFunction transform = nullptr) const;

            //Run a nullary transformation on each element of an IMany (nullptr for no transform)
            void OperateElement(const MonoVoidFunction operate = nullptr) const;

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

            void clear();

            //Ordered multiSet Union (lhs first unmodified, rhs elements after if not in lhs)
            static IMany Union(const IMany& lhs, const IMany& rhs);

            //Ordered multiSet Intersection (Remove from lhs elements that are in rhs, as many times as they're there)
            static IMany Intersection(const IMany& lhs, const IMany& rhs);

            //Ordered multiSet Difference (Preserves order)
            static IMany Difference(const IMany& lhs, const IMany& rhs);

            //Set size = number_ of unique elements
            size_t SizeSet() const;

            //Vector size = number_ of inserted elements
            size_t SizeVector() const;

            //Removes all IAny that satisfy a predicate
            void RemoveIf(const MonoBoolFunction predicate);

            //Return an unspecified element; no guarantees on randomness
            IAny pickLast() const;

            //Selects a random element; as a Set
            IAny pickRandomSet() const;

            //Return an unspecified element and remove it (and all copies of it)
            IAny popSet();

            //Return an unspecified element and remove it (retain other copies)
            IAny popVector();

            //Retrieves ith element from vector
            //Index according to indexing function and transform the returned result
            //Return IEmptyValue if it is out of scope
            IAny nthElementVector(const number_ index, const MonoIndexFunction indexing = nullptr, const MonoAnyFunction transform = nullptr) const;

            //Retrieves ith element from set (ignore previously seen transformed values)
            // eg ([1, 2, 3, 4, 5] mod 3 = [1, 2, 0]) index 2 = 0, index 3 = not found
            //Index according to indexing function and transform the returned result
            IAny nthElementSet(const number_ index, const MonoIndexFunction indexing = nullptr, const MonoAnyFunction transform = nullptr) const;

            template<typename T>
            void AssertAllType(std::string message) const {
                for (const IAny& element : container) {
                    ASSERT(element.HasCPPType<T>(), message.c_str());
                }
                return;
            }

        private:
            //Stored data
            std::vector<IAny> container;

            //A list of names; maps container index to names
            std::unordered_map<number_, std::string> names;

            //Retrieve the ith element, according to index and transform
            static IAny get_i_element(const IMany& hs, number_ index, const MonoIndexFunction hs_index = nullptr, const MonoAnyFunction hs_transform = nullptr);
        };

        //Construct IManys from heterogeneous lists
        //Use as the argument of a function to automatically receive an IMany from an initializer list
        class PolyMany {
            IMany data;
        public:
            PolyMany();
            //Construct from a heterogeneous list
            PolyMany(std::initializer_list<IAny> l);

            //Construct from a homogeneous list
            template<typename T>
            PolyMany(std::initializer_list<T> l) {
                for (auto&& x : l) {
                    data.container.push_back(IAny(x));
                }
            }

            operator IMany();
        };
    }
}