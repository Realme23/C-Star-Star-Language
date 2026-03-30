#pragma once
#include "../Polymorphic/IMany.h"
#include "Tuple.h"

namespace c_star_star {
    namespace functions {
        using c_star_star::polymorphic::IMany;
        using c_star_star::tuples::Tuple;
        using c_star_star::tribool::IComparisonResult;
        using c_star_star::tribool::IComparisonToBool;

        //Describe pure Tuple->Tuple operations
        //Only supports total, pure functions that return in finite time
        //All functions are fully specified (x /0 == 0 and x %0 == x)
        ///Operations: Lexicographically bounded recursion
        ///Identity function, pointwise add/subtract/multiply/divide/mod etc, index get/set
        ///Fully Immutable once built
        ///Atoms := (Finite) Constant values, Pointwise Identity function
        ///Describe function type-selection and dispatch explicitly, along with allocation etc
        class Function {

            //Nodes:
            //Constant values
            //Pointwise Identity, pointwise +-*/, get/set single
            //Total piecewise based on < or > or == (ie if-else)
            //Borrow: Bounded recursion calls
            //MaxHeight: Return a Tuple with the max height of the Tuple
            enum class NodeType {
                Uninitialized,      //Panic if called
                Constant,           //Return a constant value
                PointwiseIdentity,  //Return the input unchanged
                PointwisePlus,      //Return R[i] := a[i] + b[i]
                PointwiseMinus,     //Return R[i] := a[i] - b[i]
                PointwiseProduct,   //Return R[i] := a[i] * b[i]
                PointwiseQuotient,  //Return R[i] := a[i] / b[i]    (x /0 = 0)
                PointwiseRemainder, //Return R[i] := a[i] % b[i]    (x %0 = x)
                PointwiseLess,      //Return R[i] := a[i] < b[i] ? c[i] : d[i]
                PiecewiseLess,      //Return R[i] := i < c[0] ? a[i] : b[i]
                LexicographicLess,  //Return R    := a L< b ? c : d
                GetIndex,           //Return R[0] = b[a[0]]
                SetIndex,           //Return R[a[0]] = b[0]
                Borrow,             //Return the value of calling a "higher" function, with an input strictly between current input and zero
                MaxHeight,          //Return a Tuple representing the max and min-height of the input
            };

            static std::unordered_map<number_, NodeType> FunctionTypes;
            static std::unordered_map<number_, IMany> FunctionArguments;
            static std::unordered_map<std::pair<Function, Tuple>, Tuple> cache;
            static std::unordered_map<number_, std::string> names;

        public:
            IMany& FunctionNodes();

            NodeType& NodeClass();

            std::string& name();

            const IMany& FunctionNodes() const;

            const NodeType& NodeClass() const;

            const std::string& name() const;

            Function();

            //Returns R: R[i] = T[i]
            //Ignores variable and returns T
            void SetConstant(Tuple T);

            void SetIdentity();

            void SetAddFunction(Function lhs, Function rhs);

            void SetSubtractFunction(Function lhs, Function rhs);

            void SetProductFunction(Function lhs, Function rhs);

            void SetQuotientFunction(Function lhs, Function rhs);

            void SetRemainderFunction(Function lhs, Function rhs);

            //Sets the recursive call - not the recursive definition
            void SetBorrow(Function shrink, Tuple basecase, Function top);

            //Returns R[i] := check[i] < threshold[i] ? ifLess[i] : ifEqualOrMore[i]
            void SetPointwiseLess(Function check, Function threshold, Function ifLess, Function ifEqualOrMore);

            //Returns R[-Infinity:threshold[0] - 1] = left[i], R[threshold, Infinity] = right[i]
            void SetPiecewiseLess(Function threshold, Function leftExclusive, Function rightInclusive);

            //Compares from most-significant element to least significant element, like a dictionary
            //If a[i] != b[i], early exit
            //Returns a < b ? ifLess : ifNotLess
            void SetLexicographicLess(Function a, Function b, Function ifLess, Function ifNotLess);

            //R[a[0]] = b[0]
            void SetSetIndex(Function a, Function b);

            //R[0] = b[a[0]]
            void SetGetIndex(Function a, Function b);

            //R[0] = MaxHeight(a), R[1] = MinHeight(a), if a == 0 R == 0
            void SetMaxHeight(Function a);

            //Call the function associated with this node
            //Retrieve the functions/operands from within the FunctionNodes()
            Tuple dispatchThis(Tuple input_);

            //Returns a constant tuple
            static Tuple EvaluateConstant(Tuple target);

            //Returns the input unchanged
            static Tuple EvaluatePointwiseIdentity(Tuple input);

            //Returns the "pointwise" sum of both inputs
            static Tuple EvaluatePointwisePlus(Tuple input_a, Tuple input_b);

            //Returns the "pointwise" difference of both inputs
            static Tuple EvaluatePointwiseMinus(Tuple input_a, Tuple input_b);

            //Returns the "pointwise" product of both inputs
            static Tuple EvaluatePointwiseProduct(Tuple input_a, Tuple input_b);

