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
            std::erase_if(clause, [&negative_l](const Literal &literal) {
                return literal == negative_l;
            });

            if (clause.empty())
                return false; // NOTE: UNSAT
        }
    }
}

NormalForm pure_literal(NormalForm &cnf) {
    /* We need a loop, because one iteration of the algorithm may create new
     pure literals */
    while (true) {
        // TODO: A better way to do this, or at least not in every iteration?
        std::map<std::string, int> literal_appearance_count;
        for (const Clause &clause : cnf) {
            for (const Literal &literal : clause) {
                // We want to keep literals with same name, but different
                // polarity, separate from each other for the count.
                literal.pos ? literal_appearance_count[literal.name]++
                            : literal_appearance_count["!" + literal.name]++;
            }
        }

        /* Find the literal whose opposite polarity doesn't exist in any other
           clause */
        auto res_it = std::ranges::find_if(
            literal_appearance_count,
            [&literal_appearance_count](std::pair<std::string, int> p) {
                return p.first.starts_with("!")
                           ? !literal_appearance_count.contains(
                                 p.first.substr(1))

                           : !literal_appearance_count.contains("!" + p.first);
            });

        /* The case where there isn't a single pure literal found,
           we stop the algorithm at that point. */
        if (res_it == literal_appearance_count.end())
            return cnf;

        // Reconstruct the pure literal (since lit is a kv pair from the map)
        bool is_literal_negative = res_it->first.contains("!");

        const Literal pure_lit = Literal{
            !is_literal_negative,
            is_literal_negative ? res_it->first.substr(1) : res_it->first};

        // We need to remove every clause that contains the found pure literal
        std::erase_if(cnf, [&pure_lit](const Clause &clause) {
            return std::ranges::contains(clause, pure_lit);
        });
    }
}
