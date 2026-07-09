#include "lib.h"
FormulaPtr ptr(const Formula &f) { return std::make_shared<Formula>(f); }
