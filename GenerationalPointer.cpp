#include "GenerationalPointer.h"

namespace c_star_star {
	namespace pointers {
		std::unordered_map<void*, uint64_t> PointerMaps::generations;
		std::unordered_map<void*, uint64_t> PointerMaps::domains;
		std::unordered_map<void*, uint64_t> PointerMaps::domain_Generations;

		template<typename T>
		//A < operator for inserting into ordered sets etc
		constexpr bool operator<(GenerationalPointer<T> lhs, GenerationalPointer<T> rhs) {
			return lhs.raw_ptr < rhs.raw_ptr;
		}

		template<typename T>
		//Retrieve the number of pointers within a given domain
		constexpr size_t GenerationalPointer<T>::CheckDomain(uint64_t domain) {
			size_t count = std::count_if(PointerMaps::domains.begin(), PointerMaps::domains.end(), [=](const auto& e) {
				return e.second == domain;
				});
			return count;
		}

		//Create a new GenerationalPointer and perform all the associations necessary
		template<typename T>
		template<typename ...Q>
		constexpr GenerationalPointer<T> GenerationalPointer<T>::MakeNewGenerationalT(uint64_t domain, Q...q ...)
		{
			GenerationalPointer GP;
			GP.raw_ptr = getRaw(std::forward(q)...);
			if (PointerMaps::generations.find(GP.raw_ptr) != PointerMaps::generations.end())
				PointerMaps::generations.at(GP.raw_ptr)++;
			else
				PointerMaps::generations[GP.raw_ptr] = 1;
			GP.Gen = PointerMaps::generations[GP.raw_ptr];
			AssociateDomain(GP, domain);
			GP.domainID = domain;
			ASSERT_RUN(PanicCheckQuick(GP));
			return GP;
		}

		template<typename T>
		//Verify that the generation is valid, return bool
		constexpr static bool GenerationalPointer<T>::CheckGeneration(GenerationalPointer GP) {
			bool found = (PointerMaps::generations.contains(GP.raw_ptr));
			if (not found)
				return false;
			bool check = (GP.Gen == PointerMaps::generations.at(GP.raw_ptr));
			if (found and check)
				return true;
			else
				return false;
		}

		template<typename T>
		//Panic if the generation check does not return true
		constexpr static void GenerationalPointer<T>::PanicCheckGeneration(GenerationalPointer GP) {
			if (CheckGeneration(GP) == false)
				PANIC("Generation mismatch on PanicCheck!");
			else
				return;
		}

		template<typename T>
		//Delete a pointer, add it to the "freelist" if enabled
		constexpr static void GenerationalPointer<T>::DeleteNewGenerationalT(GenerationalPointer GP) {
			bool EraseResult = TryErase(GP);
			if (EraseResult == false)
				PANIC("Memory Error: Generation mismatch on deletion.");
		}

		template<typename T>
		//Try to erase if the generation checks pass, return true on success
		constexpr static bool GenerationalPointer<T>::TryErase(GenerationalPointer GP) {
			bool Check = CheckGeneration(GP);
			if (Check) {
				DeleteRaw(GP);
				DeassociateDomain(GP);
				return true;
			}
			else if (not Check) {
				return false;
			}
			UNREACHABLE("Check or not check");
		}

		template<typename T>
		//Delete the raw pointer
		constexpr static void GenerationalPointer<T>::DeleteRaw(GenerationalPointer GP) {
			delete GP.raw_ptr;
			PointerMaps::generations[GP.raw_ptr]++;
		}

		template<typename T>
		//Dereference after a panic check
		constexpr T& GenerationalPointer<T>::operator*() {
			PanicCheckGeneration(*this);
			return *raw_ptr;
		}

		template<typename T>
		//Given a GenerationalPointer and a domain, insert the GenerationalPointer into the domain
		constexpr static void GenerationalPointer<T>::AssociateDomain(GenerationalPointer GP, uint64_t domain) {
			PointerMaps::domains.insert({ GP.raw_ptr, domain });
		}

		template<typename T>
		//Given a GenerationalPointer and a domain, mark the pointer as associated
		constexpr static void GenerationalPointer<T>::DeassociateDomain(GenerationalPointer GP) {
			PointerMaps::domains.erase(GP.raw_ptr);
		}

		template<typename T>
		//Construct a new raw pointer, passing the constructor args
		template<typename... Q>
		constexpr static T* GenerationalPointer<T>::getRaw(Q... q) {
			return new T(std::forward(q)...);
		}
	}
}