#ifndef DP_H_
#define DP_H_

#include "lib.h"

std::variant<NormalForm, bool> perform_unit_propagation(NormalForm &cnf);

#endif // DP_H_
