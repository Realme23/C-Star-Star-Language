#include "../Main.h"
#include "IMacroList.h"



namespace c_star_star {
	namespace interpreter {

		//Builds IMany, with construct, append, lock and produce stages
		//Assert on violation

		//Build a new IMacroList
		IMacroList IMacroList::Newlist() { return IMacroList(); }
		//Append an IAny to the IMacroList (Must be unlocked)
		void IMacroList::Append(const c_star_star::polymorphic::IAny& A) {
			ASSUME(not _lock, "Append to locked IMacroList");
			prepared.InsertIAnyEnd(A);
		}
		template<typename T>
		//Append an IAny to the IMacroList (Must be unlocked)
		void IMacroList::Append(const T& t) {
			ASSUME(not _lock, "Append to locked IMacroList");
			prepared.InsertTypeEnd(t);
		}
		//Append an IMany to the IMacroList (Must be unlocked)
		void IMacroList::AppendMany(const c_star_star::polymorphic::IMany& A) {
			ASSUME(not _lock, "Append to locked IMacroList");
			prepared.InsertManyEnd(A);
		}
		//Lock the IMacroList, cannot append and must produce (Must be unlocked)
		void IMacroList::Lock() {
			ASSUME(not _lock, "Attempt to lock locked IMacroList");
			_lock = true;
		}
		//Produce the IMany (Must NOT be unlocked)
		c_star_star::polymorphic::IMany IMacroList::BuildIMany() {
			ASSUME(_lock, "Prepare from an unlocked IMacroList");
			return prepared;
		}


	}
}