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

//TODOs:
//Remove all IManys after prototyping
//Use generationalPointers for regular pointers
//Postconditions/preconditions for everything: test only function-related values, test all class invariants, don't test interfunctional relations with conditions
//Change all numeric integer types to number_ (all indices etc)
//  Find library that's "optimized for small case"
//Convert everything to free functions (Add UFCS later)
//Combine SequenceBuilder and FunctionBuilder; Separate out Function and Sequence execution
//A class to implement the static max_id; map<id, data>; value() = map[id] pattern (Use a single max_id for all classes)


//Design:
/*
Describe an imperative, low-level language over Tuples, with determinism, purity, and static_asserts
    Includes Sequences, Functions, Sequence Executions, Asserts
    The "type" of a sequence is the full definition of it, the type of a pure function is the map of its inputs to outputs
    Goal: describe pure, verifiable, multithreaded computations over all platforms/devices
Describe a "heritable meta-context" describing impure additions to the low-level language and hygienic macros
    Includes modules and a build system
    Module definitions are based on the high-level description
    Goal: build/package code, source-level security, describe meta/environment-information and generate/interpret/transform modules
        Meta/environment information includes exposing a piece of code as a new datatype to another module
    Design constraint: Move all computation to the low-level language and all specification in the higher-level language, include the interpreter
Describe a higher-level language, representing an "optimization contract" over the lower-level language
    describes the allowed optimizations of the implemented code
    include theories and static_asserts; describe the allowed/expected behaviours
    use constants from the heritable meta-context to describe compiler passes to apply different compiler passes
    can describe which plugins can be compiled together
*/

using namespace c_star_star::polymorphic;
using namespace c_star_star::number;
using namespace c_star_star::data_types;
using namespace c_star_star::interpreter;
using namespace c_star_star::functions;

//A tag class for sequence points
class SequencePoint {
    number_ id;
    friend class Sequence;
    friend struct std::hash<SequencePoint>;
public:
    bool operator==(const SequencePoint& other) const
    {
        return id == other.id;
    }
    bool operator!=(const SequencePoint& other) const
    {
        return !(*this == other);
    }
};

template<>
struct std::hash<SequencePoint> {
    size_t operator()(const SequencePoint& other) const
    {
        return size_t(other.id % (1ULL << 63));
    }
};

//A set of sequence-points, which are temporally ordered by a single "less than" partial order
//Irreflexive, antisymmetric, transitive
//Add points with a before relation
//Have potentially circular graphs, enforce numerical orderings and locks to assert on circularity
// Locked sequences cannot be further attached to or later unlocked
//TODO: Use tags and domains to enforce numerical locks (ie tag an event A with 4, when adding B after C (and C after A) assert that B's value > 4)
//TODO: Lock a domain permanently, no more inputs into it, and/or no more outputs from it
//TODO: Mix parallelizability and choice
//  Choice: GOTOs
//  Parallelizability: Run multiple choices together
class Sequence {
public:
    //Start a new sequencepoint, with nothing pointing to it
    SequencePoint newSequencePointStart() {
        FIXME("Add locking, not done yet");
        SequencePoint SP;
        SP.id = ++totalSequencePoints;
        return SP;
    }

    //Associate 2 preexisting sequence points (assert if already has an after)
    void insertNewLink(SequencePoint before, SequencePoint after) {
        ASSUME(not isInBeforeAfter(before), "Should not reassign already assigned SequencePoint!");
        ASSUME(not isInSelectionNodes(before), "Should not reassign already assigned SequencePoint!");
        beforeAfter.insert({ before, after });
    }

    //Create a new sequence point, associate it to "before" and return the new point
    SequencePoint newSequencePointFollower(SequencePoint before) {
        SequencePoint after = newSequencePointStart();
        insertNewLink(before, after);
        return after;
    }

    //Insert a new sequence point with many follows
    SequencePoint newSequencePointChoice(SequencePoint before, IMany sp) {
        SequencePoint after = newSequencePointStart();
        insertNewLink(before, after);
        selectionNodes.insert({ after, sp });
        return after;
    }

