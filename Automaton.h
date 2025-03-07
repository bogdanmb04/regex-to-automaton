
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <variant>
#include <algorithm>

std::string RegexToPolishForm(const std::string &regex);

namespace automaton
{
    inline auto lambda = "λ";
    inline auto delta = "δ";
    inline auto sigma = "Σ";
    inline auto vid = "∅";

    using state = std::uint16_t;
    using transition = std::pair<state, std::variant<char, const char*>>;

    struct Hash
    {
        std::size_t operator () (const transition& t) const
        {
            return std::hash<state>()(t.first) ^ std::hash<int>()(t.second.index());
        }
    };

    class Automaton
    {
    public:
        Automaton(state initialState, state finalState, char character);
        Automaton(state initialState, state finalState);
        Automaton(const Automaton& auto1, const Automaton& auto2);
        virtual ~Automaton() = default;

    public:
        bool VerifyAutomaton() const;
        std::uint16_t GetStartState() const;
        std::uint16_t GetFinalState() const;
        const std::unordered_set<char>& GetAlphabet() const;
        const std::unordered_set<state>& GetStates() const;
        const std::unordered_map<transition, std::unordered_set<state>, Hash>& GetDeltaFunction() const;
        void TieAutomatons(const Automaton& auto1, const Automaton& auto2);
        void Kleene(const Automaton& automat);

    protected:
        std::unordered_set<state> m_states;
        std::unordered_set<char> m_alphabet;
        state m_initialState;
        state m_finalState;
        std::unordered_map<transition, std::unordered_set<state>, Hash> m_deltaFunction;
    };

    std::ostream& operator << (std::ostream& os, const Automaton& automaton);

}

