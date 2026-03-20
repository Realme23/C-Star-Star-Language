#pragma once
#include "Source1.h"


namespace c_star_star {
    namespace pointers {
        //A class to hold pointer info, for GenerationalPointers
        class PointerMaps {
        public:
            static std::unordered_map<void*, uint64_t> generations;
            static std::unordered_map<void*, uint64_t> domains;
            static std::unordered_map<void*, uint64_t> domain_Generations;
            template<typename T>
            friend class GenerationalPointer;
        };


        //A single-threaded generational pointer class
        //Stores a pointer and a generation number; References a static map that tracks the generation for each allocation
        //Can associate pointers with a new tag, verify eg that all pointers allocated in a single frame are deallocated by the next frame
        //Verifies the correct generation number on each deallocation and dereference
        //Template parameters: (Default = no checks)
        //Pointed-to type T
        template<typename T>
        class GenerationalPointer {
        private:
            //Which allocation generation this address belongs to
            uint64_t Gen;
            //The actual pointer itself
            T* raw_ptr;
            //A domain associated to the pointer, used to track memory leaks
            uint64_t domainID;

        public:
            //A < operator for inserting into ordered sets etc
            constexpr friend bool operator<(GenerationalPointer lhs, GenerationalPointer rhs);

            //Retrieve the number of pointers within a given domain
            constexpr static size_t CheckDomain(uint64_t domain);

            //Create a new GenerationalPointer and perform all the associations necessary
            template<typename... Q>
            constexpr static GenerationalPointer MakeNewGenerationalT(uint64_t domain = 0, Q... q...);

            //Verify that the generation is valid, return bool
            constexpr static bool CheckGeneration(GenerationalPointer GP);

            //Panic if the generation check does not return true
            constexpr static void PanicCheckGeneration(GenerationalPointer GP);

            //Delete a pointer
            constexpr static void DeleteNewGenerationalT(GenerationalPointer GP);

            //Try to erase if the generation checks pass, return true on success
            constexpr static bool TryErase(GenerationalPointer GP);

            //Dereference after a panic check
            constexpr T& operator*();

        private:
            //Given a GenerationalPointer and a domain, insert the GenerationalPointer into the domain
            constexpr static void AssociateDomain(GenerationalPointer GP, uint64_t domain);

            //Given a GenerationalPointer and a domain, mark the pointer as associated
            constexpr static void DeassociateDomain(GenerationalPointer GP);

            //Construct a new raw pointer, passing the constructor args
            template<typename... Q>
            constexpr static T* getRaw(Q... q);

            //Delete the raw pointer
            constexpr static void DeleteRaw(GenerationalPointer GP);
        };
    }
}