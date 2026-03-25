#include "generationalpointer.h"

namespace c_star_star {
	namespace pointers {
		std::unordered_map<void*, uint64_t> PointerMaps::generations;
		std::unordered_map<void*, uint64_t> PointerMaps::domains;
		std::unordered_map<void*, uint64_t> PointerMaps::domain_Generations;

	}
}