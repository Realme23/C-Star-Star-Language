#pragma once
#include "../Main.h"
#include "../Polymorphic/IMany.h"
#include "../Datatypes/Tuple.h"
#include "../Datatypes/Number.h"

namespace c_star_star {
	namespace functions {
		namespace simple_executor {
			class SimpleExecutor;
		}
		namespace builder {
			class FunctionBuilder;
		}
		namespace tie {
			class SimpleTie;
		}
	}
}

namespace c_star_star {
	namespace functions {
		namespace core {
			using c_star_star::polymorphic::IMany;
			using c_star_star::data_types::Tuple;
			using c_star_star::interpreter::IComparisonResult;
			using c_star_star::interpreter::IComparisonToBool;

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
					Dilate,				//Return R[d[0] *i + d[1]] = (i % d[3] == d[2] ? input[(i - d[2]) /d[3]] : 0)
				};

				static std::unordered_map<number_, NodeType> FunctionTypes;
				static std::unordered_map<number_, IMany> FunctionArguments;
				static std::unordered_map<number_, std::string> names;

			public:
				IMany& FunctionNodes();

				NodeType& NodeClass();

				std::string& name();

				const IMany& FunctionNodes() const;

				const NodeType& NodeClass() const;

				const std::string& name() const;

				Function();

				//Call the function associated with this node
				//Retrieve the functions/operands from within the FunctionNodes()
				Tuple dispatchThis(Tuple input_);

				//Tuple operator()(Tuple input);

				std::string print_Function(bool is_sub = false);

				//Structural equality
				//identity == identity
				//constant == constant iff a.value == b.value
				//+-*/% [] < << | are equal if the values are recursively equal (as checked by the operator)
				//If any borrow occurs, the value is unknown (not structurally checked)
				static IComparisonResult StructuralEquality(const Function& a, const Function& b);

				//Nominal if has borrows, structural if not
				friend bool operator==(const Function& a, const Function& b);

				friend class simple_executor::SimpleExecutor;
				friend class builder::FunctionBuilder;

