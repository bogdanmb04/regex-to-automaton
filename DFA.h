#pragma once

#include "Automaton.h"
#include <set>
#include <queue>

namespace automaton
{
    using primeTransition = std::pair<std::set<state>, char>;

    struct PrimeHash {
        size_t operator () (const std::set<state>& x) const
        {
            size_t seed = 0;
            for (auto elem : x)
            {
                seed ^= std::hash<state>()(elem);
            }
            return seed;
        }
    };

    struct PrimeTransitionHash {
        size_t operator () (const primeTransition& x) const {
            size_t seed = 0;
            for (auto elem : x.first) {
                seed ^= std::hash<state>()(elem);
            }
            seed ^= std::hash<char>()(x.second);
            return seed;
        }
    };

    class DeterministicFiniteAutomaton : public Automaton
    {
    public:
        explicit DeterministicFiniteAutomaton(const automaton::Automaton& automat);
        std::ostream& PrintAutomaton(std::ostream& os);
        bool CheckWord(const std::string& word);
    private:
        std::unordered_set<state> LambdaEncloseState(state) const;
        std::unordered_set<state> PrimeLambdaEnclose(const std::unordered_set<state>& primeState) const;
        std::unordered_set<state> PrimeDeltaFunction(const std::unordered_set<state>& toCheck, char symbol);
        void GeneratePrimeTransitions();
        void OverrideAutomaton();
    private:
        std::unordered_set<std::set<state>, PrimeHash> m_primeStates;
        std::unordered_map<std::set<state>, state, PrimeHash> m_primeStatesMapping;
        std::unordered_map<primeTransition, std::set<state>, PrimeTransitionHash> m_primeTransitions;
    };

    std::ostream& operator << (std::ostream& os, const DeterministicFiniteAutomaton& automaton);

}
