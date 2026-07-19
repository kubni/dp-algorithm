#ifndef DP_H_
#define DP_H_

#include "lib.h"

/* std::variant<NormalForm, bool> perform_unit_propagation(NormalForm &cnf); */
/* NormalForm pure_literal(NormalForm &cnf); */
/* std::variant<NormalForm, bool> elimination(NormalForm &cnf, const Literal&
 * pivot); */
bool dp(NormalForm &cnf);
#endif // DP_H_