            //Returns the "pointwise" quotient of both inputs
            // x/0 == 0
            static Tuple EvaluatePointwiseQuotient(Tuple input_a, Tuple input_b);

            //R[i] := a[i] < b[i] ? c[i] : d[i]
            static Tuple EvaluatePointwiseLess(Tuple input_a, Tuple input_b, Tuple input_c, Tuple input_d);

            //R[-Infinity:threshold - 1] = a[i], R[threshold, Infinity] = b[i]
            static Tuple EvaluatePiecewiseLess(Tuple input_a, Tuple input_b, number_ threshold);

            //R[i] := a[i] == threshold ? b[i] : c[i]
            static Tuple EvaluatePiecewiseEqual(Tuple input_a, Tuple input_b, number_ threshold);

            //R[i] := a[i] == threshold ? b[i] : c[i]
            static Tuple EvaluatePointwiseEqual(Tuple input_a, Tuple input_b, Tuple input_c, number_ threshold);

            //R[a[0]] = b[0]
            static Tuple EvaluateSetIndex(Tuple input_a, Tuple input_b);

            //R[0] = b[a[0]]
            static Tuple EvaluateGetIndex(Tuple input_a, Tuple input_b);

            Tuple operator()(Tuple input);

            static Tuple normalize(Tuple input);

            //Takes four parameters: Input, Recursive call argument generator, basecase, and full-function
            //Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
            //If shrunk operand is L<= 0 or has any negative element at positive index; return the basecase
            //Return the result of applying the "top" function to the shrunk value
            //If top returns the "true" function, can be used to create recursive calls
            //eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
            static Tuple EvaluateBorrow(Tuple Op1, Function shrinker, Tuple basecase, Function top);

            //If lessOp1 < lessOp2, return ifLess else ifNotLess
            static Tuple EvaluateLexicographicLess(Tuple lessOp1, Tuple lessOp2, Tuple ifLess, Tuple ifNotLess);

            //Return the min- and max- height as a Tuple
            static Tuple EvaluateMaxHeight(Tuple input);


            std::string print_Function(bool is_sub = false);

            //Structural equality
            //identity == identity
            //constant == constant iff a.value == b.value
            //+-*/% [] < << | are equal if the values are recursively equal (as checked by the operator)
            //If any borrow occurs, the value is unknown (not structurally checked)
            static IComparisonResult StructuralEquality(const Function& a, const Function& b);

            //Nominal if has borrows, structural if not
            friend bool operator==(const Function& a, const Function& b);

            number_ id;
            static number_ max_id;
        };
    }
    namespace function_builder {

        using namespace functions;
        using c_star_star::tuples::PolyTuple;
        using c_star_star::polymorphic::IAny;

        //Builds a Function from C++ code, syntactically
        //Join terminals with overloaded operators
        class FunctionBuilder {
        public:

            enum class FunctionBuilderNodeTypes {
                Uninitialized,          //Error on usage
                ConstantTerminal,       //Represent a Constant Tuple
                VariableTerminal,       //Represent a Variable
                FunctionPlus,           //Build expression a + b
                FunctionMinus,          //Build expression a - b
                FunctionProduct,        //Build expression a * b
                FunctionQuotient,       //Build expression a / b
                FunctionMod,            //Build expression a % b
                FunctionPointwiseLess,     //Build expression a[i] < b[i] ? c[i] : d[i]
                FunctionPiecewiseLess,     //Build expression i < a[0] ? b[i] : c[i]
                FunctionLexicographicLess, //Build expression a L< b ? c : d
                FunctionGetIndex,          //Build Result[0] = a[b[0]]
                FunctionSetIndex,       //Build Result[a[0]] = b[0]
                FunctionBorrowedNode,   //Build recursive call
                UnsetBorrowFromNode,    //The node that is borrowed from
                SetBorrowFrom,          //A "borrow from" node that is set to another function
                CallFunctionNode,       //Calls a prebuilt function
            };

            number_ builder_id = 0;
            static number_ max_builder_id;
            static std::unordered_map<number_, IMany> data_static;
            static std::unordered_map<number_, FunctionBuilderNodeTypes> node_types;
            static std::unordered_map<number_, std::string> names;

            FunctionBuilderNodeTypes& FunctionBuilderNode();

            const FunctionBuilderNodeTypes& FunctionBuilderNode() const;

            IMany& data();

            const IMany& data() const;

            std::string& name();

            const std::string& name() const;

            static bool isNotUninitialized(const FunctionBuilder S);

            FunctionBuilder();

            FunctionBuilder(Tuple T);

            FunctionBuilder(number_ n);

            FunctionBuilder(std::integral auto n);

            FunctionBuilder(Function F);

            void setName(std::string s);

            static FunctionBuilder Variable();

            static FunctionBuilder BorrowFrom();

            FunctionBuilder operator+(const FunctionBuilder rhs) const;

            FunctionBuilder operator-(const FunctionBuilder rhs) const;

            FunctionBuilder operator*(const FunctionBuilder rhs) const;

