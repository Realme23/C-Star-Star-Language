# C-Star-Star-Language
A simple programming language. The idea is to try and make the writing and creation of formally verified code feasible.

The code is not currently suitable for use. Until the front-end has been written, all programming has to be done by modifying the source code. A full rewrite is expected once the prototyping is finished.

The name of the language is far from final - naming things is one of the hardest problems in computer science after all.

## Target architecture:
Front-end:  Parse the language, verify semantic checks and emit "IR" code.

Middle-end: Describe the programming with a single data-type: the "Tuple". Optimize as much as possible, while not moving through optimization barriers.

Back-end:   Use a description of the target to generate code.

A finished version of this language is expected to support C as the front-end and as the back-end.

## Ideas
1. Ghost variables: Variables that are asserted by the programmer to be guaranteed to be eliminated by dead code elimination (but can be used to assert on the validity of the code). These variables need not be const. Think of union tags or pointer provenances.
2. User defined types: These types are inspired by classes, but describe a set of semantic concepts rather than an API. The optimizations permitted are only those specified by their specification.
3. Maximize the purity of all functions as much as possible. This enables effective (but slow) algorithm-level optimizations. An implementation can theoretically derive eg Real Quantifier Elimination given only the definition of a Real Number.
4. The specification of the backend can be written in terms of user defined types. Translation is then accomplished by converting the code to be expressed purely in terms of the target, which can then be translated very easily.

## Tuples
 Tuples are finitely sized vectors of bigints. (In the current implementation, the index can also be negative.)
 This datatype was chosen because all calculations with a finite amount of memory can be represented as transformations of a single Tuple.
 It is also possible to merge multiple Tuples losslessly into a single Tuple.

# Formal verification
 Although written code is designed to be amenable to formal verification, such infrastructure does not exist yet. Suggestions are welcome.
