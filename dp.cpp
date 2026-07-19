#include "dp.h"
#include <algorithm>
#include <set>

Literal negate_literal(const Literal &l) { return Literal{!l.pos, l.name}; }

/* Returns true if SAT, false if UNSAT, NormalForm formula otherwise (we
   continue with the next step) */
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
        const Literal negative_l = negate_literal(l);

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
         * Additionally, if that clause is empty, we instantly return
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

bool does_contain_tautology(Clause &resolvent) {
    for (const Literal &l : resolvent) {
        auto it = std::ranges::find_if(resolvent, [&l](const Literal &l2) {
            return l2.name == l.name && l2.pos == !l.pos;
        });

        /* If the find_if iterator isn't at the end, it means we have a match,
         * and therefore a tautology */
        if (it != resolvent.end()) {
            return true;
        }
    }
    return false;
}

template <typename T> void deduplicate(std::vector<T> &v) {
    std::sort(v.begin(), v.end());
    auto dup = std::ranges::unique(v);
    v.erase(dup.begin(), dup.end());
}

std::variant<NormalForm, bool> elimination(NormalForm &cnf,
                                           const Literal &pivot) {
    std::set<Clause> P, N, R;
    for (auto i = 0; i < cnf.size(); i++) {
        const Clause &clause = cnf[i];
        if (std::ranges::contains(clause, pivot))
            P.insert(clause);
        else if (std::ranges::contains(clause, negate_literal(pivot)))
            N.insert(clause);
        else
            R.insert(clause);
    }

    // 3) Generate all resolvents on pivot
    NormalForm S_prim(R.begin(), R.end());
    for (const Clause &p_clause : P) {
        Clause pc_without_pivot = p_clause;
        std::erase(pc_without_pivot, pivot);
        for (const Clause &n_clause : N) {
            Clause nc_without_pivot = n_clause;
            std::erase(nc_without_pivot, negate_literal(pivot));

            // NormalForm resolvent = {pc_without_pivot, nc_without_pivot};
            // NOTE: resolvent is a Clause, not NormalForm
            Clause resolvent = pc_without_pivot;
            resolvent.append_range(nc_without_pivot);

            // 4) Process the resolvent
            /* Check whether the clause is a tautology. If it is, don't add it
               to S' */
            if (does_contain_tautology(resolvent))
                continue;

            // If the resolvent is an empty clause, return UNSAT
            if (resolvent.empty())
                return false;
            /*
              Deduplicate the resolvent, since we could have resolvent be
              p || p
             */
            deduplicate(resolvent);

            // 5) Add the resolvent to the S'
            S_prim.push_back(resolvent);
        }
    }

    // deduplicate S_prim
    deduplicate(S_prim);

    // Check if S_prim is empty set of clauses. If it is, return SAT
    if (S_prim.empty())
        return true;

    return S_prim;
}

bool dp(NormalForm &cnf) {
    while (true) {
        // Unit propagation step
        auto res = perform_unit_propagation(cnf);
        if (std::holds_alternative<NormalForm>(res)) {
            cnf = std::get<NormalForm>(res);

            // Pure literal step
            cnf = pure_literal(cnf);
            if (cnf.empty())
                return true; // SAT, because of empty clause set

            // Elimination step
            /* Pick the first literal in the first clause as the pivot.
             * This, hopefully, doesn't break correctness of the algorithm,
             * since every iteration we are guaranteed to pick another variable.
             *
             * TODO: A smarter way to pick the pivot? For example, maybe we
             * could measure the impact the pivot choice has on the number of
             * newly added clauses.
             */
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