            FunctionBuilder operator/(const FunctionBuilder rhs) const;

            FunctionBuilder operator%(const FunctionBuilder rhs) const;

            FunctionBuilder operator[](const FunctionBuilder index) const;

            FunctionBuilder operator-() const;

            FunctionBuilder(PolyTuple i);

            //Build Expression FunctionBorrowedNode
            FunctionBuilder operator()(const FunctionBuilder shrink, const PolyTuple basecase) const;

            //Apply the function to the output of F rather than input
            FunctionBuilder operator()(const FunctionBuilder F) const;

            friend IComparisonToBool operator==(const FunctionBuilder& a, const FunctionBuilder& b);

            void operator^=(const FunctionBuilder& F);

            //Returns a function that can evaluate the constructed builder
            Function buildFunction() const;

            std::string print_dbg();

        private:
            FunctionBuilder ReplaceIdentity(const FunctionBuilder& F) const;

            //Sets a borrowed-from node to its value
            void setBorrowed(const FunctionBuilder& F);

            struct SetTypeOP;
            friend class LexicographicCase2;
            friend class PointwiseCase2;
            friend class PiecewiseCase1;
            friend struct SetType;
            friend FunctionBuilder operator||(LexicographicCase2 PC, FunctionBuilder alt_2);
            friend FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2);
            friend FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_2);

            //Build expression a[i] < b[i] ? c[i] : d[i]
            static FunctionBuilder BuildPointwiseLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt1, const FunctionBuilder alt2);

            //Build expression i < index ? alt_1[i] : alt_2[i]
            static FunctionBuilder BuildPiecewiseLess(const FunctionBuilder alt_1, const FunctionBuilder index, const FunctionBuilder alt_2);

            //Build expression a < b ? c : d
            static FunctionBuilder BuildLexicographicLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt_1, const FunctionBuilder alt_2);

            //Build Expression result[0] = lhs[index[0]]
            FunctionBuilder GetIndex(const FunctionBuilder index) const;

            //Build Expression result[index[0]] = lhs[0]
            FunctionBuilder SetIndex(const FunctionBuilder index) const;

        public:
            FunctionBuilder operator[](SetTypeOP S) const;
        };

        //Describe a constant as a FunctionBuilder node
        FunctionBuilder const_(PolyTuple i);

        //Helper classes for constructing functions syntactically

        //a < b || alt_1 || alt_2
        class PointwiseCase1;
        class PointwiseCase2;

        //alt_1 | index | alt_2
        class PiecewiseCase1;

        //a << b || alt_1 || alt_2
        class LexicographicCase1;
        class LexicographicCase2;


        class PointwiseCase1 {
        public:
            FunctionBuilder test;
            FunctionBuilder value;

            PointwiseCase1(FunctionBuilder value, FunctionBuilder test);
            friend PointwiseCase2 operator||(PointwiseCase1 PC, FunctionBuilder alt_1);
        };

        class PointwiseCase2 {
            FunctionBuilder test;
            FunctionBuilder value;
            FunctionBuilder alt_1;
        public:
            PointwiseCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1);
            friend FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2);
        };

        class PiecewiseCase1 {
        public:
            FunctionBuilder index;
            FunctionBuilder alt_1;

            PiecewiseCase1(FunctionBuilder value, FunctionBuilder test);
            friend FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_1);
        };

        class LexicographicCase1 {
        public:
            FunctionBuilder test;
            FunctionBuilder value;

            LexicographicCase1(FunctionBuilder value, FunctionBuilder test);
            friend LexicographicCase2 operator||(LexicographicCase1 PC, FunctionBuilder alt_1);
        };

        class LexicographicCase2 {
            FunctionBuilder test;
            FunctionBuilder value;
            FunctionBuilder alt_1;
        public:
            LexicographicCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1);
            friend FunctionBuilder operator||(LexicographicCase2 PC, FunctionBuilder alt_2);
        };

        struct FunctionBuilder::SetTypeOP {
            FunctionBuilder F;
            SetTypeOP(FunctionBuilder F) : F(F) {}
        };

        struct SetType {
            FunctionBuilder::SetTypeOP operator>>(const FunctionBuilder F) const;
        };

        //If Test(x) == case_, return Value(x), else return Main(x);
        FunctionBuilder CreateCase(Tuple case_, FunctionBuilder Value, FunctionBuilder Main, FunctionBuilder Test);

        //base[index] = value, base[!index] = base
        FunctionBuilder CreatePiecewiseAt(FunctionBuilder base, FunctionBuilder index, FunctionBuilder value);

        //Return a function that performs pointwise x < 0 ? -x : x
        FunctionBuilder PointwiseAbs(FunctionBuilder base);
	}
}

namespace std {
	template <> struct hash<c_star_star::functions::Function>
	{
		size_t operator()(const c_star_star::functions::Function& x) const;
	};
}

namespace std {
	template<> struct
		hash<std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple>> {
		size_t operator()(const std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple>& x) const;
	};
}

size_t hash_value(std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple> x);