				number_ id;
				static number_ max_id;
			};
		}
		namespace builder {

			using namespace core;
			using c_star_star::data_types::PolyTuple;
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
					Dilate,					//Return R[d[0] *i + d[1]] = (i % d[3] == d[2] ? input[(i - d[2]) /d[3]] : 0)
					MinMaxHeight,
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

				FunctionBuilder(PolyTuple i);

				void setName(std::string s);

				static FunctionBuilder Variable();

				static FunctionBuilder BorrowFrom();

				//Check that all "upward links" are from borrow nodes
				bool CheckCycles() const;

				bool CheckCycles(std::unordered_set<FunctionBuilder>& seen) const;

				FunctionBuilder operator+(const FunctionBuilder rhs) const;

				FunctionBuilder operator-(const FunctionBuilder rhs) const;

				FunctionBuilder operator*(const FunctionBuilder rhs) const;

				FunctionBuilder operator/(const FunctionBuilder rhs) const;

				FunctionBuilder operator%(const FunctionBuilder rhs) const;

				FunctionBuilder operator[](const FunctionBuilder index) const;

				FunctionBuilder operator-() const;

				//Build Expression FunctionBorrowedNode
				FunctionBuilder operator()(const FunctionBuilder shrink, const FunctionBuilder control) const;

				//Apply the function to the output of F rather than input
				FunctionBuilder operator()(const FunctionBuilder F) const;

				friend IComparisonToBool operator==(const FunctionBuilder& a, const FunctionBuilder& b);

				friend class Dilate;
				
				//Build a dilate expression
				FunctionBuilder(Dilate d);

				friend class MinMaxHeight;

				//Build a dilate expression
				FunctionBuilder(MinMaxHeight d);

				//Create a set of seen functionbuilders from this one
				void build_seen_set(std::unordered_set<FunctionBuilder>& seen) const;

				//Set a borrowed-from node only!
				//Set it to another function
				void operator^=(const FunctionBuilder& F);

				//Returns a function that can evaluate the constructed builder
				Function buildFunction() const;

				std::string print_dbg();

				//Print the node type
				static std::string printNodeType(FunctionBuilderNodeTypes type);

				//Returns R: R[i] = T[i]
				//Ignores variable and returns T
				static void SetConstant(Function& F, Tuple T);

				static void SetIdentity(Function& F);

				static void SetAddFunction(Function& F, Function lhs, Function rhs);

				static void SetSubtractFunction(Function& F, Function lhs, Function rhs);

				static void SetProductFunction(Function& F, Function lhs, Function rhs);

				static void SetQuotientFunction(Function& F, Function lhs, Function rhs);

				static void SetRemainderFunction(Function& F, Function lhs, Function rhs);

				//Sets the recursive call - not the recursive definition
				static void SetBorrow(Function& F, Function shrink, Function top, Function control);

				//Returns R[i] := check[i] < threshold[i] ? ifLess[i] : ifEqualOrMore[i]
				static void SetPointwiseLess(Function& F, Function check, Function threshold, Function ifLess, Function ifEqualOrMore);

				//Returns R[-Infinity:threshold[0] - 1] = left[i], R[threshold, Infinity] = right[i]
				static void SetPiecewiseLess(Function& F, Function threshold, Function leftExclusive, Function rightInclusive);

				//Compares from most-significant element to least significant element, like a dictionary
				//If a[i] != b[i], early exit
				//Returns a < b ? ifLess : ifNotLess
				static void SetLexicographicLess(Function& F, Function a, Function b, Function ifLess, Function ifNotLess);

				//R[a[0]] = b[0]
				static void SetSetIndex(Function& F, Function a, Function b);

				//R[0] = b[a[0]]
				static void SetGetIndex(Function& F, Function a, Function b);

				//R[0] = MaxHeight(a), R[1] = MinHeight(a), if a == 0 R == 0
				static void SetMaxHeight(Function& F, Function a);

				//R[p0 *i + p1] = (i % p3 == p2 ? input[i - p2] /p3 : 0)
				static void SetDilate(Function& F, Function input, Function parameter1, Function parameter2, Function parameter3, Function parameter4);

			private:
				FunctionBuilder ReplaceIdentity(const FunctionBuilder& F) const;

				FunctionBuilder ReplaceIdentity(const FunctionBuilder& F, std::unordered_set<FunctionBuilder>& seen) const;

				static FunctionBuilder DeepCopy(const FunctionBuilder& F);

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

			class MinMaxHeight {
				friend class FunctionBuilder;
				FunctionBuilder input;
			public:
				MinMaxHeight(FunctionBuilder input): input(input) {}
			};

			class Dilate {
				friend class FunctionBuilder;
				FunctionBuilder input, parameter1, parameter2, parameter3, parameter4;
			public:
				Dilate(FunctionBuilder input, FunctionBuilder parameter1, FunctionBuilder parameter2, FunctionBuilder parameter3, FunctionBuilder parameter4) : input(input), parameter1(parameter1), parameter2(parameter2), parameter3(parameter3), parameter4(parameter4) {}
			};
		}

		namespace simple_executor {

			using namespace core;

			//Lexicographically compare two Tuples
			//Returns true for a < b; false for a > b, weak for a == b
			template<bool weak>
			IComparisonResult IsLexLess(const Tuple& a, const Tuple& b) {
				using c_star_star::data_types::Tuple;

				if (+Tuple::isEqual(a, b))
					return weak;

				Tuple startTa = Tuple::getMaxHeight(a);
				Tuple startTb = Tuple::getMaxHeight(b);
				Tuple endTa = Tuple::getMinHeight(a);
				Tuple endTb = Tuple::getMinHeight(b);

				//If a is zero, check the lead digit of b
				if (+Tuple::isZero(a)) {
					number_ base = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
					if (base > 0)
						return true;
					else if (base < 0)
						return false;
					else if (base == 0)
						UNREACHABLE("Lead digit should not be zero!");
				}

				//If b is zero, check the lead digit of a
				if (+Tuple::isZero(b)) {
					number_ base = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
					if (base > 0)
						return false;
					else if (base < 0)
						return true;
					else if (base == 0)
						UNREACHABLE("Lead digit should not be zero!");
				}
				ASSERT((not Tuple::isZero(a)) and (not Tuple::isZero(b)), "a and b should not be zero by this point!");

				//Get the number_ with the bigger "height"
				if (startTa.GetNumberConst_Index(0) > startTb.GetNumberConst_Index(0))
					return false;
				else if (startTa.GetNumberConst_Index(0) < startTb.GetNumberConst_Index(0))
					return true;

				//If they have the same height, compare the lead digit
				number_ a_lead = a.GetNumberConst_Index(startTa.GetNumberConst_Index(0));
				number_ b_lead = b.GetNumberConst_Index(startTb.GetNumberConst_Index(0));
				number_ a_tail = a.GetNumberConst_Index(endTa.GetNumberConst_Index(0));
				number_ b_tail = a.GetNumberConst_Index(endTb.GetNumberConst_Index(0));

				if (a_lead > b_lead) {
					return false;
				}
				else if (b_lead > a_lead) {
					return true;
				}
				else if (a_lead == b_lead) {
					//Same height, same lead digits
					//Iteratively compare smaller digits
					number_ index = startTa.GetNumberConst_Index(0);
					//Compare upto the lowest tail
					number_ end_index = std::min(endTa.GetNumberConst_Index(0), endTb.GetNumberConst_Index(0));
					while (index-- >= end_index) {
						number_ a_indexed = a.GetNumberConst_Index(index);
						number_ b_indexed = b.GetNumberConst_Index(index);
						if (a_indexed > b_indexed) {
							return false;
						}
						else if (a_indexed < b_indexed) {
							return true;
						}
						else if (a_indexed == b_indexed) {
							continue;
						}
					}
					UNREACHABLE("Equality should be checked early!");
				}
				UNREACHABLE("No more cases left!");
			}

			//Returns a constant tuple
			Tuple EvaluateConstant(Tuple target);

			//Returns the input unchanged
			Tuple EvaluatePointwiseIdentity(Tuple input);

			//Returns the "pointwise" sum of both inputs
			Tuple EvaluatePointwisePlus(Tuple input_a, Tuple input_b);

			//Returns the "pointwise" difference of both inputs
			Tuple EvaluatePointwiseMinus(Tuple input_a, Tuple input_b);

			//Returns the "pointwise" product of both inputs
			Tuple EvaluatePointwiseProduct(Tuple input_a, Tuple input_b);

			//Returns the "pointwise" quotient of both inputs
			// x/0 == 0, 0/0 == 0
			// quotient always satisfies "Euclidean division" (remainder between 0 and b)
			// q = sign(b)*(a / floor(b))
			Tuple EvaluatePointwiseQuotient(Tuple input_a, Tuple input_b);

			// r = a - b*(a/b)
			// x % 0 == x, 0 % 0 == 0
			// 0 <= r <= b
			Tuple EvaluatePointwiseRemainder(Tuple input_a, Tuple input_b);

			//R[i] := a[i] < b[i] ? c[i] : d[i]
			Tuple EvaluatePointwiseLess(Tuple input_a, Tuple input_b, Tuple input_c, Tuple input_d);

			//R[-Infinity:threshold - 1] = a[i], R[threshold, Infinity] = b[i]
			Tuple EvaluatePiecewiseLess(Tuple input_a, Tuple input_b, number_ threshold);

			//R[a[0]] = b[0]
			Tuple EvaluateSetIndex(Tuple input_a, Tuple input_b);

			//R[0] = b[a[0]]
			Tuple EvaluateGetIndex(Tuple input_a, Tuple input_b);

			//Takes four parameters: Input, Recursive call argument generator, and full-function and control
			//Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
			//Control controls the recursion, the recursion terminates if at any point control(operand) looks like it won't terminate
			// The test for recursion and validity is control(operand) and control(shrunk(operand))
			//Misbehaving is: when it isn't smaller, when the right half is empty, when it has an element in the right half < 0
			//Return the result of applying the "top" function to the shrunk value (not the control(operand))
			//If top returns the "true" function, can be used to create recursive calls
			//eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
			Tuple EvaluateBorrow(Tuple Op1, Function shrinker, Function top, Function control);

			//If lessOp1 L< lessOp2, return ifLess else ifNotLess
			Tuple EvaluateLexicographicLess(Tuple lessOp1, Tuple lessOp2, Tuple ifLess, Tuple ifNotLess);

			//Return the min- and max- height as a Tuple
			//min-height at 0 and max-height at 1
			Tuple EvaluateMaxHeight(Tuple input);

			//Return R[d[0] *i + d[1]] = (i % d[3] == d[2] ? input[i] : 0)
			Tuple EvaluateDilate(Tuple input, Tuple parameter1, Tuple parameter2, Tuple parameter3, Tuple parameter4);

			class SimpleExecutor {
			public:
				static Tuple ExecuteFunction(Function F, Tuple input);
				static Tuple ExecuteFunctionCached(Function F, Tuple input);
				static std::unordered_map<std::pair<Function, Tuple>, Tuple> cache;
			};
		}

		//A library of FunctionBuilders
		namespace library {
			using namespace builder;

			//If Test(x) == case_, return Value(x), else return Main(x);
			FunctionBuilder CreateCase(Tuple case_, FunctionBuilder Value, FunctionBuilder Main, FunctionBuilder Test);

			//base[index] = value, base[!index] = base
			FunctionBuilder CreatePiecewiseAt(FunctionBuilder base, FunctionBuilder index, FunctionBuilder value);

			//Return a function that performs pointwise x < 0 ? -x : x
			FunctionBuilder PointwiseAbs(FunctionBuilder base);

			//Return R[shift, shift + offset] = input[start, start + offset]
			FunctionBuilder ShiftPiecewise(FunctionBuilder input, FunctionBuilder shift, FunctionBuilder start, FunctionBuilder offset);
		}

		//Tie together a list of Tuples into a single Tuple, so that a function may take multiple Tuples
		//A list of Tuples can be tied into a single Tuple, which can be passed into functions
		//The tied-together tuples can be extracted with Functions and then operated on
		//	The default-case for invalid Tuples must be all zeroes
		//This is separate from Functions as a concept, and is allowed to use a different representation for each Tie
		namespace tie {
			using namespace core;

			//A Tie object stores implementation-specific information about that tie, including the resulting Tuple itself
			// The inputs must be fully constructible from the Tie's "operator Tuple()" alone
			class SimpleTie {
				Tuple T;
			public:
				SimpleTie(Tuple T): T(T) {}
				Tuple tuple() const {
					return T;
				}
			};

			//This ties together Tuples by storing the number of input Tuples at zero,
			// the cumulative sum of min-heights (for their start) and the difference (max-height - min-height),
			// and then concatenating their data from min-height to max-height
			SimpleTie SimpleTieTogether(const std::initializer_list<Tuple>& list_T);

			//Extracting the Tuple is as easy as looking up the index within the number of tuples, if its stored then looking up the min-heights and the size, then jumping to the corresponding relevant indexes and extracting that Tuple
			Function SimpleUntieApart(const SimpleTie& T, number_ index);
		}
	}
}

namespace std {
	template <> struct hash<c_star_star::functions::core::Function>
	{
		size_t operator()(const c_star_star::functions::core::Function& x) const;
	};
}
namespace std {
	template <> struct hash<c_star_star::functions::builder::FunctionBuilder>
	{
		size_t operator()(const c_star_star::functions::builder::FunctionBuilder& x) const;
	};
}
namespace std {
	template<> struct
		hash<std::pair<c_star_star::functions::core::Function, c_star_star::data_types::Tuple>> {
		size_t operator()(const std::pair<c_star_star::functions::core::Function, c_star_star::data_types::Tuple>& x) const;
	};
}