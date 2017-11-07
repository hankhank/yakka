# About
A very basic xgboost tree compiler. 
Takes the common xgboost tree dump format and using llvm spits out a function 
that will evaluate it given fixed memory locations.

For use in applications were you want to jit the xgboost tree.

Does not currently support missing data branches.

