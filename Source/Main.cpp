#include "Main.h"

#include "Asserts/ConfigReleaseChecked.h"
#include "Datatypes/Number.h"
#include "Utils/GenerationalPointer.cpp"
#include "Utils/IComparison.cpp"
#include "Polymorphic/IAny.cpp"
#include "Polymorphic/IMany.cpp"
#include "Interpreter/IMacroList.cpp"
#include "Datatypes/Tuple.cpp"
#include "Functions/Function.cpp"
#include "Functions/Sequence.cpp"

//TODOs:
//Remove all IManys after prototyping
//Use generationalPointers for regular pointers
//Postconditions/preconditions for everything: test only function-related values, test all class invariants, don't test interfunctional relations with conditions
//Change all numeric integer types to number_ (all indices etc)
//  Find library that's "optimized for small case"
//Convert everything to free functions (Add UFCS later)
//Combine SequenceBuilder and FunctionBuilder; Separate out Function and Sequence execution
//A class to implement the static max_id; map<id, data>; value() = map[id] pattern (Use a single max_id for all classes)


using namespace c_star_star::polymorphic;
using namespace c_star_star::number;
using namespace c_star_star::data_types;
using namespace c_star_star::interpreter;
using namespace c_star_star::function;

//Convert a string to a tuple
Tuple parse_tuple(const std::string& is)
{
    TODO;
    Tuple T;
    using boost::spirit::qi::stream;
    using boost::spirit::qi::stream_parser;
    using boost::spirit::qi::parse;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::space;
    using boost::phoenix::push_back;
    using boost::phoenix::ref;
    using boost::phoenix::placeholders::_1;
    //stream_parser<char, number_> num_parser;
    auto num_parser = char_;

    std::vector<number_> temp_num;
    auto parse_action = [&](auto& ctx)->void {
        temp_num.push_back(_attr(ctx));
    };

    //parse(is.begin(), is.end(), char_('[') >> *((+(char_ >> char_(':'))>> char_) % char_(',')) >> char_(']'));



    return T;
}



int main() {
    using namespace c_star_star::function::library;
    using namespace c_star_star::function::simple_executor;
    using namespace c_star_star::function::tie;
    using namespace c_star_star::data_types;

    using namespace c_star_star::sequence;


    return 0;
}
