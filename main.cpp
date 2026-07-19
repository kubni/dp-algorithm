#include "dp.h"
#include "lib.h"
#include <iostream>

int main() {
    // FormulaPtr p = ptr(Atom{"p"});
    // FormulaPtr q = ptr(Atom{"q"});
    // FormulaPtr r = ptr(Atom{"r"});
    // FormulaPtr pAndq = ptr(Binary{Binary::And, p, q});
    // FormulaPtr f = ptr(Binary{Binary::Or, ptr(Not{pAndq}), r});

    // NormalForm cnf = tseytin(f);
    // print_cnf(cnf);
    // std::cout << (dp(cnf) ? "SAT" : "UNSAT") << std::endl;  // SAT

    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr np = ptr(Not{p});
    FormulaPtr nq = ptr(Not{q});

    FormulaPtr c1 = ptr(Binary{Binary::Or, p, q});
    FormulaPtr c2 = ptr(Binary{Binary::Or, p, nq});
    FormulaPtr c3 = ptr(Binary{Binary::Or, np, q});
    FormulaPtr c4 = ptr(Binary{Binary::Or, np, nq});

    FormulaPtr f = ptr(Binary{Binary::And, ptr(Binary{Binary::And, c1, c2}),
                              ptr(Binary{Binary::And, c3, c4})});

    NormalForm cnf = tseytin(f);
    print_cnf(cnf);
    std::cout << (dp(cnf) ? "SAT" : "UNSAT") << std::endl; // UNSAT

    return 0;
}
