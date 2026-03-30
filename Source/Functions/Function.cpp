#include "Function.h"

namespace std {
	size_t
		hash<std::pair<c_star_star::functions::core::Function, c_star_star::data_types::Tuple>>
		::operator()(const std::pair<c_star_star::functions::core::Function, c_star_star::data_types::Tuple>& x) const
	{
		return std::hash<c_star_star::functions::core::Function>()(x.first) ^ std::hash<c_star_star::data_types::Tuple>()(x.second);
	}
}
namespace std {
	size_t hash<c_star_star::functions::core::Function>::operator()(const c_star_star::functions::core::Function& x) const
	{
		using namespace boost::multiprecision;
		using namespace std;
		return (size_t)abs((x.id) % (1LL << 31));
	}
}

namespace c_star_star {
	namespace functions {
		namespace core {
			using c_star_star::polymorphic::IMany;
			using c_star_star::data_types::Tuple;
			using c_star_star::interpreter::IComparisonResult;

			//Describe pure Tuple->Tuple operations
			//Only supports total, pure functions that return in finite time
			//All functions are fully specified (x /0 == 0 and x %0 == x)
			///Operations: Lexicographically bounded recursion
			///Identity function, pointwise add/subtract/multiply/divide/mod, index get/set
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

			//Call the function associated with this node
			//Retrieve the functions/operands from within the FunctionNodes()
			Tuple Function::dispatchThis(Tuple input) {
				Tuple output;
				output = simple_executor::SimpleExecutor::ExecuteFunctionCached(*this, input);
				return output;
			}

			//Call a function
			Tuple Function::operator()(Tuple input) {
				return dispatchThis(input);
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
			std::unordered_map<number_, std::string> Function::names;
		}

		namespace builder {

			FunctionBuilder::FunctionBuilderNodeTypes& FunctionBuilder::FunctionBuilderNode() {
				if (not node_types.contains(builder_id)) {
					node_types[builder_id] = FunctionBuilderNodeTypes::Uninitialized;
				}

				return node_types[builder_id];
			}

			const FunctionBuilder::FunctionBuilderNodeTypes& FunctionBuilder::FunctionBuilderNode() const {
				if (not node_types.contains(builder_id)) {
					node_types[builder_id] = FunctionBuilderNodeTypes::Uninitialized;
				}

				return node_types[builder_id];
			}

			IMany& FunctionBuilder::data() {
				return data_static[builder_id];
			}

			const IMany& FunctionBuilder::data() const {
				return data_static[builder_id];
			}

			std::string& FunctionBuilder::name() {
				return names[builder_id];
			}

			const std::string& FunctionBuilder::name() const {
				return names[builder_id];
			}

			bool FunctionBuilder::isNotUninitialized(const FunctionBuilder S) {
				if (S.FunctionBuilderNode() == FunctionBuilderNodeTypes::Uninitialized)
					return false;
				else if (S.FunctionBuilderNode() != FunctionBuilderNodeTypes::Uninitialized)
					return true;
				UNREACHABLE("True or not true");
				return {};
			}

			FunctionBuilder::FunctionBuilder() {
				builder_id = ++max_builder_id;
				FunctionBuilderNode() = FunctionBuilderNodeTypes::Uninitialized;
				data().clear();
				name() = "";
			}

			FunctionBuilder::FunctionBuilder(Tuple T) : FunctionBuilder() {
				FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
				data().InsertTypeEnd(T);
			}

			FunctionBuilder::FunctionBuilder(number_ n) : FunctionBuilder() {
				FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
				data().InsertTypeEnd(Tuple(n));
			}

			FunctionBuilder::FunctionBuilder(std::integral auto n) : FunctionBuilder() {
				FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
				data().InsertTypeEnd(Tuple(n));
			}

			FunctionBuilder::FunctionBuilder(Function F) : FunctionBuilder() {
				FunctionBuilderNode() = FunctionBuilderNodeTypes::CallFunctionNode;
				data().InsertTypeEnd(F);
			}

			void FunctionBuilder::setName(std::string s) {
				name() = s;
			}

			FunctionBuilder FunctionBuilder::Variable() {
				FunctionBuilder SB;
				SB.FunctionBuilderNode() = FunctionBuilderNodeTypes::VariableTerminal;
				SB.data().clear();
				return SB;
			}

			FunctionBuilder FunctionBuilder::BorrowFrom() {
				FunctionBuilder SB;
				SB.FunctionBuilderNode() = FunctionBuilderNodeTypes::UnsetBorrowFromNode;
				SB.data().clear();
				Function top;
				SB.data().InsertTypeEnd(top);
				return SB;
			}

			FunctionBuilder FunctionBuilder::operator+(const FunctionBuilder rhs) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(rhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPlus;
				ASSUME(result.data().SizeVector() == 2, "Must have 2 elements!");
				result.data().AssertAllType<FunctionBuilder>("Must have 2 FunctionBuilders!");
				return result;
			}

