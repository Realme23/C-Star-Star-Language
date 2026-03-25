#pragma once
#include "../Main.h"
#include "../Polymorphic/IAny.h"
#include "../Polymorphic/IMany.h"


namespace c_star_star {
	namespace interpreter {
        //Builds IMany, with construct, append, lock and produce stages
//Assert on violation
        class IMacroList {
        public:
            //Build a new IMacroList
            static IMacroList Newlist();

            //Append an IAny to the IMacroList (Must be unlocked)
            void Append(const c_star_star::polymorphic::IAny& A);

            template<typename T>
            //Append an IAny to the IMacroList (Must be unlocked)
            void Append(const T& t);

            //Append an IMany to the IMacroList (Must be unlocked)
            void AppendMany(const c_star_star::polymorphic::IMany& A);

            //Lock the IMacroList, cannot append and must produce (Must be unlocked)
            void Lock();

            //Produce the IMany (Must NOT be unlocked)
            c_star_star::polymorphic::IMany BuildIMany();

        private:
            c_star_star::polymorphic::IMany prepared;
            bool _lock = false;
        };


	}
}