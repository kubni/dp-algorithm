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
    FormulaPtr pAndq = ptr(Binary{Binary::And, p, q});
    FormulaPtr f = ptr(Not{pAndq});

    NormalForm cnf = tseytin(f);
    std::cout << "Formula: !(p & q)\n";
    std::cout << "Tseytin CNF: ";
    printCNF(cnf);

    FormulaPtr eq = ptr(Binary{Binary::Eq, p, q});
    std::cout << "\nFormula: (p <-> q)\n";
    std::cout << "Tseytin CNF: ";
    printCNF(tseytin(eq));

    return 0;
}