			FunctionBuilder FunctionBuilder::operator-(const FunctionBuilder rhs) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(rhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionMinus;
				return result;
			}

			FunctionBuilder FunctionBuilder::operator*(const FunctionBuilder rhs) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(rhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionProduct;
				return result;
			}

			FunctionBuilder FunctionBuilder::operator/(const FunctionBuilder rhs) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(rhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionQuotient;
				return result;
			}

			FunctionBuilder FunctionBuilder::operator%(const FunctionBuilder rhs) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(rhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionMod;
				return result;
			}

			FunctionBuilder FunctionBuilder::operator[](const FunctionBuilder index) const {
				return GetIndex(index);
			}

			FunctionBuilder FunctionBuilder::operator-() const {
				const FunctionBuilder& lhs = *this;
				const FunctionBuilder& rhs = Tuple(-1);
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(rhs);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionProduct;
				return result;
			}

			FunctionBuilder::FunctionBuilder(PolyTuple i) : FunctionBuilder::FunctionBuilder() {
				FunctionBuilderNode() = FunctionBuilderNodeTypes::ConstantTerminal;
				data().InsertTypeEnd(Tuple(i));
			}

			//Build Expression FunctionBorrowedNode
			FunctionBuilder FunctionBuilder::operator()(const FunctionBuilder shrink, const PolyTuple basecase) const {
				ASSUME(isNotUninitialized(shrink), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(*this);
				result.data().InsertTypeEnd<FunctionBuilder>(shrink);
				result.data().InsertTypeEnd<Tuple>(basecase);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionBorrowedNode;
				return result;
			}

			//Apply the function to the output of F rather than input
			FunctionBuilder FunctionBuilder::operator()(const FunctionBuilder F) const {
				const FunctionBuilder& applier = *this;
				ASSUME(isNotUninitialized(applier), "Must be initialized!");
				ASSUME(isNotUninitialized(F), "Must be initialized!");
				return ReplaceIdentity(F);
			}

			IComparisonToBool operator==(const FunctionBuilder& a, const FunctionBuilder& b) {
				if (a.builder_id != b.builder_id)
					return false;
				else if (a.builder_id == b.builder_id)
					return true;
				UNREACHABLE("operator==");
			}

			void FunctionBuilder::operator^=(const FunctionBuilder& F) {
				setBorrowed(F);
			}

			//Returns a function that can evaluate the constructed builder
			Function FunctionBuilder::buildFunction() const {
				Function return_value;
				switch (FunctionBuilderNode())
				{
				case FunctionBuilder::FunctionBuilderNodeTypes::Uninitialized: {
					PANIC("Uninitialized!");
					return {};
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::ConstantTerminal: {
					ASSERT(data().SizeVector() == 1, "Expecting 1 element!");
					Tuple ConstantValue = data().pickLast().GetCppType<Tuple>();
					SetConstant(return_value, ConstantValue);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::VariableTerminal: {
					ASSERT(data().SizeVector() == 0, "Expecting no element!");
					SetIdentity(return_value);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPlus: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					data().AssertAllType<FunctionBuilder>("Expecting 2 FunctionBuilders!");
					IAny a1 = data().nthElementVector(0);
					FunctionBuilder FB1 = a1.GetCppType<FunctionBuilder>();
					Function F1 = FB1.buildFunction();
					Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetAddFunction(return_value, F1, F2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMinus: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetSubtractFunction(return_value, F1, F2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionProduct: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetProductFunction(return_value, F1, F2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionQuotient: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetQuotientFunction(return_value, F1, F2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionMod: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					Function F1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function F2 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetRemainderFunction(return_value, F1, F2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPointwiseLess: {
					ASSERT(data().SizeVector() == 4, "Expecting 4 elements!");
					Function value = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function test = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					Function alt_1 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
					Function alt_2 = data().nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
					SetPointwiseLess(return_value, value, test, alt_1, alt_2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionPiecewiseLess: {
					ASSERT(data().SizeVector() == 3, "Expecting 3 elements!");
					Function alt_1 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function threshold = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					Function alt_2 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
					SetPiecewiseLess(return_value, threshold, alt_1, alt_2);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionLexicographicLess: {
					ASSERT(data().SizeVector() == 4, "Expecting 4 elements!");
					Function F0 = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function F1 = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					Function F2 = data().nthElementVector(2).GetCppType<FunctionBuilder>().buildFunction();
					Function F3 = data().nthElementVector(3).GetCppType<FunctionBuilder>().buildFunction();
					SetLexicographicLess(return_value, F0, F1, F2, F3);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionGetIndex: {
					ASSERT(data().SizeVector() == 2, "Expecting 2 elements!");
					Function tuple = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function index = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetGetIndex(return_value, index, tuple);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionSetIndex: {
					ASSERT(data().SizeVector() == 2, ((std::string)"Expecting 2 elements! Found: " + std::to_string(data().SizeVector())).c_str());
					Function tuple = data().nthElementVector(0).GetCppType<FunctionBuilder>().buildFunction();
					Function index = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					SetSetIndex(return_value, index, tuple);
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::FunctionBorrowedNode: {
					//Construct the BorrowNode
					ASSERT(data().SizeVector() == 3, "Expecting 3 elements!");
					//Get the stored function object:
					//Get the reference to the recursive call
					FunctionBuilder top_builder = (data().nthElementVector(0).GetCppType<FunctionBuilder>());
					ASSERT(top_builder.FunctionBuilderNode() == FunctionBuilderNodeTypes::SetBorrowFrom, "Must be set before construction");
					//Retrieve the stored function object from the recursive call object
					Function top = top_builder.data().nthElementVector(0).GetCppType<Function>();
					Function shrink = data().nthElementVector(1).GetCppType<FunctionBuilder>().buildFunction();
					Tuple    basecase = data().nthElementVector(2).GetCppType<Tuple>();
					SetBorrow(return_value, shrink, basecase, top);

					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::UnsetBorrowFromNode: {
					PANIC("Construction of an unset borrow-from node. Must set it first");
					UNREACHABLE("Panic");
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::SetBorrowFrom: {
					//The returned function_id must match the function_id of the stored function
					//Get the target functionbuilder
					FunctionBuilder target = data().nthElementVector(1).GetCppType<FunctionBuilder>();
					//This will call the "build" on the recursive operation
					//This call will call the switch at FunctionBuilder::FunctionBuilderNodeTypes::FunctionBorrowedNode
					// which will receive the zeroth element of this data, which is the function constructed here

					//id of return_value is this
					return_value = data().nthElementVector(0).GetCppType<Function>();

					//values associated with return_value is this
					//return_value.copyFunction(target.buildFunction());
					Function target_data = target.buildFunction();

					return_value.NodeClass() = target_data.NodeClass();
					return_value.FunctionNodes() = target_data.FunctionNodes();
					break;
				}
				case FunctionBuilder::FunctionBuilderNodeTypes::CallFunctionNode: {
					ASSERT(data().SizeVector() == 1, "Expecting 1 elements!");
					Function F0 = data().nthElementVector(0).GetCppType<Function>();
					return F0;
				}
				default:
					break;
				}
				if (name() != "")
					return_value.name() = name();
				return return_value;
				UNREACHABLE("Unreachable code!");
			}

			std::string FunctionBuilder::print_dbg() {
				TODO_("Improve function builder");
				std::stringstream s;
				s << "Type: " << (int)FunctionBuilderNode() << " ";
				if (FunctionBuilderNode() == FunctionBuilderNodeTypes::VariableTerminal) {
					s << "x";
					return s.str();
				}
				else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::ConstantTerminal) {
					s << "build: [" + data().pickLast().GetCppType<Tuple>().to_str() + "]";
					return s.str();
				}
				else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::UnsetBorrowFromNode) {
					s << "recurse: [" "]";
					return s.str();
				}
				else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::FunctionBorrowedNode) {
					s << "recurse: [" "]";
					return s.str();
				}
				else {
					s << "Elements: [";
					IMany::MonoVoidFunction PrintRecursive = [&s](IAny element) -> void {
						if (+element.HasCPPType<FunctionBuilder>()) {
							s << element.GetCppType<FunctionBuilder>("").print_dbg() << ", ";
							return;
						}
						else
							return;
						};
					data().OperateElement(PrintRecursive);
					s << "]";
					return s.str();
				}
			}

			FunctionBuilder FunctionBuilder::ReplaceIdentity(const FunctionBuilder& F) const {
				if (FunctionBuilderNode() == FunctionBuilderNodeTypes::VariableTerminal) {
					return F;
				}
				else if (FunctionBuilderNode() == FunctionBuilderNodeTypes::ConstantTerminal) {
					return *this;
				}
				else {
					IMany::MonoAnyFunction ReplaceRecursive = [&](IAny element) -> IAny {
						if (+element.HasCPPType<FunctionBuilder>())
							return IAny{ element.GetCppType<FunctionBuilder>("").ReplaceIdentity(F) };
						else
							return element;
						};
					FunctionBuilder FB = *this;
					FB.data() = data().TransformMap(ReplaceRecursive);
					//Everything else is unchanged, only data() is recursively transformed
					return FB;
				}
				UNREACHABLE("If-else");
			}

			//Sets a borrowed-from node to its value
			void FunctionBuilder::setBorrowed(const FunctionBuilder& F) {
				number_ x = builder_id;
				ASSUME(FunctionBuilderNode() == FunctionBuilderNodeTypes::UnsetBorrowFromNode, "Can only set Borrowed-from nodes!");
				ASSUME(F.FunctionBuilderNode() != FunctionBuilderNodeTypes::UnsetBorrowFromNode, "Cannot set to Borrowed-from nodes!");
				//When building it, return this function
				//When building recursive calls to this, use this function for "top"
				data().clear();
				data().InsertTypeEnd<Function>(Function());
				data().InsertTypeEnd(F);
				FunctionBuilderNode() = FunctionBuilderNodeTypes::SetBorrowFrom;
				ASSUME(FunctionBuilderNode() == FunctionBuilderNodeTypes::SetBorrowFrom, "It remains a recursive call!");
				ASSUME(builder_id == x, "builder_id should not change!");
			}

			//Build expression a[i] < b[i] ? c[i] : d[i]
			FunctionBuilder FunctionBuilder::BuildPointwiseLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt1, const FunctionBuilder alt2) {
				ASSUME(isNotUninitialized(value), "Must be initialized");
				ASSUME(isNotUninitialized(test), "Must be initialized");
				ASSUME(isNotUninitialized(alt1), "Must be initialized");
				ASSUME(isNotUninitialized(alt2), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(value);
				result.data().InsertTypeEnd<FunctionBuilder>(test);
				result.data().InsertTypeEnd<FunctionBuilder>(alt1);
				result.data().InsertTypeEnd<FunctionBuilder>(alt2);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPointwiseLess;
				return result;
			}

			//Build expression i < index ? alt_1[i] : alt_2[i]
			FunctionBuilder FunctionBuilder::BuildPiecewiseLess(const FunctionBuilder alt_1, const FunctionBuilder index, const FunctionBuilder alt_2) {
				ASSUME(isNotUninitialized(alt_1), "Must be initialized");
				ASSUME(isNotUninitialized(index), "Must be initialized");
				ASSUME(isNotUninitialized(alt_2), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(alt_1);
				result.data().InsertTypeEnd<FunctionBuilder>(index);
				result.data().InsertTypeEnd<FunctionBuilder>(alt_2);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionPiecewiseLess;
				return result;
			}

			//Build expression a < b ? c : d
			FunctionBuilder FunctionBuilder::BuildLexicographicLess(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt_1, const FunctionBuilder alt_2) {
				ASSUME(isNotUninitialized(value), "Must be initialized");
				ASSUME(isNotUninitialized(test), "Must be initialized");
				ASSUME(isNotUninitialized(alt_1), "Must be initialized");
				ASSUME(isNotUninitialized(alt_2), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(value);
				result.data().InsertTypeEnd<FunctionBuilder>(test);
				result.data().InsertTypeEnd<FunctionBuilder>(alt_1);
				result.data().InsertTypeEnd<FunctionBuilder>(alt_2);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionLexicographicLess;
				return result;
			}

			//Build Expression result[0] = lhs[index[0]]
			FunctionBuilder FunctionBuilder::GetIndex(const FunctionBuilder index) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(index), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(index);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionGetIndex;
				return result;
			}

			//Build Expression result[index[0]] = lhs[0]
			FunctionBuilder FunctionBuilder::SetIndex(const FunctionBuilder index) const {
				const FunctionBuilder& lhs = *this;
				ASSUME(isNotUninitialized(lhs), "Must be initialized");
				ASSUME(isNotUninitialized(index), "Must be initialized");
				FunctionBuilder result;
				result.data().InsertTypeEnd<FunctionBuilder>(lhs);
				result.data().InsertTypeEnd<FunctionBuilder>(index);
				result.FunctionBuilderNode() = FunctionBuilderNodeTypes::FunctionSetIndex;
				return result;
			}

			//Returns R: R[i] = T[i]
			//Ignores variable and returns T
			void FunctionBuilder::SetConstant(Function& F, Tuple T) {
				F.NodeClass() = Function::NodeType::Constant;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(T);
			}

			void FunctionBuilder::SetIdentity(Function& F) {
				F.NodeClass() = Function::NodeType::PointwiseIdentity;
				F.FunctionNodes().clear();
			}

			void FunctionBuilder::SetAddFunction(Function& F, Function lhs, Function rhs) {
				F.NodeClass() = Function::NodeType::PointwisePlus;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(lhs);
				F.FunctionNodes().InsertTypeEnd(rhs);
			}

			void FunctionBuilder::SetSubtractFunction(Function& F, Function lhs, Function rhs) {
				F.NodeClass() = Function::NodeType::PointwiseMinus;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(lhs);
				F.FunctionNodes().InsertTypeEnd(rhs);
			}

			void FunctionBuilder::SetProductFunction(Function& F, Function lhs, Function rhs) {
				F.NodeClass() = Function::NodeType::PointwiseProduct;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(lhs);
				F.FunctionNodes().InsertTypeEnd(rhs);
			}

			void FunctionBuilder::SetQuotientFunction(Function& F, Function lhs, Function rhs) {
				F.NodeClass() = Function::NodeType::PointwiseQuotient;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(lhs);
				F.FunctionNodes().InsertTypeEnd(rhs);
			}

			void FunctionBuilder::SetRemainderFunction(Function& F, Function lhs, Function rhs) {
				F.NodeClass() = Function::NodeType::PointwiseRemainder;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(lhs);
				F.FunctionNodes().InsertTypeEnd(rhs);
			}

			//Sets the recursive call - not the recursive definition
			void FunctionBuilder::SetBorrow(Function& F, Function shrink, Tuple basecase, Function top) {
				F.NodeClass() = Function::NodeType::Borrow;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(top);
				F.FunctionNodes().InsertTypeEnd(shrink);
				F.FunctionNodes().InsertTypeEnd(basecase);
			}

			//Returns R[i] := check[i] < threshold[i] ? ifLess[i] : ifEqualOrMore[i]
			void FunctionBuilder::SetPointwiseLess(Function& F, Function check, Function threshold, Function ifLess, Function ifEqualOrMore) {
				F.NodeClass() = Function::NodeType::PointwiseLess;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(check);
				F.FunctionNodes().InsertTypeEnd(threshold);
				F.FunctionNodes().InsertTypeEnd(ifLess);
				F.FunctionNodes().InsertTypeEnd(ifEqualOrMore);
			}

			//Returns R[-Infinity:threshold[0] - 1] = left[i], R[threshold, Infinity] = right[i]
			void FunctionBuilder::SetPiecewiseLess(Function& F, Function threshold, Function leftExclusive, Function rightInclusive) {
				F.NodeClass() = Function::NodeType::PiecewiseLess;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(leftExclusive);
				F.FunctionNodes().InsertTypeEnd(rightInclusive);
				F.FunctionNodes().InsertTypeEnd(threshold);
			}

			//Compares from most-significant element to least significant element, like a dictionary
			//If a[i] != b[i], early exit
			//Returns a < b ? ifLess : ifNotLess
			void FunctionBuilder::SetLexicographicLess(Function& F, Function a, Function b, Function ifLess, Function ifNotLess) {
				F.NodeClass() = Function::NodeType::LexicographicLess;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(a);
				F.FunctionNodes().InsertTypeEnd(b);
				F.FunctionNodes().InsertTypeEnd(ifLess);
				F.FunctionNodes().InsertTypeEnd(ifNotLess);
			}

			//R[a[0]] = b[0]
			void FunctionBuilder::SetSetIndex(Function& F, Function a, Function b) {
				F.NodeClass() = Function::NodeType::SetIndex;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(a);
				F.FunctionNodes().InsertTypeEnd(b);
			}

			//R[0] = b[a[0]]
			void FunctionBuilder::SetGetIndex(Function& F, Function a, Function b) {
				F.NodeClass() = Function::NodeType::GetIndex;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(a);
				F.FunctionNodes().InsertTypeEnd(b);
			}

			//R[0] = MaxHeight(a), R[1] = Minheight(a)
			void FunctionBuilder::SetMaxHeight(Function& F, Function a) {
				F.NodeClass() = Function::NodeType::MaxHeight;
				F.FunctionNodes().clear();
				F.FunctionNodes().InsertTypeEnd(a);
			}


			std::unordered_map<number_, IMany> FunctionBuilder::data_static{};
			number_ FunctionBuilder::max_builder_id = 0;
			std::unordered_map<number_, FunctionBuilder::FunctionBuilderNodeTypes> FunctionBuilder::node_types;
			std::unordered_map<number_, std::string> FunctionBuilder::names;
			//Describe the input variable as a functionbuilder node
			const FunctionBuilder var_ = FunctionBuilder::Variable();
			//A valueless variable to syntactically define SetIndex
			SetType set;

			//A function to construct a const_ from a listr of numbers
			FunctionBuilder const_(PolyTuple i) {
				return i;
			}

			PointwiseCase1::PointwiseCase1(FunctionBuilder value, FunctionBuilder test) : value(value), test(test) {}
			PointwiseCase2::PointwiseCase2(FunctionBuilder value, FunctionBuilder test, FunctionBuilder alt_1) : test(test), value(value), alt_1(alt_1) {}

			PointwiseCase2 operator||(PointwiseCase1 PC, FunctionBuilder alt_1) { return PointwiseCase2(PC.value, PC.test, alt_1); }
			FunctionBuilder operator||(PointwiseCase2 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPointwiseLess(PC.value, PC.test, PC.alt_1, alt_2); }
			PointwiseCase1 operator<(FunctionBuilder value, FunctionBuilder test) { return PointwiseCase1(value, test); }
			PointwiseCase1 operator>(FunctionBuilder value, FunctionBuilder test) { return PointwiseCase1(test, value); }

			PiecewiseCase1::PiecewiseCase1(FunctionBuilder alt_1, FunctionBuilder index) : alt_1(alt_1), index(index) {}

			FunctionBuilder operator|(PiecewiseCase1 PC, FunctionBuilder alt_2) { return FunctionBuilder::BuildPiecewiseLess(PC.alt_1, PC.index, alt_2); }
			PiecewiseCase1 operator|(FunctionBuilder alt_1, FunctionBuilder index) { return PiecewiseCase1(alt_1, index); }

			LexicographicCase2 operator||(const LexicographicCase1 PC, const FunctionBuilder alt_1) { return LexicographicCase2(PC.value, PC.test, alt_1); }
			FunctionBuilder operator||(const LexicographicCase2 PC, const FunctionBuilder alt_2) { return FunctionBuilder::BuildLexicographicLess(PC.value, PC.test, PC.alt_1, alt_2); }
			LexicographicCase1 operator<<(const FunctionBuilder value, const FunctionBuilder test) { return LexicographicCase1(value, test); }
			LexicographicCase1 operator>>(const FunctionBuilder value, const FunctionBuilder test) { return LexicographicCase1(test, value); }


			FunctionBuilder FunctionBuilder::operator[](const SetTypeOP S) const {
				return FunctionBuilder::SetIndex(S.F);
			}

			FunctionBuilder::SetTypeOP SetType::operator>>(const FunctionBuilder F) const {
				return FunctionBuilder::SetTypeOP(F);
			}

			LexicographicCase1::LexicographicCase1(const FunctionBuilder value, const FunctionBuilder test) : value(value), test(test) {}
			LexicographicCase2::LexicographicCase2(const FunctionBuilder value, const FunctionBuilder test, const FunctionBuilder alt_1) : test(test), value(value), alt_1(alt_1) {}

			//If Test(x) == case_, return Value(x), else return Main(x);
			FunctionBuilder CreateCase(Tuple case_, FunctionBuilder Value, FunctionBuilder Main, FunctionBuilder Test) {
				FunctionBuilder Identity = var_;
				FunctionBuilder Case = case_;
				FunctionBuilder IsLess = Test << Case || Main || Value;
				FunctionBuilder IsMore = Case << Test || Main || IsLess;
				return IsMore;
			}

			//base[index] = value, base[!index] = base
			FunctionBuilder CreatePiecewiseAt(FunctionBuilder base, FunctionBuilder index, FunctionBuilder value) {
				//F[-Inf:Index -1] = base; F[Index:Index] = value; F[Index +1:Inf] = base;
				FunctionBuilder FinalResult = base | index | value | index + 1 | base;
				return FinalResult;
			}

			//Return a function that performs pointwise x < 0 ? -x : x
			FunctionBuilder PointwiseAbs(FunctionBuilder base) {
				FunctionBuilder abs = var_ < 0 || -base || base;
				return abs;
			}
		}

		namespace simple_executor {

			template<bool weak>
			IComparisonResult IsLexLess(const Tuple& a, const Tuple& b);

			//Returns a constant tuple
			Tuple EvaluateConstant(Tuple target) {
				Tuple result = target;
				return result;
			};

			//Returns the input unchanged
			Tuple EvaluatePointwiseIdentity(Tuple input) {
				return input;
			};

			//Returns the "pointwise" sum of both inputs
			Tuple EvaluatePointwisePlus(Tuple input_a, Tuple input_b) {
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
			Tuple EvaluatePointwiseMinus(Tuple input_a, Tuple input_b) {
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
			Tuple EvaluatePointwiseProduct(Tuple input_a, Tuple input_b) {
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
			Tuple EvaluatePointwiseQuotient(Tuple input_a, Tuple input_b) {
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
			Tuple EvaluatePointwiseLess(Tuple input_a, Tuple input_b, Tuple input_c, Tuple input_d) {
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
			Tuple EvaluatePiecewiseLess(Tuple input_a, Tuple input_b, number_ threshold) {
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

			////R[i] := a[i] == threshold ? b[i] : c[i]
			//Tuple EvaluatePiecewiseEqual(Tuple input_a, Tuple input_b, number_ threshold) {
			//	Tuple result;
			//	std::unordered_set<number_> shared_indices;
			//	for (const auto& i : input_a.data) {
			//		shared_indices.insert(i.first);
			//	}
			//	for (const auto& i : input_b.data) {
			//		shared_indices.insert(i.first);
			//	}
			//	for (const auto& index : shared_indices) {
			//		number_ a_val = input_a.GetNumberConst_Index(index);
			//		number_ b_val = input_b.GetNumberConst_Index(index);
			//		if (index == threshold) {
			//			result.data[index] = a_val;
			//		}
			//		else if (index != threshold) {
			//			result.data[index] = b_val;
			//		}
			//	}
			//	return result;
			//}

			////R[i] := a[i] == threshold ? b[i] : c[i]
			//Tuple EvaluatePointwiseEqual(Tuple input_a, Tuple input_b, Tuple input_c, number_ threshold) {
			//	Tuple result;
			//	std::unordered_set<number_> shared_indices;
			//	for (const auto& i : input_a.data) {
			//		shared_indices.insert(i.first);
			//	}
			//	for (const auto& i : input_b.data) {
			//		shared_indices.insert(i.first);
			//	}
			//	for (const auto& i : input_c.data) {
			//		shared_indices.insert(i.first);
			//	}
			//	for (const auto& index : shared_indices) {
			//		number_ a_val = input_a.GetNumberConst_Index(index);
			//		number_ b_val = input_b.GetNumberConst_Index(index);
			//		number_ c_val = input_c.GetNumberConst_Index(index);
			//		if (a_val == threshold) {
			//			result.data[index] = b_val;
			//		}
			//		else if (a_val != threshold) {
			//			result.data[index] = c_val;
			//		}
			//	}
			//	return result;
			//}

			//R[a[0]] = b[0]
			Tuple EvaluateSetIndex(Tuple input_a, Tuple input_b) {
				Tuple result;
				number_ index = input_a.GetNumberConst_Index(0);
				result.GetNumber_Index(index) = input_b.GetNumberConst_Index(0);
				return result;
			}

			//R[0] = b[a[0]]
			Tuple EvaluateGetIndex(Tuple input_a, Tuple input_b) {
				Tuple result;
				number_ index = input_a.GetNumberConst_Index(0);
				result.GetNumber_Index(0) = input_b.GetNumberConst_Index(index);
				return result;
			}

			//Takes four parameters: Input, Recursive call argument generator, basecase, and full-function
			//Uses "shrinker" to "shrink" the operand to a lexicographically smaller tuple
			//If shrunk operand is L<= 0 or has any negative element at positive index; return the basecase
			//Return the result of applying the "top" function to the shrunk value
			//If top returns the "true" function, can be used to create recursive calls
			//eg: A borrow node with shrinker = x - 1, and a borrow node with shrinker = x - 2, on top(x) = borrow1 + borrow2 can represent the fibonacci function
			Tuple EvaluateBorrow(Tuple Op1, Function shrinker, Tuple basecase, Function top) {
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
				bool isNegative = +IsLexLess<true>(shrunk1, ZeroT);
				//If not Input <<= Shrunk
				bool isNotSmaller = not IsLexLess<true>(shrunk1, Op1);
				//If Input < 0 != 0
				bool hasAnyNegative = not Tuple::isZero(EvaluatePointwiseLess(shrunk1, ZeroT, shrunk1, ZeroT));
				//If Input > 0 == 0
				bool hasAnyNegativeRight = not Tuple::isZero(EvaluatePointwiseLess(shrunk1Right, ZeroT, shrunk1Right, ZeroT));

				if (isRightEmpty or isNegative or isNotSmaller or hasAnyNegativeRight) {
					return basecase;
				}
				return top(shrunk1);
			}

			//If lessOp1 < lessOp2, return ifLess else ifNotLess
			Tuple EvaluateLexicographicLess(Tuple lessOp1, Tuple lessOp2, Tuple ifLess, Tuple ifNotLess) {
				IComparisonResult x = +IsLexLess<false>(lessOp1, lessOp2);
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
			Tuple EvaluateMaxHeight(Tuple input)
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

			Tuple SimpleExecutor::ExecuteFunction(Function F, Tuple input) {
				input.normalize();
				CASES(F.NodeClass(),
					Function::NodeType::Uninitialized,
					Function::NodeType::Constant,
					Function::NodeType::PointwiseIdentity,
					Function::NodeType::PointwisePlus,
					Function::NodeType::PointwiseMinus,
					Function::NodeType::PointwiseProduct,
					Function::NodeType::PointwiseQuotient,
					Function::NodeType::PointwiseRemainder,
					Function::NodeType::PointwiseLess,
					Function::NodeType::PiecewiseLess,
					Function::NodeType::GetIndex,
					Function::NodeType::SetIndex,
					Function::NodeType::Borrow,
					Function::NodeType::LexicographicLess,
					Function::NodeType::MaxHeight
				);
				Tuple output;

				switch (F.NodeClass()) {
				case Function::NodeType::Uninitialized: {
					PANIC("Execute an unitialized function!");
					UNREACHABLE("Function should be initialized!");
					break;
				}
				case Function::NodeType::Constant: {
					ASSUME(F.FunctionNodes().SizeVector() == 1, "FunctionNode Constant must store 1 element!");
					Tuple target = F.FunctionNodes().pickLast().GetCppType<Tuple>("FunctionNode should store Tuple!");
					output = EvaluateConstant(target);
					break;
				}
				case Function::NodeType::PointwiseIdentity: {
					ASSUME(F.FunctionNodes().SizeVector() == 0, "FunctionNode Identity must store no elements!");
					output = EvaluatePointwiseIdentity(input);
					break;
				}
				case Function::NodeType::PointwisePlus: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode Plus must store two elements!");
					Function lhs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function rhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple lhs_t = lhs_f(input);
					Tuple rhs_t = rhs_f(input);
					output = EvaluatePointwisePlus(lhs_t, rhs_t);
					break;
				}
				case Function::NodeType::PointwiseMinus: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode Minus must store two elements!");
					Function lhs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function rhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple lhs_t = lhs_f(input);
					Tuple rhs_t = rhs_f(input);
					output = EvaluatePointwiseMinus(lhs_t, rhs_t);
					break;
				}
				case Function::NodeType::PointwiseProduct: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode Product must store two elements!");
					Function lhs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function rhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple lhs_t = lhs_f(input);
					Tuple rhs_t = rhs_f(input);
					output = EvaluatePointwiseProduct(lhs_t, rhs_t);
					break;
				}
				case Function::NodeType::PointwiseQuotient: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode Quotient must store two elements!");
					Function lhs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function rhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple    lhs_t = lhs_f(input);
					Tuple    rhs_t = rhs_f(input);
					output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
					break;
				}
				case Function::NodeType::PointwiseRemainder: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode Remainder must store two elements!");
					Function lhs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function rhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple    lhs_t = lhs_f(input);
					Tuple    rhs_t = rhs_f(input);
					output = EvaluatePointwiseQuotient(lhs_t, rhs_t);
					break;
				}
				case Function::NodeType::PointwiseLess: {
					ASSUME(F.FunctionNodes().SizeVector() == 4, "FunctionNode PointwiseLess must store four elements!");
					Function value = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function test = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Function alt_1 = F.FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
					Function alt_2 = F.FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
					Tuple    ahs_t = value(input);
					Tuple    bhs_t = test(input);
					Tuple    chs_t = alt_1(input);
					Tuple    dhs_t = alt_2(input);
					output = EvaluatePointwiseLess(ahs_t, bhs_t, chs_t, dhs_t);
					break;
				}
				case Function::NodeType::PiecewiseLess: {
					ASSUME(F.FunctionNodes().SizeVector() == 3, "FunctionNode PiecewiseLess must store three elements!");
					Function left_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function right_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Function threshold_f = F.FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
					Tuple    left_t = left_f(input);
					Tuple    right_t = right_f(input);
					Tuple    threshold = threshold_f(input);
					output = EvaluatePiecewiseLess(left_t, right_t, threshold.GetNumberConst_Index(0));
					break;
				}
				case Function::NodeType::GetIndex: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode GetIndex must store two elements!");
					Function ahs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function bhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple ahs_t = ahs_f(input);
					Tuple bhs_t = bhs_f(input);
					output = EvaluateGetIndex(ahs_t, bhs_t);
					break;
				}
				case Function::NodeType::SetIndex: {
					ASSUME(F.FunctionNodes().SizeVector() == 2, "FunctionNode SetIndex must store two elements!");
					Function ahs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function bhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Tuple ahs_t = ahs_f(input);
					Tuple bhs_t = bhs_f(input);
					output = EvaluateSetIndex(ahs_t, bhs_t);
					break;
				}
				case Function::NodeType::Borrow: {
					ASSUME(F.FunctionNodes().SizeVector() == 3, "FunctionNode Borrow must store 3 elements!");
					Function top = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("[0] element must be a Function Pointer! (Top)");
					Function shrink = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("[1] element must be a Function! (shrink)");
					Tuple basecase = F.FunctionNodes().nthElementVector(2).GetCppType<Tuple>("[2] element must be a Tuple! (basecase)");
					output = EvaluateBorrow(input, shrink, basecase, top);
					break;
				}
				case Function::NodeType::LexicographicLess: {
					ASSUME(F.FunctionNodes().SizeVector() == 4, "FunctionNode LexicographicLess must store four elements!");
					Function ahs_f = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Function bhs_f = F.FunctionNodes().nthElementVector(1).GetCppType<Function>("Second element must be a Function!");
					Function chs_f = F.FunctionNodes().nthElementVector(2).GetCppType<Function>("Third element must be a Function!");
					Function dhs_f = F.FunctionNodes().nthElementVector(3).GetCppType<Function>("Fourth element must be a Function!");
					Tuple ahs_t = ahs_f(input);
					Tuple bhs_t = bhs_f(input);
					Tuple chs_t = chs_f(input);
					Tuple dhs_t = dhs_f(input);
					output = EvaluateLexicographicLess(ahs_t, bhs_t, chs_t, dhs_t);
					break;
				}
				case Function::NodeType::MaxHeight: {
					ASSUME(F.FunctionNodes().SizeVector() == 1, "FunctionNode MaxHeight must store 1 elements!");
					Function a = F.FunctionNodes().nthElementVector(0).GetCppType<Function>("First element must be a Function!");
					Tuple a_t = a(input);
					output = EvaluateMaxHeight(a_t);
					break;
				}
				default:
					UNREACHABLE("All cases should be covered.");
					PANIC("Default Case!");
				}
				return output;
			}
			Tuple SimpleExecutor::ExecuteFunctionCached(Function F, Tuple input) {
				input.normalize();
				Tuple output;
				if (cache.find({ F, input }) != cache.end()) {
					return cache.at({ F, input });
				}
				else {
					output = ExecuteFunction(F, input);
				}

				if (cache.find({ F, input }) == cache.end()) {
					cache.insert({ { F, input }, output });
				}
				return output;
			}
			std::unordered_map<std::pair<Function, Tuple>, Tuple> SimpleExecutor::cache;
		}
	}
}