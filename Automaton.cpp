#include "Automaton.h"

#include <bits/fs_fwd.h>

using namespace automaton;

int priority(const char c) {
    switch (c) {
        case '|': return 0;
        case '.': return 1;
        case '*': return 2;
        default: return -1;
    }
}

template<typename T>
std::unordered_set<T> TieSets(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2)
{
    std::unordered_set<T> mergedSet;
    mergedSet.insert(set1.begin(), set1.end());
    mergedSet.insert(set2.begin(), set2.end());
    return mergedSet;
}

template<typename T, typename U, typename Hash>
std::unordered_map<T, U, Hash> TieMaps(const std::unordered_map<T, U, Hash>& map1, const std::unordered_map<T, U, Hash>& map2) {
    std::unordered_map<T, U, Hash> mergedMap;
    mergedMap.insert(map1.begin(), map1.end());
    mergedMap.insert(map2.begin(), map2.end());
    return mergedMap;
}

std::string RegexToPolishForm(const std::string &regex) {
    std::stack<char> operationStack;
    std::string output;

    for (size_t i = 0; i < regex.size(); ++i) {
        const char character = regex[i];
        if (('0' <= character and character <= '9')
            or ('a' <= character and character <= 'z')
            or ('A' <= character and character <= 'Z')) {
            output.push_back(character);
            } else {
                if (character == '(') {
                    operationStack.push(character);
                } else {
                    if (character == ')') {
                        while (!operationStack.empty() && operationStack.top() != '(') {
                            output.push_back(operationStack.top());
                            operationStack.pop();
                        }
                        if (!operationStack.empty()) {
                            operationStack.pop();
                        }

                        if (i + 1 < regex.size() && regex[i + 1] == '*') {
                            output.push_back('*');
                            ++i;
                        }
                    } else {
                        while (!operationStack.empty() && priority(operationStack.top()) >= priority(character)) {
                            output.push_back(operationStack.top());
                            operationStack.pop();
                               }
                        operationStack.push(character);
                    }
                }
            }
    }

    while (!operationStack.empty()) {
        output.push_back(operationStack.top());
        operationStack.pop();
    }

    return output;
}

Automaton::Automaton(state initialState, state finalState, char character)
    : m_states{initialState, finalState}
      , m_alphabet{character}
      , m_initialState{initialState}
      , m_finalState{finalState}
      , m_deltaFunction{{{initialState, character}, {finalState}}} {
    /*EMPTY*/
}

Automaton::Automaton(const state initialState, const state finalState)
    : m_states{initialState, finalState}
      , m_initialState{initialState}
      , m_finalState{finalState} {
    /*EMPTY*/
}

Automaton::Automaton(const Automaton &auto1, const Automaton &auto2) {
    state toReplace = auto1.m_finalState;
    state replacement = auto2.m_initialState;
    m_initialState = auto1.m_initialState;
    m_finalState = auto2.m_finalState;
    m_deltaFunction = auto1.m_deltaFunction;
    for (auto it: auto2.m_deltaFunction) {
        m_deltaFunction.insert(it);
    }
    m_alphabet = auto1.m_alphabet;
    for (auto it: auto2.m_alphabet) {
        m_alphabet.insert(it);
    }
    m_states = auto1.m_states;
    for (auto it: auto2.m_states) {
        m_states.insert(it);
    }
    m_states.erase(toReplace);
    for (auto &it: m_deltaFunction) {
        if (it.second.contains(toReplace)) {
            it.second.erase(toReplace);
            it.second.insert(replacement);
        }
    }
}

void Automaton::TieAutomatons(const Automaton &auto1, const Automaton &auto2) {
    m_alphabet = std::move(TieSets<char>(auto1.m_alphabet, auto2.m_alphabet));
    m_states = std::move(TieSets<state>(auto1.m_states, auto2.m_states));
    m_deltaFunction = std::move(TieMaps(auto1.m_deltaFunction, auto2.m_deltaFunction));
    m_deltaFunction[{m_initialState, lambda}].insert(auto1.m_initialState);
    m_deltaFunction[{m_initialState, lambda}].insert(auto2.m_initialState);
    m_deltaFunction[{auto1.m_finalState, lambda}].insert(m_finalState);
    m_deltaFunction[{auto2.m_finalState, lambda}].insert(m_finalState);
}

void Automaton::Kleene(const Automaton &automat) {
    m_alphabet = automat.m_alphabet;
    m_states = TieSets<state>(m_states, automat.m_states);
    m_deltaFunction = automat.m_deltaFunction;
    m_deltaFunction[{m_initialState, lambda}].insert(m_finalState);
    m_deltaFunction[{m_initialState, lambda}].insert(automat.m_initialState);
    m_deltaFunction[{automat.m_finalState, lambda}].insert(automat.m_initialState);
    m_deltaFunction[{automat.m_finalState, lambda}].insert(m_finalState);
}


state Automaton::GetStartState() const {
    return m_initialState;
}

state Automaton::GetFinalState() const {
    return m_finalState;
}

const std::unordered_map<transition, std::unordered_set<state>, Hash> &Automaton::GetDeltaFunction() const {
    return m_deltaFunction;
}

const std::unordered_set<char> &Automaton::GetAlphabet() const {
    return m_alphabet;
}

const std::unordered_set<state>& Automaton::GetStates() const {
    return m_states;
}


std::ostream &automaton::operator <<(std::ostream &os, const Automaton &automaton) {
    os << sigma << ": ";
    for (char c: automaton.GetAlphabet())
        os << c << " ";
    os << std::endl;
    os << "Q: ";
    for (state s: automaton.GetStates())
        os << "q" << s << " ";
    os << std::endl;
    os << "initial state: q" << automaton.GetStartState() << std::endl;
    os << "final state: q" << automaton.GetFinalState() << std::endl;
    os << "deltaFunction:\n";
    auto visitor = [](const auto &arg) -> std::string {
        using type = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<type, char>) {
            return std::string(1, arg);
        }

        if constexpr (std::is_same_v<type, const char *>) {
            return std::string(arg);
        }

        return {};
    };
    for (const auto &fullTransition: automaton.GetDeltaFunction()) {
        //https://levelup.gitconnected.com/understanding-std-visit-in-c-a-type-safe-way-to-traverse-variant-objects-dbeff9b47003
        //https://medium.com/@weidagang/modern-c-std-variant-and-std-visit-3c16084db7dc
        for (const auto& state : fullTransition.second) {
            os << std::format("{}(q{}, {}) -> q{}, ",
                              delta,
                              fullTransition.first.first,
                              std::visit(visitor, fullTransition.first.second),
                              state) << std::endl;
        }
    }
    return os;
}

bool Automaton::VerifyAutomaton() const {
    if (m_alphabet.empty() || m_states.empty()) {
        return false;
    }

    if (!m_states.contains(m_initialState) || !m_states.contains(m_finalState)) {
        return false;
    }

    return true;
}
