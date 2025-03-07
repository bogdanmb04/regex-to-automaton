#include "DFA.h"
#include <iomanip>

using namespace automaton;

template<typename T>
std::set<T> UnorderedSetToSet(const std::unordered_set<T>& uSet) {
    std::set<T> result;
    result.insert(uSet.begin(), uSet.end());
    return result;
}

template<typename T>
std::unordered_set<T> SetToUnorderedSet(const std::set<T>& oSet) {
    std::unordered_set<T> result;
    result.insert(oSet.begin(), oSet.end());
    return result;
}

std::ostream& DeterministicFiniteAutomaton::PrintAutomaton(std::ostream& os)
{
    os << std::setfill('_')<< std::setw(7 + m_alphabet.size() * 4) << '\n';
    os << "|";
    os << std::setfill(' ') << std::setw(5);
    os << delta;
    os << "|";
    os << std::setw(m_alphabet.size() * 4);
    os << sigma;
    os << "|\n";
    os << "|";
    os << std::setfill('-') << std::setw(5);
    os << "|";
    os << std::setfill('-') << std::setw(m_alphabet.size() * 4 + 1);
    os << "|\n";
    os << "|";
    os << "Q" << std::setfill(' ') << std::setw(4);
    os << "|";
    for (char symbol : m_alphabet) {
        os << " " << symbol << " |";
    }
    os << "\n";
    os <<"|";
    os << std::setfill('-') << std::setw(5) << "|";
    os << std::setw(m_alphabet.size() * 4 + 1) << "|\n";
    for (state elem : m_states) {
        os << "|";
        os << std::setfill(' ') << std::setw(3);
        os << "q" << elem;
        os << "|";
        for (char symbol : m_alphabet) {
            if (m_deltaFunction.contains({elem, symbol})) {
                os << "q" << *m_deltaFunction[{elem, symbol}].begin() << " |";
            }
            else {
                os << vid << std::setw(3)<<"|";
            }
        }
        os << "\n";
    }
    os << std::setfill('-')<< std::setw(7 + m_alphabet.size() * 4) << '\n';
    return os;
}

bool DeterministicFiniteAutomaton::CheckWord(const std::string &word) {
    //begin going through the delta func
    state init = m_initialState;
    for (char symbol : word) {
        if (m_deltaFunction.contains({init, symbol})) {
            init = *m_deltaFunction[{init, symbol}].begin();
        }
        else {
            return false;
        }
    }
    if (init != m_finalState) {
        return false;
    }
    return true;
}

std::unordered_set<state> DeterministicFiniteAutomaton::LambdaEncloseState(state q) const
{
    std::unordered_set<state> possibleStates;
    std::unordered_set<state> visited;
    possibleStates.insert(q);
    visited.insert(q);
    std::queue<state> statesToCheck;
    statesToCheck.push(q);
    bool checked = false;

    if (!m_deltaFunction.contains({q, lambda}))
        return {};

    while (!statesToCheck.empty())
    {
        for (const auto& [input, output] : m_deltaFunction)
        {
            if (input.first == q && std::holds_alternative<const char*>(input.second) && !checked)
            {
                checked = true;
                for (auto elem : output) {
                    statesToCheck.push(elem);
                    possibleStates.insert(elem);
                }
            }
            else if (possibleStates.contains(input.first) && !visited.contains(input.first) && std::holds_alternative<const char*>(input.second))
            {
                visited.insert(input.first);
                for (auto elem : output) {
                    statesToCheck.push(elem);
                    possibleStates.insert(elem);
                }
            }
        }
        statesToCheck.pop();
    }

    //TESTING OUTPUT
    // std::cout << lambda << "-enclosing for state q" << q <<": ";
    // for (auto elem : possibleStates)
    //     std::cout << "q" << elem << " ";
    // std::cout << std::endl;
    return possibleStates;
}

std::unordered_set<state> DeterministicFiniteAutomaton::PrimeLambdaEnclose(const std::unordered_set<state>& primeState) const
{
    std::unordered_set<state> possibleStates;
    std::queue<state> statesToCheck;

    for (auto elem : primeState) {
        statesToCheck.push(elem);
    }

    while (!statesToCheck.empty()) {
        auto elem = statesToCheck.front();
        statesToCheck.pop();
        auto stateSet = LambdaEncloseState(elem);
        if (stateSet.size() == 0 && primeState.size() > 1)
            possibleStates.insert(elem);
        possibleStates.insert(stateSet.begin(), stateSet.end());
    }
    return possibleStates;
}

std::unordered_set<state> DeterministicFiniteAutomaton::PrimeDeltaFunction(const std::unordered_set<state>& toCheck, char symbol) {
    std::unordered_set<state> possibleStates;
    for (auto elem : toCheck)
    {
        if (m_deltaFunction.contains({elem, symbol}))
        {
            possibleStates.insert(m_deltaFunction[{elem, symbol}].begin(), m_deltaFunction[{elem, symbol}].end());
        }
    }
    return possibleStates;
}

void DeterministicFiniteAutomaton::GeneratePrimeTransitions()
{
    for (const auto& set : m_primeStates) {
        for (char symbol : m_alphabet) {
            std::set<state> possibleStates;
            for (auto elem : set) {
                std::unordered_set<state> possibleState;
                try {
                    possibleState = m_deltaFunction.at({elem, symbol});
                } catch (std::out_of_range& e) {
                    possibleState = {};
                }
                if (!possibleState.empty()) {
                    possibleStates.insert(possibleState.begin(), possibleState.end());
                }
            }
            if (!possibleStates.empty())
                m_primeTransitions[{set, symbol}] = possibleStates;
        }
    }
} //where can each set go with the respective symbol?

