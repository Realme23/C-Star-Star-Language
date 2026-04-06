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

    /*
    //Classes and definitions
    SequenceExecution Fibonacci;        //New function factorial: reads from cin and prints factorial
    SequenceStream cin_, cout_, n, i, a, b, c;  //Intermediate variables

    Metaframe x;                        //A new interpreter context

    Theory numerical_;              //A new type representing Tuples with one variable
    Function ExceptZero;                // f[x] = x | 0 | 0 | 1 | x

    //Definition of the numerical type
    numerical_({                        //Set the restrictions on Tuples
        ExceptZero[numerical_] == Zero, //All non-zero-index variables are identically equal to zero
    })({                                //Set the equivalence classes, numerical_(a) == numerical_(b) iff Tuple(a) == Tuple(b) or a == b
        Identity,                       
    })({                                //Set the operations and closure: +*[]
        TuplesPlus,5
        TuplesProduct,
        TuplesSetConstant,
    });

    //Definition of the interpreter context
    x({
                                        //Inherit from global
    })({                                
        std::cin, std::cout             //Inherit cin and cout
    })({
        cin_ <= std::cin,               //cin and cout are based on console IO
        cout_ <= std::cout,
        n[numerical_],                  //n, i, a, b and c are numerical types
        i[numerical_],
        a[numerical_],
        b[numerical_],
        c[numerical_],
    })({                                //meta commands: all numerical variables must be pure
        (numerical_(var_)[pure_]),
    });

    //Definition of the Fibonacci function
    Fibonacci({})({                     //Factorial Definition and code
        n <= cin_,                      //It takes no inputs
        a <= 0,
        b <= 1,
        for_({i, n - 1})({              //for i = 0; i <= n-1; i++
            c <= a,
            a <= b,
            b <= a + c,
        }),
        cout_ <= b,                     //output factorial to console
    })({
        pure_(a, b, c, i, n),           //describe the optimization contract: a, b, c, i are pure variables
        stream_(cin_, cout_)            //cin and cout are impure variables
    });

    //The "type" of this function as per the optimization contract, is:
    //n <= cin_, cout_ <= b[var_ = ...(n)]
    //The cin_ and cout_ statements are kept intact because they are streams, their order matters everytime
    //b is a pure function of n and the value output on cout_ is equivalent to ...(x) of the value received on cin_

    //Example compiler description:
    Theory CBackend;    //Describe the CBackend as a theory
    CBackend({ ... });  //Include types, blocks, functions, branching flow, memory, etc

    Theory Compiler;    //Describe the compiler passes
    Compiler({});

    
    //Example passes: Replace all restrictedtuples with their types, replace for with while
    //On the lhs only for everything, add asserts that the variable belongs to the type
    //Demonstrate that Fibonacci__1 and Fibonacci have the same "type"
    Fibonacci({})({
        n[numerical_] <= cin_,
        a[numerical_] <= 0,
        b[numerical_] <= 1,
        i[numerical_] <= 0,
        while_({i != n})({
            assert_[i >= 0 and i + 1 <= n],     //Because it came from a strong for loop, i is always between 0 and n - 1 inclusive
            c[numerical_] <= a,
            a[numerical_] <= b,
            b[numerical_] <= a + c,
            i[numerical_] <= i + 1,
        }),
        cout_ <= b[numerical_],
    });

    //Convert to recursion
    SequenceFunction FibonacciLoop1;
    FibonacciLoop1({ i[numerical_], a[numerical_], b[numerical_], c[numerical_], n[numerical_] })({
        assert_[i >= 0 and i + 1 <= n],
        c[numerical_] <= a,
        a[numerical_] <= b,
        b[numerical_] <= a + c,
        i[numerical_] <= i + 1,
        if_({i != n})({
            return_ <= FibonacciLoop1(i + 1, a, b, c, n)
        })({
            return_ <= (i + 1, a, b, c, n)
        })
    });

    Fibonacci({})({
        n[numerical_] <= cin_,
        a[numerical_] <= 0,
        b[numerical_] <= 1,
        i[numerical_] <= 0,
        (i, a, b, c, n) <= FibonacciLoop1(i, a, b, c, n),
        cout_ <= b,
    });

    //Constant propagation:
    Fibonacci({})({
        n[numerical_] <= cin_,
        (i, a, b, c, n) <= FibonacciLoop1(0, 0, 1, Indeterminate([numerical_]), n),
        cout_ <= b,
    });

    //Theory propagation:
    FibonacciLoop1({ i[numerical_], a[numerical_], b[numerical_], c[numerical_], n[numerical_] })({
        assert_[i >= 0 and i + 1 <= n],
        if_({i != n})({
            return_ <= FibonacciLoop1(i + 1, b, a + b, a, n)
        })({
            return_ <= (i + 1, a, b, c, n)
        })
    });

    //Invariant code elimination:
    FibonacciLoop1({ i[numerical_], a[numerical_], b[numerical_], n[numerical_]})({
        assert_[i >= 0 and i + 1 <= n],
        if_({i != n})({
            return_ <= FibonacciLoop1(i + 1, b, a + b, n)
        })({
            return_ <= (i + 1, a, b, n)
        })
    });

    Fibonacci({})({
        n[numerical_] <= cin_,
        (_, _, b, _) <= FibonacciLoop1(0, 0, 1, n),
        cout_ <= b,
    });

    //Theory Strengthening:
    FibonacciLoop1({ i[numerical_], a[numerical_], b[numerical_], n[numerical_] })({
        assert_[i >= 0 and i + 1 <= n],
        if_({i != n})({
            assert_[i != n],
            return_ <= FibonacciLoop1(i + 1, b, a + b, n)
        })({
            assert_[i = n],
            return_ <= (i + 1, a, b, n)
        })
    });

    Fibonacci({})({
        n[numerical_] <= cin_,
        (_, _, b, _) <= FibonacciLoop1(0, 0, 1, n),
        cout_ <= b,
    });


    //Proof of validity
    //Show that Fibonacci as defined here is equivalent to the Fibonacci defined above, with all relaxations being part of the optimization contract
    Fibonacci({})({                     //Factorial Definition and code
        n << cin_,                      //It takes no inputs
        a << 0,
        b << 1,
        for_({i, n - 1})({              //for i = 0; i <= n-1; i++
            c << a,
            a << b,
            b << a + c,
        }),
        cout_ << b,                     //output factorial to console
    })({
        pure_(a, b, c, i, n),           //describe the optimization contract: a, b, c, i are pure variables, their existence can be optimized out
        stream_(cin_, cout_)            //cin and cout are impure variables, every operation and every value therein is protected
    });
    */
    return 0;
}