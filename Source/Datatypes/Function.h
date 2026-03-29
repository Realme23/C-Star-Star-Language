#pragma once
#include "../Polymorphic/IMany.h"
#include "Tuple.h"

namespace c_star_star {
	namespace tuples {
		class Tuple;
	}
}

namespace c_star_star {
	namespace functions {
		using c_star_star::polymorphic::IMany;
		using c_star_star::tuples::Tuple;
		using c_star_star::tribool::IComparisonResult;

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