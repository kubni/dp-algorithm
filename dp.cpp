#include "dp.h"
#include <algorithm>

// Returns true if SAT, false if UNSAT, NormalForm formula otherwise (we
// continue with the next step)
std::variant<NormalForm, bool> perform_unit_propagation(NormalForm &cnf) {

    // We repeat everything while there are unit clauses in our formula:
    while (true) {
        // If there are no more clauses at all, the formula is SAT:
        if (cnf.empty())
            return true;

        // NOTE: We need to go one-by-one with unit clause search.

        auto uit = std::ranges::find_if(
            cnf, [](const Clause &c) { return c.size() == 1; });

        // If there are no more unit clauses, we continue with the next step
        // (pure literal)
        if (uit == cnf.end())
            return cnf;

        // Extract the literal from the matched unit clause
        const Literal l = (*uit)[0];
        const Literal negative_l = Literal{!l.pos, l.name};

        /*
         If our literal is found with a same polarity in a clause, we remove
         the whole clause since its surely true (because our literal is
         true, and the clause is a disjunction)

         NOTE: erase_if will delete ALL of the clauses matching this predicate,
         which will also include the current unit clause we matched with uit
         */
        std::erase_if(cnf, [&l](const Clause &clause) {
            return std::ranges::contains(clause, l);
        });

        /*
         * If our literal is found with the opposite polarity in a clause,
         * we remove that opposite polarity literal from that clause.
         * Additionally, if that clause is not empty, we instantly return
         * UNSAT.
         */
        for (Clause &clause : cnf) {
            std::erase_if(clause, [&negative_l](const auto &literal) {
                return literal == negative_l;
            });

            if (clause.empty())
                return false; // NOTE: UNSAT
        }
    }
}
