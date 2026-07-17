#include "dp.h"
#include <algorithm>

std::vector<int> find_unit_clauses(const NormalForm &cnf) {
    std::vector<int> unit_clause_indices;
    for (int i = 0; i < cnf.size(); i++) {
        // If we find an unit clause, remember its original index
        if (cnf[i].size() == 1)
            unit_clause_indices.push_back(i);
    }

    return unit_clause_indices;
}

/* struct Literal { */
/*     bool pos; */
/*     std::string name; */
/* }; */

/* using Clause = std::vector<Literal>; */
/* using NormalForm = std::vector<Clause>; */

// Returns true if SAT, false if UNSAT, NormalForm formula otherwise (we
// continue with the next step)
std::variant<NormalForm, bool> perform_unit_propagation(NormalForm &cnf) {

    // We repeat everything while there are unit clauses in our formula:
    while (true) {
        // If there are no more clauses at all, the formula is SAT:
        if (!cnf.size())
            return true;

        // If there are no more unit clauses, we continue with the next step
        // (pure literal)
        std::vector<int> unit_clause_indices = find_unit_clauses(cnf);
        if (!unit_clause_indices.size())
            return cnf;

        std::vector<Literal> unit_clause_literals;
        for (int i : unit_clause_indices) {
            // Get the literal from that unit clause, behave as if its satisfied
            Literal unit_literal = cnf[i][0];
            // unit_literal.pos = true; // NOTE: Wrong, the whole literal
            // should be true, the current approach breaks negative literals

            // And remember it
            unit_clause_literals.push_back(unit_literal);
        }

        // Delete all of the unit clauses found
        // TODO TODO: Check whether the algorithm is correct if we erase every
        // unit clause (like we do here) first, instead of one by one?
        // Seems like it should be, but doesn't hurt to check.
        std::erase_if(cnf,
                      [](const auto &clause) { return clause.size() == 1; });

        // Now, for each literal we found before, we need to go through all
        // other clauses:
        for (const Literal &l : unit_clause_literals) {
            /*
             If our literal is found with a same polarity in a clause, we remove
             the whole clause since its surely true (because our literal is
             true, and the clause is a disjunction)
             */
            std::erase_if(cnf, [&l](const auto &clause) {
                return std::ranges::contains(clause, l);
            });

            /*
             * If our literal is found with the opposite polarity in a clause,
             * we remove that opposite polarity literal from that clause.
             * Additionally, if that clause is not empty, we instantly return
             * UNSAT.
             */
            for (Clause &clause : cnf) {
                std::erase_if(
                    clause, [&l](const auto &literal) { return literal == l; });
                if (clause.size() == 0)
                    return false; // NOTE: UNSAT
            }
        }
    }
}
