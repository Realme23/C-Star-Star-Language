#pragma once
#include "../Main.h"
#include "../Polymorphic/IMany.h"
#include "../Datatypes/Tuple.h"
#include "../Datatypes/Number.h"

namespace c_star_star {
    namespace sequence {
        class SequencePoint;
        class Sequence;
    }
}

namespace std {
    template<>
    struct hash<c_star_star::sequence::SequencePoint> {
        size_t operator()(const c_star_star::sequence::SequencePoint& other) const;
    };
}

namespace c_star_star {
	namespace sequence {
        using c_star_star::polymorphic::IMany;
        using c_star_star::polymorphic::IAny;
        using c_star_star::data_types::Tuple;
        using c_star_star::interpreter::IComparisonResult;
        using c_star_star::interpreter::IComparisonToBool;

        //A tag class for sequence points
        class SequencePoint {
            number_ id;
            friend class SequenceFlow;
            friend struct std::hash<SequencePoint>;
        public:
            bool operator==(const SequencePoint& other) const;
            bool operator!=(const SequencePoint& other) const;
        };

        //A flow chart representing the sequence
        class SequenceFlow {
        public:
            //Start a new sequencepoint, with nothing pointing to it
            SequencePoint newSequencePointStart();

            //Associate 2 preexisting sequence points (assert if already has an after)
            void insertNewLink(SequencePoint before, SequencePoint after);

            //Create a new sequence point, associate it to "before" and return the new point
            SequencePoint newSequencePointFollower(SequencePoint before);

            //Insert a new sequence point with many follows
            SequencePoint newSequencePointChoice(SequencePoint before, IMany sp);

            //Follow a single sequence point
            //If no follower, return EmptyAny
            //Otherwise, if simple, return the next point
            //Otherwise, if comnplex, return the zeroth choice
            IAny followSequencePointSimple(SequencePoint before);

            IAny followSequencePointChoice(SequencePoint before, number_ choice);

            //Follow a choice point, by selecting a choice number_ (0 -> 0th choice, 1 -> 1st choice etc)
            //If the point is not a choice point, only 0 is a valid choice (simple follow)
            //If the index is invalid, return EmptyAny
            IAny followSequencePointAggregate(SequencePoint before, number_ choice);

        private:
            //A single-key multi-value map from after-points to before-points
            std::unordered_map<SequencePoint, SequencePoint> beforeAfter;
            //A list of nodes with multiple outputs
            std::unordered_map<SequencePoint, IMany> selectionNodes;
            //A number_ associated to each ordering, can be used to enforce far-apart before/after relations
            std::unordered_map<SequencePoint, number_> orderingAssertNumber;
            static number_ totalSequencePoints;

            //If beforeAfter has a "before"
            bool isInBeforeAfter(SequencePoint before) const;

            //If selectionNodes has a "before"
            bool isInSelectionNodes(SequencePoint before) const;
        };
	}
}
