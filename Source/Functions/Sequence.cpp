#pragma once
#include "Sequence.h"

namespace std {
	size_t hash<c_star_star::sequence::SequencePoint>::operator()(const c_star_star::sequence::SequencePoint& other) const
	{
		return size_t(other.id % (1ULL << 63));
	}
}

namespace c_star_star {
	namespace sequence {
        
        bool SequencePoint::operator==(const SequencePoint& other) const
        {
            return id == other.id;
        }
        bool SequencePoint::operator!=(const SequencePoint& other) const
        {
            return !(*this == other);
        }

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
        
            //Start a new sequencepoint, with nothing pointing to it
            SequencePoint SequenceFlow::newSequencePointStart() {
                FIXME("Add locking, not done yet");
                SequencePoint SP;
                SP.id = ++totalSequencePoints;
                return SP;
            }

            //Associate 2 preexisting sequence points (assert if already has an after)
            void SequenceFlow::insertNewLink(SequencePoint before, SequencePoint after) {
                ASSUME(not isInBeforeAfter(before), "Should not reassign already assigned SequencePoint!");
                ASSUME(not isInSelectionNodes(before), "Should not reassign already assigned SequencePoint!");
                beforeAfter.insert({ before, after });
            }

            //Create a new sequence point, associate it to "before" and return the new point
            SequencePoint SequenceFlow::newSequencePointFollower(SequencePoint before) {
                SequencePoint after = newSequencePointStart();
                insertNewLink(before, after);
                return after;
            }

            //Insert a new sequence point with many follows
            SequencePoint SequenceFlow::newSequencePointChoice(SequencePoint before, IMany sp) {
                SequencePoint after = newSequencePointStart();
                insertNewLink(before, after);
                selectionNodes.insert({ after, sp });
                return after;
            }

            //Follow a single sequence point
            //If no follower, return EmptyAny
            //Otherwise, if simple, return the next point
            //Otherwise, if comnplex, return the zeroth choice
            IAny SequenceFlow::followSequencePointSimple(SequencePoint before) {
                if (not isInBeforeAfter(before))
                    return {};
                else if (isInBeforeAfter(before)) {
                    SequencePoint after = beforeAfter.at(before);
                    return IAny{ after };
                }
                UNREACHABLE("Cases covered!");
            }

            IAny SequenceFlow::followSequencePointChoice(SequencePoint before, number_ choice) {
                if (not isInSelectionNodes(before))
                    return IAny{};
                IMany choices = selectionNodes[before];
                choices.AssertAllType<SequencePoint>("Must all be SequencePoints");
                if (choices.SizeVector() < number::to_size_t(choice))
                    return IAny{};
                SequencePoint result = choices.nthElementVector(choice).GetCppType<SequencePoint>("Expected SequencePoint");
                return IAny{ result };
            }

            //Follow a choice point, by selecting a choice number_ (0 -> 0th choice, 1 -> 1st choice etc)
            //If the point is not a choice point, only 0 is a valid choice (simple follow)
            //If the index is invalid, return EmptyAny
            IAny SequenceFlow::followSequencePointAggregate(SequencePoint before, number_ choice) {
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
                    if (selectionNodes[before].SizeVector() < number::to_size_t(choice))
                        return IAny{};

                }
            }

            //If beforeAfter has a "before"
            bool SequenceFlow::isInBeforeAfter(SequencePoint before) const {
                return not (beforeAfter.contains(before));
            }

            //If selectionNodes has a "before"
            bool SequenceFlow::isInSelectionNodes(SequencePoint before) const {
                return not (selectionNodes.contains(before));
            }

        number_ SequenceFlow::totalSequencePoints = 0;

	}
}