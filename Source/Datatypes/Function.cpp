#include "Function.h"

size_t hash_value(c_star_star::functions::Function x) {
	return std::hash<c_star_star::functions::Function>()(x);
}

size_t hash_value(std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple> x) {
	return std::hash<c_star_star::functions::Function>()(x.first) ^ std::hash<c_star_star::tuples::Tuple>()(x.second);
}

namespace std {
	size_t
		hash<std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple>>
		::operator()(const std::pair<c_star_star::functions::Function, c_star_star::tuples::Tuple>& x) const
	{
		return std::hash<c_star_star::functions::Function>()(x.first) ^ std::hash<c_star_star::tuples::Tuple>()(x.second);
	}
}

namespace std {
	size_t hash<c_star_star::functions::Function>::operator()(const c_star_star::functions::Function& x) const
	{
		using namespace boost::multiprecision;
		using namespace std;
		return (size_t)abs((x.id) % (1LL << 31));
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

		IMany& Function::FunctionNodes() {
			return FunctionArguments.at(id);
		}

		Function::NodeType& Function::NodeClass() {
			return FunctionTypes.at(id);
		}

		std::string& Function::name() {
			return names.at(id);
		}

		const IMany& Function::FunctionNodes() const {
			return FunctionArguments.at(id);
		}

		const Function::NodeType& Function::NodeClass() const {
			return FunctionTypes.at(id);
		}

		const std::string& Function::name() const {
			return names.at(id);
		}

		Function::Function() {
			id = ++max_id;
			FunctionTypes.insert({ id, NodeType::Uninitialized });
			FunctionArguments.insert({ id, IMany() });
			names.insert({ id, "" });
		}

		//Returns R: R[i] = T[i]
		//Ignores variable and returns T
		void Function::SetConstant(Tuple T) {
			NodeClass() = Function::NodeType::Constant;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(T);
		}

		void Function::SetIdentity() {
			NodeClass() = Function::NodeType::PointwiseIdentity;
			FunctionNodes().clear();
		}

		void Function::SetAddFunction(Function lhs, Function rhs) {
			NodeClass() = Function::NodeType::PointwisePlus;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(lhs);
			FunctionNodes().InsertTypeEnd(rhs);
		}

		void Function::SetSubtractFunction(Function lhs, Function rhs) {
			NodeClass() = Function::NodeType::PointwiseMinus;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(lhs);
			FunctionNodes().InsertTypeEnd(rhs);
		}

		void Function::SetProductFunction(Function lhs, Function rhs) {
			NodeClass() = Function::NodeType::PointwiseProduct;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(lhs);
			FunctionNodes().InsertTypeEnd(rhs);
		}

		void Function::SetQuotientFunction(Function lhs, Function rhs) {
			NodeClass() = Function::NodeType::PointwiseQuotient;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(lhs);
			FunctionNodes().InsertTypeEnd(rhs);
		}

		void Function::SetRemainderFunction(Function lhs, Function rhs) {
			NodeClass() = Function::NodeType::PointwiseRemainder;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(lhs);
			FunctionNodes().InsertTypeEnd(rhs);
		}

		//Sets the recursive call - not the recursive definition
		void Function::SetBorrow(Function shrink, Tuple basecase, Function top) {
			NodeClass() = Function::NodeType::Borrow;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(top);
			FunctionNodes().InsertTypeEnd(shrink);
			FunctionNodes().InsertTypeEnd(basecase);
		}

		//Returns R[i] := check[i] < threshold[i] ? ifLess[i] : ifEqualOrMore[i]
		void Function::SetPointwiseLess(Function check, Function threshold, Function ifLess, Function ifEqualOrMore) {
			NodeClass() = Function::NodeType::PointwiseLess;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(check);
			FunctionNodes().InsertTypeEnd(threshold);
			FunctionNodes().InsertTypeEnd(ifLess);
			FunctionNodes().InsertTypeEnd(ifEqualOrMore);
		}

		//Returns R[-Infinity:threshold[0] - 1] = left[i], R[threshold, Infinity] = right[i]
		void Function::SetPiecewiseLess(Function threshold, Function leftExclusive, Function rightInclusive) {
			NodeClass() = Function::NodeType::PiecewiseLess;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(leftExclusive);
			FunctionNodes().InsertTypeEnd(rightInclusive);
			FunctionNodes().InsertTypeEnd(threshold);
		}

		//Compares from most-significant element to least significant element, like a dictionary
		//If a[i] != b[i], early exit
		//Returns a < b ? ifLess : ifNotLess
		void Function::SetLexicographicLess(Function a, Function b, Function ifLess, Function ifNotLess) {
			NodeClass() = Function::NodeType::LexicographicLess;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(a);
			FunctionNodes().InsertTypeEnd(b);
			FunctionNodes().InsertTypeEnd(ifLess);
			FunctionNodes().InsertTypeEnd(ifNotLess);
		}

		//R[a[0]] = b[0]
		void Function::SetSetIndex(Function a, Function b) {
			NodeClass() = Function::NodeType::SetIndex;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(a);
			FunctionNodes().InsertTypeEnd(b);
		}

		//R[0] = b[a[0]]
		void Function::SetGetIndex(Function a, Function b) {
			NodeClass() = Function::NodeType::GetIndex;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(a);
			FunctionNodes().InsertTypeEnd(b);
		}

		//R[0] = MaxHeight(a), R[1] = Minheight(a)
		void Function::SetMaxHeight(Function a) {
			NodeClass() = Function::NodeType::MaxHeight;
			FunctionNodes().clear();
			FunctionNodes().InsertTypeEnd(a);
		}

		//Call the function associated with this node
		//Retrieve the functions/operands from within the FunctionNodes()
		Tuple Function::dispatchThis(Tuple input_) {
			Tuple input = normalize(input_);
			CASES(NodeClass(),
				NodeType::Uninitialized,
				NodeType::Constant,
				NodeType::PointwiseIdentity,
				NodeType::PointwisePlus,
				NodeType::PointwiseMinus,
				NodeType::PointwiseProduct,
				NodeType::PointwiseQuotient,
				NodeType::PointwiseRemainder,
				NodeType::PointwiseLess,
				NodeType::PiecewiseLess,
				NodeType::GetIndex,
				NodeType::SetIndex,
				NodeType::Borrow,
				NodeType::LexicographicLess,
				NodeType::MaxHeight
			);
			Tuple output;
			if (cache.find({ *this, input }) != cache.end()) {
				return cache.at({ *this, input });
			}

			switch (NodeClass()) {
			case NodeType::Uninitialized: {
				PANIC("Execute an unitialized function!");
				UNREACHABLE("Function should be initialized!");
				break;
			}
			case NodeType::Constant: {
				ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
				Tuple target = FunctionNodes().pickLast().GetCppType<Tuple>("FunctionNode should store Tuple!");
				output = EvaluateConstant(target);
				break;
			}
			case NodeType::PointwiseIdentity: {
				ASSUME(FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
				output = EvaluatePointwiseIdentity(input);
				break;
			}
			case NodeType::PointwisePlus: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple lhs_t = lhs_f(input);
				Tuple rhs_t = rhs_f(input);
				output = EvaluatePointwisePlus(lhs_t, rhs_t);
				break;
			}
			case NodeType::PointwiseMinus: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple lhs_t = lhs_f(input);
				Tuple rhs_t = rhs_f(input);
				output = EvaluatePointwiseMinus(lhs_t, rhs_t);
				break;
			}
			case NodeType::PointwiseProduct: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple lhs_t = lhs_f(input);
				Tuple rhs_t = rhs_f(input);
				output = EvaluatePointwiseProduct(lhs_t, rhs_t);
				break;
			}
			case NodeType::PointwiseQuotient: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple    lhs_t = lhs_f(input);
				Tuple    rhs_t = rhs_f(input);
				output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
				break;
			}
			case NodeType::PointwiseRemainder: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple    lhs_t = lhs_f(input);
				Tuple    rhs_t = rhs_f(input);
				output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
				break;
			}
			case NodeType::PointwiseLess: {
				ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
				Function value = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function test = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function alt_1 = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				Function alt_2 = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
				Tuple    ahs_t = value(input);
				Tuple    bhs_t = test(input);
				Tuple    chs_t = alt_1(input);
				Tuple    dhs_t = alt_2(input);
				output = EvaluatePointwiseLess(ahs_t, bhs_t, chs_t, dhs_t);
				break;
			}
			case NodeType::PiecewiseLess: {
				ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
				Function left_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function right_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function threshold_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				Tuple    left_t = left_f(input);
				Tuple    right_t = right_f(input);
				Tuple    threshold = threshold_f(input);
				output = EvaluatePiecewiseLess(left_t, right_t, threshold.GetNumberConst_Index(0));
				break;
			}
			case NodeType::GetIndex: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple ahs_t = ahs_f(input);
				Tuple bhs_t = bhs_f(input);
				output = EvaluateGetIndex(ahs_t, bhs_t);
				break;
			}
			case NodeType::SetIndex: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Tuple ahs_t = ahs_f(input);
				Tuple bhs_t = bhs_f(input);
				output = EvaluateSetIndex(ahs_t, bhs_t);
				break;
			}
			case NodeType::Borrow: {
				ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode Borrow must store 3 elements!");
				Function top = FunctionNodes().nthElementVector(0).GetCppType<Function>("[0] element must be a Function Pointer! (Top)");
				Function shrink = FunctionNodes().nthElementVector(1).GetCppType<Function>("[1] element must be a Function! (shrink)");
				Tuple basecase = FunctionNodes().nthElementVector(2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
				output = EvaluateBorrow(input, shrink, basecase, top);
				break;
			}
			case NodeType::LexicographicLess: {
				ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
				Tuple ahs_t = ahs_f(input);
				Tuple bhs_t = bhs_f(input);
				Tuple chs_t = chs_f(input);
				Tuple dhs_t = dhs_f(input);
				output = EvaluateLexicographicLess(ahs_t, bhs_t, chs_t, dhs_t);
				break;
			}
			case NodeType::MaxHeight: {
				ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode MaxHeight must store 1 elements!");
				Function a = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Tuple a_t = a(input);
				output = EvaluateMaxHeight(a_t);
				break;
			}
			default:
				UNREACHABLE("All cases should be covered.");
				PANIC("Default Case!");
			}
			if (cache.find({ *this, input }) == cache.end()) {
				cache.insert({ { *this, input }, output });
			}
			return output;
		}

		//Returns a constant tuple
		Tuple Function::EvaluateConstant(Tuple target) {
			Tuple result = target;
			return result;
		};

		//Returns the input unchanged
		Tuple Function::EvaluatePointwiseIdentity(Tuple input) {
			return input;
		};

		//Returns the "pointwise" sum of both inputs
		Tuple Function::EvaluatePointwisePlus(Tuple input_a, Tuple input_b) {
			std::unordered_set<number_> storedIndices;
			Tuple result;
			for (const auto& index : input_a.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : input_b.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : storedIndices) {
				result.data[index] = input_a.GetNumberConst_Index(index) + input_b.GetNumberConst_Index(index);
			}
			return result;
		};

		//Returns the "pointwise" difference of both inputs
		Tuple Function::EvaluatePointwiseMinus(Tuple input_a, Tuple input_b) {
			std::unordered_set<number_> storedIndices;
			Tuple result;
			for (const auto& index : input_a.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : input_b.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : storedIndices) {
				result.data[index] = input_a.GetNumberConst_Index(index) - input_b.GetNumberConst_Index(index);
			}
			return result;
		};

		//Returns the "pointwise" product of both inputs
		Tuple Function::EvaluatePointwiseProduct(Tuple input_a, Tuple input_b) {
			std::unordered_set<number_> storedIndices;
			Tuple result;
			for (const auto& index : input_a.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : input_b.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : storedIndices) {
				result.data[index] = input_a.GetNumberConst_Index(index) * input_b.GetNumberConst_Index(index);
			}
			return result;
		};

		//Returns the "pointwise" quotient of both inputs
		// x/0 == 0
		Tuple Function::EvaluatePointwiseQuotient(Tuple input_a, Tuple input_b) {
			std::unordered_set<number_> storedIndices;
			Tuple result;
			for (const auto& index : input_a.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : input_b.data) {
				storedIndices.insert(index.first);
			}
			for (const auto& index : storedIndices) {
				number_ a = input_a.GetNumberConst_Index(index);
				number_ b = input_b.GetNumberConst_Index(index);
				number_ c = 0;
				if (b == 0)
					c = 0;
				else
					c = (a / b);
				result.data[index] = c;
			}
			return result;
		};

		//R[i] := a[i] < b[i] ? c[i] : d[i]
		Tuple Function::EvaluatePointwiseLess(Tuple input_a, Tuple input_b, Tuple input_c, Tuple input_d) {
			Tuple result;
			std::unordered_set<number_> shared_indices;
			for (const auto& i : input_a.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_b.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_c.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& index : shared_indices) {
				number_ a_val = input_a.GetNumberConst_Index(index);
				number_ b_val = input_b.GetNumberConst_Index(index);
				number_ c_val = input_c.GetNumberConst_Index(index);
				number_ d_val = input_d.GetNumberConst_Index(index);
				if (a_val < b_val) {
					result.data[index] = c_val;
				}
				else if (a_val >= b_val) {
					result.data[index] = d_val;
				}
			}
			return result;
		}

		//R[-Infinity:threshold - 1] = a[i], R[threshold, Infinity] = b[i]
		Tuple Function::EvaluatePiecewiseLess(Tuple input_a, Tuple input_b, number_ threshold) {
			Tuple result;
			std::unordered_set<number_> shared_indices;
			for (const auto& i : input_a.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_b.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& index : shared_indices) {
				number_ a_val = input_a.GetNumberConst_Index(index);
				number_ b_val = input_b.GetNumberConst_Index(index);
				if (index < threshold) {
					if (a_val != 0)
						result.data[index] = a_val;
				}
				else if (index >= threshold) {
					if (b_val != 0)
						result.data[index] = b_val;
				}
			}
			return result;
		}

		//R[i] := a[i] == threshold ? b[i] : c[i]
		Tuple Function::EvaluatePiecewiseEqual(Tuple input_a, Tuple input_b, number_ threshold) {
			Tuple result;
			std::unordered_set<number_> shared_indices;
			for (const auto& i : input_a.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_b.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& index : shared_indices) {
				number_ a_val = input_a.GetNumberConst_Index(index);
				number_ b_val = input_b.GetNumberConst_Index(index);
				if (index == threshold) {
					result.data[index] = a_val;
				}
				else if (index != threshold) {
					result.data[index] = b_val;
				}
			}
			return result;
		}

		//R[i] := a[i] == threshold ? b[i] : c[i]
		Tuple Function::EvaluatePointwiseEqual(Tuple input_a, Tuple input_b, Tuple input_c, number_ threshold) {
			Tuple result;
			std::unordered_set<number_> shared_indices;
			for (const auto& i : input_a.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_b.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& i : input_c.data) {
				shared_indices.insert(i.first);
			}
			for (const auto& index : shared_indices) {
				number_ a_val = input_a.GetNumberConst_Index(index);
				number_ b_val = input_b.GetNumberConst_Index(index);
				number_ c_val = input_c.GetNumberConst_Index(index);
				if (a_val == threshold) {
					result.data[index] = b_val;
				}
				else if (a_val != threshold) {
					result.data[index] = c_val;
				}
			}
			return result;
		}

		//R[a[0]] = b[0]
		Tuple Function::EvaluateSetIndex(Tuple input_a, Tuple input_b) {
			Tuple result;
			number_ index = input_a.GetNumberConst_Index(0);
			result.GetNumber_Index(index) = input_b.GetNumberConst_Index(0);
			return result;
		}

		//R[0] = b[a[0]]
		Tuple Function::EvaluateGetIndex(Tuple input_a, Tuple input_b) {
			Tuple result;
			number_ index = input_a.GetNumberConst_Index(0);
			result.GetNumber_Index(0) = input_b.GetNumberConst_Index(index);
			return result;
		}

		//Call a function
		Tuple Function::operator()(Tuple input) {
			return dispatchThis(input);
		}

		//Remove all zero elements
		Tuple Function::normalize(Tuple input) {
			std::erase_if(input.data, [](auto x)->bool { return x.second == 0; });
			if (input.data.size() == 0)
				input = Tuple();
			return input;
		}

		//Takes four parameters: Input, Recursive call argument generator, basecase, and full-function
		//Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
		//If shrunk operand is L<= 0 or has any negative element at positive index; return the basecase
		//Return the result of applying the "top" function to the shrunk value
		//If top returns the "true" function, can be used to create recursive calls
		//eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
		Tuple Function::EvaluateBorrow(Tuple Op1, Function shrinker, Tuple basecase, Function top) {
			static Tuple ZeroT = Tuple(0);
			if (+Tuple::isZero(Op1)) {
				return basecase;
			}
			Tuple shrunk1 = shrinker(Op1);
			Tuple shrunk1Left = EvaluatePiecewiseLess(shrunk1, ZeroT, number_(0));
			Tuple shrunk1Right = EvaluatePiecewiseLess(ZeroT, shrunk1, number_(0));
			//If the left half is not empty
			bool isLeftNotEmpty = not Tuple::isZero(shrunk1Left);
			//If the right half is empty
			bool isRightEmpty = +Tuple::isZero(shrunk1Right);
			//If Input << 0
			bool isNegative = +Tuple::isLess(shrunk1, ZeroT);
			//If not Input <<= Shrunk
			bool isNotSmaller = not Tuple::isLess(shrunk1, Op1);
			//If Input < 0 != 0
			bool hasAnyNegative = not Tuple::isZero(Function::EvaluatePointwiseLess(shrunk1, ZeroT, shrunk1, ZeroT));
			//If Input > 0 == 0
			bool hasAnyNegativeRight = not Tuple::isZero(Function::EvaluatePointwiseLess(shrunk1Right, ZeroT, shrunk1Right, ZeroT));

			if (isRightEmpty or isNegative or isNotSmaller or hasAnyNegativeRight) {
				return basecase;
			}
			return top(shrunk1);
		}

		//If lessOp1 < lessOp2, return ifLess else ifNotLess
		Tuple Function::EvaluateLexicographicLess(Tuple lessOp1, Tuple lessOp2, Tuple ifLess, Tuple ifNotLess) {
			IComparisonResult x = Tuple::isLess(lessOp1, lessOp2);
			ASSUME(x.isKnown(), "Tuple::isLess should be complete!");
			if (+x) {
				return ifLess;
			}
			else if (!x) {
				return ifNotLess;
			}
			else
				UNREACHABLE("Either less or not less.");
		}

		//Return (0: Minheight, 1: Maxheight) if not zero; else return 0
		Tuple Function::EvaluateMaxHeight(Tuple input)
		{
			if (+Tuple::isZero(input)) {
				return Tuple(0);
			}
			ASSUME(not Tuple::isZero(input), "Has been checked!");
			Tuple MaxHeight = Tuple::getMaxHeight(input);
			Tuple MinHeight = Tuple::getMinHeight(input);
			Tuple result;
			result.GetNumber_Index(1) = MaxHeight.GetNumberConst_Index(0);
			result.GetNumber_Index(0) = MinHeight.GetNumberConst_Index(0);
			return result;
		}

		std::string Function::print_Function(bool is_sub) {
			if (name() != "" and is_sub)
				return name();
			//        std::string id_ = to_string(id);
			std::string id_ = "";
			std::string return_val = "";
			Function::NodeType NodeTypeClass = NodeClass();
			switch (NodeTypeClass)
			{
			case NodeType::Uninitialized: {
				PANIC("Print uninitialized function!");
				UNREACHABLE("Panic");
				return_val += "";
				break;
			}
			case NodeType::Constant: {
				ASSUME(FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
				Tuple target = FunctionNodes().pickLast().GetCppType<Tuple>("FunctionNode should store Tuple!");
				if (+Tuple::isZero(target)) {
					return_val += "0";
				}
				else if (+Tuple::hasSingleLimb(target, 0)) {
					return_val += to_string(target.GetNumberConst_Index(0));
				}
				else {
					return_val += target.to_str();
				}
				break;
			}
			case NodeType::PointwiseIdentity: {
				ASSUME(FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
				return_val += "x";
				break;
			}
			case NodeType::PointwisePlus: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				return_val += std::string("(") + lhs_f.print_Function(true) + " + " + rhs_f.print_Function(true) + ")";
				break;
			}
			case NodeType::PointwiseMinus: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				return_val += std::string("(") + lhs_f.print_Function(true) + " - " + rhs_f.print_Function(true) + ")";
				break;
			}
			case NodeType::PointwiseProduct: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				return_val += std::string("(") + lhs_f.print_Function(true) + " * " + rhs_f.print_Function(true) + ")";
				break;
			}
			case NodeType::PointwiseQuotient: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				return_val += std::string("(") + lhs_f.print_Function(true) + " / " + rhs_f.print_Function(true) + ")";
				return return_val;
			}
			case NodeType::PointwiseRemainder: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
				Function lhs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function rhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				return_val += std::string("(") + lhs_f.print_Function(true) + " % " + rhs_f.print_Function(true) + ")";
				return return_val;
			}
			case NodeType::PointwiseLess: {
				ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
				return std::string("(") + ahs_f.print_Function(true) + " < " + bhs_f.print_Function(true) + " ? " + chs_f.print_Function(true) + " : " + dhs_f.print_Function(true) + ")" + id_;
			}
			case NodeType::PiecewiseLess: {
				ASSUME(FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
				Function left = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function right = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function threshold = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				return std::string("(") + left.print_Function(true) + " | " + threshold.print_Function(true) + " | " + right.print_Function(true) + ")" + id_;
			}
			case NodeType::GetIndex: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				if (ahs_f.NodeClass() == NodeType::Constant) {
					Tuple indexTuple = ahs_f.FunctionNodes().nthElementVector(0).GetCppType<Tuple>();
					return_val = bhs_f.print_Function(true) + "[" + to_string(indexTuple.GetNumberConst_Index(0)) + "]";
				}
				else {
					return_val += bhs_f.print_Function(true) + "[" + ahs_f.print_Function(true) + "]";
				}
				break;
			}
			case NodeType::SetIndex: {
				ASSUME(FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				if (ahs_f.NodeClass() == NodeType::Constant) {
					Tuple t = ahs_f.FunctionNodes().nthElementVector(0).GetCppType<Tuple>();
					number_ n = t.GetNumberConst_Index(0);
					if (n == 0) {
						return_val = bhs_f.print_Function(true);
					}
					else {
						return_val = std::string("[") + to_string(n) + ": " + bhs_f.print_Function(true) + "]";
					}
				}
				else {
					return_val = std::string("[") + ahs_f.print_Function(true) + "[0]: " + bhs_f.print_Function(true) + "]";
				}
				break;
			}
			case NodeType::Borrow: {
				//Change, replace with "independent improper functions"
				//PANIC("Disimplement!");
				Function top = FunctionNodes().nthElementVector(0).GetCppType<Function>("[0] element must be a Function! (Top)");
				Function shrink = FunctionNodes().nthElementVector(1).GetCppType<Function>("[1] element must be a Function! (shrink)");
				Tuple basecase = FunctionNodes().nthElementVector(2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
				std::string top_name = (top.name() == "" ? "top" + to_string(top.id) : top.name());
				return top_name + "(" + basecase.to_str() + ", " + shrink.print_Function(true) + ")" + id_;
			}
			case NodeType::LexicographicLess: {
				ASSUME(FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
				Function ahs_f = FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
				Function bhs_f = FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
				Function chs_f = FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
				Function dhs_f = FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
				return std::string("(") + ahs_f.print_Function(true) + " << " + bhs_f.print_Function(true) + " ? " + chs_f.print_Function(true) + " : " + dhs_f.print_Function(true) + ")" + id_;
			}
			default:
				UNREACHABLE("All cases should be covered.");
				PANIC("Default Case!");
			}
			return return_val + id_;
		}

		//Structural equality
		//identity == identity
		//constant == constant iff a.value == b.value
		//+-*/% [] < << | are equal if the values are recursively equal (as checked by the operator)
		//If any borrow occurs, the value is unknown (not structurally checked)
		IComparisonResult Function::StructuralEquality(const Function& a, const Function& b) {
			if (a.id == b.id) {
				return true;
			}
			if (a.NodeClass() == Function::NodeType::Borrow or b.NodeClass() == Function::NodeType::Borrow) {
				return IComparisonResult::ConstructUnknown();
			}
			if (a.NodeClass() != b.NodeClass()) {
				return false;
			}
			if (a.NodeClass() == Function::NodeType::Uninitialized) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				return true;
			}
			if (a.NodeClass() == Function::NodeType::PointwiseIdentity) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				return true;
			}
			if (a.NodeClass() == Function::NodeType::Constant) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				Tuple a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Tuple>();
				Tuple b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Tuple>();
				if (Tuple::isEqual(a0, b0)) {
					return true;
				}
				else {
					return false;
				}
			}
			if (std::set{ Function::NodeType::PointwisePlus, Function::NodeType::PointwiseMinus, Function::NodeType::PointwiseProduct, Function::NodeType::PointwiseQuotient, Function::NodeType::PointwiseRemainder, Function::NodeType::GetIndex, Function::NodeType::SetIndex }.contains(a.NodeClass())) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				if (a0 == a1 and b0 == b1) {
					return true;
				}
				else {
					return false;
				}
			}
			if (std::set{ Function::NodeType::PiecewiseLess }.contains(a.NodeClass())) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				Function a2 = (a.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
				Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				Function b2 = (b.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
				if (a0 == a1 and b0 == b1 and a2 == b2) {
					return true;
				}
				else {
					return false;
				}
			}
			if (std::set{ Function::NodeType::PointwiseLess, Function::NodeType::LexicographicLess }.contains(a.NodeClass())) {
				ASSUME(a.NodeClass() == b.NodeClass(), "Already checked!");
				Function a0 = (a.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function a1 = (a.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				Function a2 = (a.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
				Function a3 = (a.FunctionNodes().nthElementVector(3)).GetCppType<Function>();
				Function b0 = (b.FunctionNodes().nthElementVector(0)).GetCppType<Function>();
				Function b1 = (b.FunctionNodes().nthElementVector(1)).GetCppType<Function>();
				Function b2 = (b.FunctionNodes().nthElementVector(2)).GetCppType<Function>();
				Function b3 = (b.FunctionNodes().nthElementVector(3)).GetCppType<Function>();
				if (a0 == a1 and b0 == b1 and a2 == b2 and a3 == b3) {
					return true;
				}
				else {
					return false;
				}
			}
			UNREACHABLE("Checked all cases!");
		}

		//Nominal if has borrows, structural if not
		bool operator==(const Function& a, const Function& b) {
			IComparisonResult x = Function::StructuralEquality(a, b);
			if (x.isKnown()) {
				return (x.getBoolOrAssert("If-guard"));
			}
			return (a.id == b.id);
		}

		number_ Function::max_id;

		std::unordered_map<number_, Function::NodeType> Function::FunctionTypes;
		std::unordered_map<number_, IMany> Function::FunctionArguments;
		std::unordered_map<std::pair<Function, Tuple>, Tuple> Function::cache;
		std::unordered_map<number_, std::string> Function::names;
	}
}


