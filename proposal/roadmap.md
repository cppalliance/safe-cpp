# Roadmap

Getting a robust build of Safe C++ requires a number of steps.

1. Lower all of AST to MIR when `#feature on safety` is asserted. Right now I only lower a subset of the AST. The big hurdle, which I've been working on, is lowering try-catch blocks. Borrow checking involves a lot of data flow analysis, and relocation involves something drop elaboration, and that requires data flow analysis and CFG surgery, which really requires a MIR representation.
1. Lower all of MIR to LLVM. This is pretty easy to do.
1. Lower all of AST to MIR even when `#feature on safety` is not on. Maintaining both AST->MIR->LLVM and AST->LLVM is a maintenance burden. I want to move both object models to the same backend.
1. Replace the constant evaluation interpreter with a MIR interpreter. Currently constexpr functions are evaluated directly from the AST. That's not so reliable in the new object model. Plus, interpreting AST in more than one spot is a maintenance burden.
1. Get a comprehensive standard library for Safe C++. There are a lot of degrees of freedom in the design. Do we want to augment existing containers with safe funtions, or replace them with containers that only have safe APIs? The goal is to reduce the exposure to unsafe APIs, so generally replacing existing containers with safe ones is my preferred approach. But what I'm doing here is not intended to be definitive. This in-development standard library is mostly to support compiler development. 
1. Finalize all the language semantics. What are the variances for pointers and legacy references? How do we treat phantom data? Can we provide safety for global variables with dynamic initializers? There are a number of remaining questions that are outside the scope of borrow checking and pattern matching and the major safety features.