    //Follow a single sequence point
    //If no follower, return EmptyAny
    //Otherwise, if simple, return the next point
    //Otherwise, if comnplex, return the zeroth choice
    IAny followSequencePointSimple(SequencePoint before) {
        if (not isInBeforeAfter(before))
            return {};
        else if (isInBeforeAfter(before)) {
            SequencePoint after = beforeAfter.at(before);
            return IAny{ after };
        }
        UNREACHABLE("Cases covered!");
    }

    IAny followSequencePointChoice(SequencePoint before, number_ choice) {
        if (not isInSelectionNodes(before))
            return IAny{};
        IMany choices = selectionNodes[before];
        choices.AssertAllType<SequencePoint>("Must all be SequencePoints");
        if (choices.SizeVector() < to_size_t(choice))
            return IAny{};
        SequencePoint result = choices.nthElementVector(choice).GetCppType<SequencePoint>("Expected SequencePoint");
        return IAny{ result };
    }

    //Follow a choice point, by selecting a choice number_ (0 -> 0th choice, 1 -> 1st choice etc)
    //If the point is not a choice point, only 0 is a valid choice (simple follow)
    //If the index is invalid, return EmptyAny
    IAny followSequencePointAggregate(SequencePoint before, number_ choice) {
        if (choice < 0)
            return IAny{};

        if (not isInBeforeAfter(before) and not isInSelectionNodes(before))
            return IAny{};


        CASES_TRUE(isInBeforeAfter(before), isInSelectionNodes(before));
        if (isInBeforeAfter(before)) {
            if (choice > 0)
                return IAny{};
            return followSequencePointSimple(before);
        }
        else if (isInSelectionNodes(before)) {
            if (selectionNodes[before].SizeVector() < to_size_t(choice))
                return IAny{};

        }
    }

private:
    //A single-key multi-value map from after-points to before-points
    std::unordered_map<SequencePoint, SequencePoint> beforeAfter;
    //A list of nodes with multiple outputs
    std::unordered_map<SequencePoint, IMany> selectionNodes;
    //A number_ associated to each ordering, can be used to enforce far-apart before/after relations
    std::unordered_map<SequencePoint, number_> orderingAssertNumber;
    static number_ totalSequencePoints;

    //If beforeAfter has a "before"
    bool isInBeforeAfter(SequencePoint before) const {
        return not (beforeAfter.contains(before));
    }

    //If selectionNodes has a "before"
    bool isInSelectionNodes(SequencePoint before) const {
        return not (selectionNodes.contains(before));
    }
};

number_ Sequence::totalSequencePoints = 0;


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
    using namespace c_star_star::functions::library;
    using namespace c_star_star::functions::simple_executor;
    using namespace c_star_star::functions::tie;
    using namespace c_star_star::data_types;

    Tuple input_x = PolyTuple({ 10, 20, 30 });
    Tuple input_y = PolyTuple({ 60, 70 });
    Tuple input_z;
    input_z.GetNumber_Index(-200) = 10;
    input_z.GetNumber_Index(0)    = 11;
    input_z.GetNumber_Index(300)  = 12;

    std::cout << input_x.to_str() << '\n';
    std::cout << input_y.to_str() << '\n';
    std::cout << input_z.to_str() << '\n';

    SimpleTie result = tie::SimpleTieTogether({ input_x, input_y, input_z });
    std::cout << result.tuple().to_str() << '\n';

    Tuple extract1 = tie::cppUntieApart(result, 0);
    Tuple extract2 = tie::cppUntieApart(result, 1);
    Tuple extract3 = tie::cppUntieApart(result, 2);

    std::cout << extract1.to_str() << '\n';
    std::cout << extract2.to_str() << '\n';
    std::cout << extract3.to_str() << '\n';

    Tuple parameter;
    parameter.GetNumber_Index(0) = 1;
    parameter.GetNumber_Index(1) = 8;
    parameter.GetNumber_Index(2) = 8;
    parameter.GetNumber_Index(3) = 1;
    Tuple copy1 = tie::SimpleUntieApart(0).dispatchThis(result.tuple());
    Tuple copy2 = tie::SimpleUntieApart(1).dispatchThis(result.tuple());
    Tuple copy3 = tie::SimpleUntieApart(2).dispatchThis(result.tuple());
    std::cout << copy1.to_str() << '\n';
    std::cout << copy2.to_str() << '\n';
    std::cout << copy3.to_str() << '\n';

    return 0;
}
