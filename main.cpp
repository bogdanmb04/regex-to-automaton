#include <iostream>
#include "Automaton.h"
#include "DFA.h"
#include <fstream>
#include <regex>
#include <filesystem>

automaton::Automaton* BuildAutomaton(const std::string& regex)
{
    using namespace automaton;
    std::string polishFormRegex = RegexToPolishForm(regex);
    std::stack<Automaton*> automatonStack;
    std::uint16_t counter = 0;

    //exciting stuff here!!

    for(char character : polishFormRegex)
    {
        if(('a' <= character && character <= 'z')
            || ('A' <= character && character <= 'Z')
            || ('0' <= character && character <= '9'))
        {
            std::uint16_t next = counter + 1;
            auto* automat = new Automaton{counter, next, character};
            automatonStack.push(automat);
        }

        if(character == '*')
        {
            auto A = automatonStack.top();
            automatonStack.pop();
            auto* C = new Automaton{counter, static_cast<state>(counter + 1)};
            C->Kleene(*A);
            delete A;
            automatonStack.push(C);
        }

        if(character == '.')
        {
            auto B = automatonStack.top();
            automatonStack.pop();
            auto A = automatonStack.top();
            automatonStack.pop();

            auto* C = new Automaton{*A, *B};
            delete A;
            delete B;
            automatonStack.push(C);
        }

        if(character == '|')
        {
            auto B = automatonStack.top();
            automatonStack.pop();
            auto A = automatonStack.top();
            automatonStack.pop();

            auto* C = new Automaton{counter, static_cast<state>(counter + 1)};
            C->TieAutomatons(*A, *B);
            delete A;
            delete B;
            automatonStack.push(C);
        }
        counter += 2;
    }

    auto* finalAutomaton = automatonStack.top();
    automatonStack.pop();
    return finalAutomaton;
}

std::string ParsingRegex(const std::string& regex) {
    std::string result = "";
    for (auto character : regex) {
        if (character != '.') {
            result += character;
        }
    }
    return result;
}

bool ValidateRegex(const std::string& regex) {
    std::string toCheck = ParsingRegex(regex);
    try {
        std::regex re(toCheck);
        return true;
    }
    catch (std::regex_error& e) {
        return false;
    }
}

int main()
{
    // std::string myRegex = "a.b.a.(a.a|b.b)*.c.(a.b)*";
    // std::string myRegex = "(a.a|b)*.b.b";
    std::ifstream fin("../input.txt");
    if (!fin.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    if (!fout.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    std::string myRegex;
    fin >> myRegex;
    if (!ValidateRegex(myRegex))
    {
        std::cerr << "Input a valid regex :)";
        return 1;
    }
    auto* myAutomaton = BuildAutomaton(myRegex);
    std::cout << *myAutomaton;
    automaton::DeterministicFiniteAutomaton myDFA(*myAutomaton);
    bool in = true;
    while(in)
    {
        system("clear");
        std::cout << "Select one of the following options:" << std::endl;
        std::cout << "1) Display the input regex\n";
        std::cout << "2) Display the automata in terminal and output file\n";
        std::cout << "3) Check if a word is accepted by the automata\n";
        std::cout << "4) Exit\n";

        int option;
        std::cin >> option;

        switch (option) {
            case 1: {
                std::cout << ParsingRegex(myRegex) << std::endl;
                break;
            }
            case 2: {
                myDFA.PrintAutomaton(std::cout);
                break;
            }
            case 3: {
                std::cout << "Input your word: ";
                std::string word;
                std::cin >> word;
                if (myDFA.CheckWord(word))
                    std::cout << "Valid word\n";
                else std::cout << "Invalid word\n";
                break;
            }
            default: {
                in = false;
            }
        }

    }
}