void DeterministicFiniteAutomaton::OverrideAutomaton() {
    m_deltaFunction.clear();
    m_states.clear();
    bool changeInit = true;
    bool changeFin = true;
    for (const auto& [primeTrans, result]: m_primeTransitions) {
        if (m_primeTransitions.contains(primeTrans))
            m_deltaFunction[{m_primeStatesMapping[primeTrans.first], primeTrans.second}] = {m_primeStatesMapping[result]};
        if (primeTrans.first.contains(m_initialState) && changeInit) {
            m_initialState = m_primeStatesMapping[primeTrans.first];
            changeInit = false;
        }
        if (primeTrans.first.contains(m_finalState) && changeFin) {
            m_finalState = m_primeStatesMapping[primeTrans.first];
            changeFin = false;
        }
    }
    for (const auto& [input, output] : m_deltaFunction) {
        m_states.insert(input.first);
        m_states.insert(output.begin(), output.end());
    }
}


DeterministicFiniteAutomaton::DeterministicFiniteAutomaton(const automaton::Automaton& automat)
    : Automaton{automat}
{
    bool hasLambdaTransition = false;
    for (const auto& elem : automat.GetDeltaFunction()) {
        if (std::holds_alternative<const char*>(elem.first.second)) {
            hasLambdaTransition = true;
        }
    }

    if (!hasLambdaTransition) {
        return;
    }
    //no lambda transitions means the automaton only has concatenation - already simplified by our standards! ty Cristi :3

    std::queue<std::unordered_set<state> > toCheck;
    toCheck.push({m_initialState});

    while (!toCheck.empty()) {
        auto stateSet = toCheck.front();
        toCheck.pop();
        auto result = PrimeLambdaEnclose(stateSet);
        if (stateSet.size() == 1 && result.size() == 0) //sets with no lambda transitions = state with only alphabet transitions, insert the states of the delta function result into m_primeStates
        {
            auto singleState = *stateSet.begin();
            std::unordered_set<state> possibleStates;
            for (char symbol : m_alphabet) {
                if (m_deltaFunction.contains({singleState, symbol})) {
                    possibleStates.insert(m_deltaFunction[{singleState, symbol}].begin(), m_deltaFunction[{singleState, symbol}].end());
                    toCheck.push(possibleStates);
                    m_primeStates.insert(UnorderedSetToSet<state>(stateSet));
                }
                else if (singleState == m_finalState) {
                    m_primeStates.insert({singleState});
                }
            }
        }
        else if (!m_primeStates.contains(UnorderedSetToSet<state>(result)))
        {
            m_primeStates.insert(UnorderedSetToSet<state>(result));
            for (char symbol : m_alphabet) {
                std::unordered_set<state> transitionResult;
                for (auto elem : result) {
                    if (m_deltaFunction.contains({elem, symbol})) {
                        transitionResult.insert(m_deltaFunction[{elem, symbol}].begin(), m_deltaFunction[{elem, symbol}].end());
                    }
                }
                toCheck.push(transitionResult);
            }
        }
    }
    std::cout << "---------------------------------------------------\n";

    for (const auto& elem : m_primeStates) {
        for (auto val : elem) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "---------------------------------------------------\n";

    int counter = 0;
    for (const auto& elem : m_primeStates) {
        m_primeStatesMapping.insert({elem, counter});
        counter += 1;
    }

    for (const auto& [x, y] : m_primeStatesMapping) {
        for (auto elem : x) {
            std::cout << elem << " ";
        }
        std::cout << ": " << y << std::endl;
    }

    std::cout << "---------------------------------------------------\n";

    GeneratePrimeTransitions();

    for (const auto& [pTransition, result] : m_primeTransitions) {
        for (const auto& elem : pTransition.first) {
            std::cout << elem << " ";
        }
        std::cout << "with " << pTransition.second << ": ";
        for (const auto& elem: result) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    for (auto& [pTransition, result] : m_primeTransitions) {
        if (!m_primeStates.contains(result)) {
            result = UnorderedSetToSet(PrimeLambdaEnclose(SetToUnorderedSet(result)));
        }
    }

    std::cout << "---------------------------------------------------\n";

    for (const auto& [pTransition, result] : m_primeTransitions) {
        for (const auto& elem : pTransition.first) {
            std::cout << elem << " ";
        }
        std::cout << "with " << pTransition.second << ": ";
        for (const auto& elem: result) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "---------------------------------------------------\n";

    OverrideAutomaton();
}

std::ostream& automaton::operator << (std::ostream& os, const DeterministicFiniteAutomaton& automaton) {
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
            if (!fullTransition.second.empty())
                os << std::format("{}(q{}, {}) -> q{}, ",
                              delta,
                              fullTransition.first.first,
                              std::visit(visitor, fullTransition.first.second),
                              state) << std::endl;
            else
                os << std::format("{}(q{}, {}) -> q{}, ",
                              delta,
                              fullTransition.first.first,
                              std::visit(visitor, fullTransition.first.second),
                              vid) << std::endl;

        }
    }
    return os;
}
// template<typename Os, typename T>
// concept Printable = requires(Os& os, const T& t)
// {
//     {os << t};
// };
//
// template<Printable T>
// void PrintToConsole(const T& t) {
//     std::cout << t;
// }