#ifndef LIB_H_
#define LIB_H_

#include <iostream>
#include <map>
#include <memory>
#include <variant>

// Structures for defining a Formula
struct False;
struct True;
struct Atom;
struct Not;
struct Binary;

using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False {};
struct True {};
struct Atom {
    std::string name;
};
struct Not {
    FormulaPtr subformula;
};
struct Binary {
    enum Type { And, Or, Impl, Eq } type;
    FormulaPtr left, right;
};

FormulaPtr ptr(const Formula &f);

template <typename T> bool is(const FormulaPtr &f) {
    return std::holds_alternative<T>(*f);
}

template <typename T> T as(const FormulaPtr &f) { return std::get<T>(*f); }

// Valuation
using Valuation = std::map<std::string, bool>;

#endif // LIB_H_
