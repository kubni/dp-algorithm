#include "lib.h"
#include <iostream>
#include <utility>
FormulaPtr ptr(const Formula &f) { return std::make_shared<Formula>(f); }

std::string tseytinRec(const FormulaPtr &f, int &subCount, NormalForm &cnf) {
    if (is<False>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{false, sub}});
        return sub;
    }
    if (is<True>(f)) {
        std::string sub = "s" + std::to_string(++subCount);
        cnf.push_back({Literal{true, sub}});
        return sub;
    }
    if (is<Atom>(f))
        return as<Atom>(f).name;
    if (is<Not>(f)) {
        std::string subformula =
            tseytinRec(as<Not>(f).subformula, subCount, cnf);
        std::string substitution = "s" + std::to_string(++subCount);
        cnf.push_back(
            {Literal{false, subformula}, Literal{false, substitution}});
        cnf.push_back({Literal{true, subformula}, Literal{true, substitution}});
        return substitution;
    }

    Binary b = as<Binary>(f);
    std::string l = tseytinRec(b.left, subCount, cnf);
    std::string r = tseytinRec(b.right, subCount, cnf);
    std::string sub = "s" + std::to_string(++subCount);
    if (b.type == Binary::And) {
        cnf.push_back({Literal{false, sub}, Literal{true, l}});
        cnf.push_back({Literal{false, sub}, Literal{true, r}});
        cnf.push_back(
            {Literal{true, sub}, Literal{false, l}, Literal{false, r}});
        return sub;
    }

    // s_i <=> (l || r)
    if (b.type == Binary::Or) {
        // (!s || l || r) && (!l || s_i) && (!r || s_i)
        cnf.push_back(
            {Literal{false, sub}, Literal{true, l}, Literal{true, r}});
        cnf.push_back({Literal{true, sub}, Literal{false, l}});
        cnf.push_back({Literal{true, sub}, Literal{false, r}});
        return sub;
    }

    // s_i <=> (l => r)
    if (b.type == Binary::Impl) {
        // (!s || !l || r) && (l || s) && (!r || s)
        cnf.push_back(
            {Literal{false, sub}, Literal{false, l}, Literal{true, r}});
        cnf.push_back({Literal{true, sub}, Literal{true, l}});
        cnf.push_back({Literal{true, sub}, Literal{false, r}});
        return sub;
    }

    // s <=> (l <=> r)
    if (b.type == Binary::Eq) {
        // (¬s ∨ ¬l ∨ r)
        // (¬s ∨ l ∨ ¬r)
        // (s ∨ ¬l ∨ ¬r)
        // (s ∨ l ∨ r)
        cnf.push_back(
            {Literal{false, sub}, Literal{false, l}, Literal{true, r}});
        cnf.push_back(
            {Literal{false, sub}, Literal{true, l}, Literal{false, r}});
        cnf.push_back(
            {Literal{true, sub}, Literal{false, l}, Literal{false, r}});
        cnf.push_back({Literal{true, sub}, Literal{true, l}, Literal{true, r}});
        return sub;
    }

    std::unreachable();
}

NormalForm tseytin(const FormulaPtr &f) {
    NormalForm cnf;
    int subCount = 0;
    std::string sub = tseytinRec(f, subCount, cnf);
    cnf.push_back({Literal{true, sub}});
    return cnf;
}

void print_cnf(const NormalForm &cnf) {
    for (const auto &clause : cnf) {
        std::cout << "[ ";
        for (const auto &lit : clause)
            std::cout << (lit.pos ? "" : "!") << lit.name << " ";
        std::cout << "]";
    }
    std::cout << std::endl;
}
