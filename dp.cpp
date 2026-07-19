#include "dp.h"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <set>

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

std::variant<NormalForm, bool> process_resolvent(NormalForm &resolvent) {
    // Drop tautologies (clauses that have both x and !x)
    // for (const Clause &c : resolvent) {
    // TODO: Invalidation?
    std::erase_if(resolvent, [](const Clause &c) {
        for (const Literal &l : c) {
            auto it = std::ranges::find_if(c, [&l](const Literal &l2) {
                return l2.name == l.name && l2.pos == !l.pos;
            });

            /*
              If iterator isn't at end(), it means we have a match,
              which means the clause should be deleted, so we return true
            */
            if (it != c.end()) {
                return true;
            }
        }
        return false;
    });

    // Deduplicate resolvent
    // unique() works on consecutive duplicated elements, so we need to sort
    // Sort literals in the each clause:
    for (Clause &c : resolvent)
        std::sort(c.begin(), c.end());
    std::sort(resolvent.begin(), resolvent.end());
    auto dup = std::ranges::unique(resolvent);
    resolvent.erase(dup.begin(), dup.end());

    // Check if the resolvent is an empty set now, and return UNSAT if so
    if (resolvent.empty())
        return false; // NOTE: "UNSAT";

    return resolvent;
}

std::variant<NormalForm, bool> elimination(NormalForm &cnf,
                                           const Literal &pivot) {
    // Note all of the variable names
    // TODO: Unneeded?
    std::set<std::string> variable_names;
    for (const Clause &c : cnf) {
        for (const Literal &l : c) {
            variable_names.insert(l.name);
        }
    }

    // Use while(true) and search for them with find_first_of
    // or just find_if ?

    // Pick the first literal in the first clause as the pivot
    // TODO: is this strong enough? We remove the pivot later from every
    // clause so each time this will be a new variable.
    // const Literal pivot = cnf[0][0]; FIXME
    std::set<Clause> P, N, R; // TODO: Clause vs const Clause& vs const Clause
    for (auto i = 0; i < cnf.size(); i++) {
        const Clause &clause = cnf[i];
        if (std::ranges::contains(clause, pivot)) {
            pivot.pos ? P.insert(clause) : N.insert(clause);
        } else
            R.insert(clause);

        // // Drop tautologies
        // TODO: Here or in process_resolvents
        // if (P.contains(clause) && N.contains(clause)) {
        //     P.erase(clause);
        //     N.erase(clause);

        //     // Erase it from the cnf too
        //     // TODO: IS it safe to do here since we iterate over cnf?
        //     //  If it isn't, we could remember its index and do it outside of
        //     //  the loop
        //     std::erase(cnf, clause);
        // }
    }

    // 3) Generate all resolvents on pivot
    NormalForm S_prim(R.begin(), R.end());
    // TODO: Time complexity is too high?
    for (const Clause &p_clause : P) {
        Clause pc_without_pivot = p_clause;
        std::erase(pc_without_pivot, pivot);
        for (const Clause &n_clause : N) {
            Clause nc_without_pivot = n_clause;
            std::erase(nc_without_pivot, pivot);

            NormalForm resolvent = {pc_without_pivot, nc_without_pivot};

            // 4) Process the resolvents
            auto res = process_resolvent(resolvent);
            if (std::holds_alternative<bool>(res))
                return false; // UNSAT
            else {
                resolvent = std::get<NormalForm>(res);

                // 5) Add the resolvent to the S'
                S_prim.append_range(resolvent);
            }
        }
    }

    return S_prim;
}

bool dp(NormalForm &cnf) {
    while (true) {
        // Unit propagation step
        auto res = perform_unit_propagation(cnf);
        if (std::holds_alternative<NormalForm>(res)) {
            NormalForm cnf = std::get<NormalForm>(res);

            // Pure literal step
            cnf = pure_literal(cnf);

            // Elimination step

            // Pick the first literal in the first clause as the pivot
            // TODO: is this strong enough? We remove the pivot later from every
            // clause so each time this will be a new variable.
            const Literal pivot = cnf[0][0];
            res = elimination(cnf, pivot);
            if (std::holds_alternative<NormalForm>(res))
                cnf = std::get<NormalForm>(res);
            else
                return std::get<bool>(res);

        } else
            return std::get<bool>(res);
    }
}
