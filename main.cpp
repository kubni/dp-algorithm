#include "dp.h"
#include "lib.h"
#include <iostream>

void printCNF(const NormalForm &cnf) {
    for (const auto &clause : cnf) {
        std::cout << "[ ";
        for (const auto &lit : clause)
            std::cout << (lit.pos ? "" : "!") << lit.name << " ";
        std::cout << "]";
    }
    std::cout << std::endl;
}

int main() {
    FormulaPtr p = ptr(Atom{"p"});
    FormulaPtr q = ptr(Atom{"q"});
    FormulaPtr r = ptr(Atom{"r"});
    FormulaPtr pAndq = ptr(Binary{Binary::And, p, q});
    FormulaPtr f = ptr(Binary{Binary::Or, ptr(Not{pAndq}), r});

    NormalForm cnf = tseytin(f);
    auto res = perform_unit_propagation(cnf);
    if (std::holds_alternative<NormalForm>(res))
        // printCNF(as<NormalForm>(res));
        printCNF(std::get<NormalForm>(res));
    else
        std::cout << "Res is boolean: "
                  << (std::get<bool>(res) ? "SAT" : "UNSAT") << std::endl;

    return 0;
}
