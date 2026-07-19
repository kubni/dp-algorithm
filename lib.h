#ifndef LIB_H_
#define LIB_H_

#include <map>
#include <memory>
#include <variant>
#include <vector>

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

using Valuation = std::map<std::string, bool>;

struct Literal {
    bool pos;
    std::string name;

    bool operator==(const Literal &other) const {
        return pos == other.pos && name == other.name;
    }

    bool operator<(const Literal &other) const {
        if (name != other.name)
            return name < other.name;
        return pos < other.pos;
    }
};

using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

std::string tseytinRec(const FormulaPtr &f, int &subCount, NormalForm &cnf);
NormalForm tseytin(const FormulaPtr &f);

#endif // LIB_H_
