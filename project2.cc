/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include "lexer.h"

using namespace std;

// Rule structure to represent a grammar rule
struct Rule {
    string lhs;
    vector<string> rhs;

    // Equality operator for rules
    bool operator==(const Rule& other) const {
        if (lhs != other.lhs) return false;
        if (rhs.size() != other.rhs.size()) return false;
        for (size_t i = 0; i < rhs.size(); i++) {
            if (rhs[i] != other.rhs[i]) return false;
        }
        return true;
    }
};

// Global variables
vector<Rule> grammar;
vector<string> terminals;
vector<string> non_terminals;
set<string> terminalSet;
set<string> nonTerminalSet;

// read grammar
void ReadGrammar() {
    LexicalAnalyzer lexer;
    Token token;
    
    // Process rules until we reach the end of the grammar (HASH)
    while (true) {
        // Get the left-hand side (a non-terminal)
        token = lexer.GetToken();
        if (token.token_type == HASH) {
            break;
        }
        
        if (token.token_type != ID) {
            cout << "SYNTAX ERROR !!!!!!!!!!!!!!!" << endl;
            exit(1);
        }
        
        string lhs = token.lexeme;
        
        // Add to non-terminals if not already there
        if (nonTerminalSet.find(lhs) == nonTerminalSet.end()) {
            nonTerminalSet.insert(lhs);
            non_terminals.push_back(lhs);
        }
        
        // Check for ARROW token
        token = lexer.GetToken();
        if (token.token_type != ARROW) {
            cout << "SYNTAX ERROR !!!!!!!!!!!!!!!" << endl;
            exit(1);
        }
        
        // Process right-hand sides (possibly multiple alternatives)
        while (true) {
            vector<string> rhs;
            
            // Parse symbols until OR or STAR
            token = lexer.GetToken();
            while (token.token_type == ID) {
                string symbol = token.lexeme;
                rhs.push_back(symbol);
                
                // Track symbols (potentially terminals)
                if (terminalSet.find(symbol) == terminalSet.end() && 
                    nonTerminalSet.find(symbol) == nonTerminalSet.end()) {
                    terminalSet.insert(symbol);
                    terminals.push_back(symbol);
                }
                
                token = lexer.GetToken();
            }
            
            // Create a rule with this alternative
            Rule rule;
            rule.lhs = lhs;
            rule.rhs = rhs;
            grammar.push_back(rule);
            
            // Check if we're at the end of the rule or have another alternative
            if (token.token_type == STAR) {
                break;
            } else if (token.token_type == OR) {
                continue;
            } else {
                cout << "SYNTAX ERROR !!!!!!!!!!!!!!!" << endl;
                exit(1);
            }
        }
    }
    
    // Remove non-terminals from the terminals list
    auto it = terminals.begin();
    while (it != terminals.end()) {
        if (nonTerminalSet.find(*it) != nonTerminalSet.end()) {
            terminalSet.erase(*it);
            it = terminals.erase(it);
        } else {
            ++it;
        }
    }
}

/* 
 * Task 1: 
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
*/
void Task1() {
    vector<string> ordered_symbols;
    set<string> added_symbols;

    // Iterate through the grammar to maintain the order of appearance
    for (const Rule& rule : grammar) {
        if (added_symbols.find(rule.lhs) == added_symbols.end()) {
            ordered_symbols.push_back(rule.lhs);
            added_symbols.insert(rule.lhs);
        }
        for (const string& symbol : rule.rhs) {
            if (added_symbols.find(symbol) == added_symbols.end()) {
                ordered_symbols.push_back(symbol);
                added_symbols.insert(symbol);
            }
        }
    }

    // Print terminals
    for (const string& symbol : ordered_symbols) {
        if (terminalSet.find(symbol) != terminalSet.end()) {
            cout << symbol << " ";
        }
    }
    cout << endl;

    // Print non-terminals
    for (const string& symbol : ordered_symbols) {
        if (nonTerminalSet.find(symbol) != nonTerminalSet.end()) {
            cout << symbol << " ";
        }
    }
    cout << endl;
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
*/
void Task2() {
    // Calculate nullable non-terminals
    set<string> nullable;
    
    // Initialization: Add all non-terminals with epsilon rules
    for (const Rule& rule : grammar) {
        if (rule.rhs.empty()) {
            nullable.insert(rule.lhs);
        }
    }
    
    // Iteratively find more nullable non-terminals
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const Rule& rule : grammar) {
            // Skip if already nullable
            if (nullable.find(rule.lhs) != nullable.end()) {
                continue;
            }
            
            // Check if all symbols on the RHS are nullable
            bool all_nullable = true;
            for (const string& symbol : rule.rhs) {
                if (nullable.find(symbol) == nullable.end()) {
                    all_nullable = false;
                    break;
                }
            }
            
            // If all symbols are nullable, the LHS is nullable
            if (all_nullable && !rule.rhs.empty()) {
                nullable.insert(rule.lhs);
                changed = true;
            }
        }
    }
    
    // Print in order of appearance in the grammar
    cout << "Nullable = { ";
    bool first = true;
    for (const string& non_terminal : non_terminals) {
        if (nullable.find(non_terminal) != nullable.end()) {
            if (!first) {
                cout << ", ";
            }
            cout << non_terminal;
            first = false;
        }
    }
    cout << " }" << endl;
}

// Task 3: FIRST sets
void Task3() {
    unordered_map<string, set<string>> FIRST;

    // Initialize FIRST sets for terminals
    for (const string& terminal : terminals) {
        FIRST[terminal].insert(terminal);
    }

    // Initialize FIRST sets for non-terminals
    for (const string& non_terminal : non_terminals) {
        FIRST[non_terminal] = set<string>();
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule& rule : grammar) {
            const string& lhs = rule.lhs;
            const vector<string>& rhs = rule.rhs;

            if (rhs.empty()) {
                if (FIRST[lhs].insert("").second) {
                    changed = true;
                }
            } else {
                size_t i;
                for (i = 0; i < rhs.size(); ++i) {
                    const string& symbol = rhs[i];
                    bool containsEpsilon = false;

                    for (const string& first : FIRST[symbol]) {
                        if (first != "") {
                            if (FIRST[lhs].insert(first).second) {
                                changed = true;
                            }
                        } else {
                            containsEpsilon = true;
                        }
                    }

                    if (!containsEpsilon) break;
                }

                if (i == rhs.size()) {
                    if (FIRST[lhs].insert("").second) {
                        changed = true;
                    }
                }
            }
        }
    }

    // Print FIRST sets
    for (const string& non_terminal : non_terminals) {
        cout << "FIRST(" << non_terminal << ") = { ";
        bool first = true;
        for (const string& terminal : terminals) {
            if (FIRST[non_terminal].find(terminal) != FIRST[non_terminal].end()) {
                if (!first) cout << ", ";
                cout << terminal;
                first = false;
            }
        }
        if (FIRST[non_terminal].find("") != FIRST[non_terminal].end()) {
            if (!first) cout << ", ";
            cout << "";
        }
        cout << " }" << endl;
    }
}

// Task 4: FOLLOW sets
void Task4() {
    unordered_map<string, set<string>> FOLLOW;

    // Initialize FOLLOW sets
    for (const string& non_terminal : non_terminals) {
        FOLLOW[non_terminal] = set<string>();
    }

    // Add $ to FOLLOW(S) where S is the start symbol
    FOLLOW[non_terminals[0]].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule& rule : grammar) {
            const string& lhs = rule.lhs;
            const vector<string>& rhs = rule.rhs;

            for (size_t i = 0; i < rhs.size(); ++i) {
                if (nonTerminalSet.find(rhs[i]) != nonTerminalSet.end()) {
                    set<string> first_beta;
                    bool all_epsilon = true;

                    for (size_t j = i + 1; j < rhs.size(); ++j) {
                        for (const string& first : FIRST[rhs[j]]) {
                            if (first != "") {
                                first_beta.insert(first);
                                all_epsilon = false;
                            }
                        }
                        if (!all_epsilon) break;
                    }

                    for (const string& terminal : first_beta) {
                        if (FOLLOW[rhs[i]].insert(terminal).second) {
                            changed = true;
                        }
                    }

                    if (all_epsilon || i == rhs.size() - 1) {
                        for (const string& follow : FOLLOW[lhs]) {
                            if (FOLLOW[rhs[i]].insert(follow).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // Print FOLLOW sets
    for (const string& non_terminal : non_terminals) {
        cout << "FOLLOW(" << non_terminal << ") = { ";
        bool first = true;
        if (FOLLOW[non_terminal].find("$") != FOLLOW[non_terminal].end()) {
            cout << "$";
            first = false;
        }
        for (const string& terminal : terminals) {
            if (FOLLOW[non_terminal].find(terminal) != FOLLOW[non_terminal].end()) {
                if (!first) cout << ", ";
                cout << terminal;
                first = false;
            }
        }
        cout << " }" << endl;
    }
}


// Task 5: left factoring
void Task5() {
    // Start with the initial grammar
    vector<Rule> result = grammar;
    
    // Keep track of counter for new non-terminals
    unordered_map<string, int> counters;
    for (const string& nt : non_terminals) {
        counters[nt] = 1;
    }
    
    // Process until no more left factoring can be done
    bool changed = true;
    while (changed) {
        changed = false;
        
        // Group rules by their LHS
        unordered_map<string, vector<Rule>> rulesByLHS;
        for (const Rule& rule : result) {
            rulesByLHS[rule.lhs].push_back(rule);
        }
        
        // For each non-terminal, check if left factoring is needed
        for (const auto& pair : rulesByLHS) {
            const string& lhs = pair.first;
            const vector<Rule>& rules = pair.second;
            
            // Need at least 2 rules to consider left factoring
            if (rules.size() < 2) {
                continue;
            }
            
            // Find the longest common prefix among any two rules
            size_t max_prefix_length = 0;
            vector<Rule> rules_with_prefix;
            vector<string> max_prefix;
            
            for (size_t i = 0; i < rules.size(); i++) {
                for (size_t j = i + 1; j < rules.size(); j++) {
                    size_t prefix_length = commonPrefixLength(rules[i].rhs, rules[j].rhs);
                    if (prefix_length > 0 && (prefix_length > max_prefix_length || 
                        (prefix_length == max_prefix_length && 
                         lexicographical_compare(rules[i].rhs.begin(), rules[i].rhs.begin() + prefix_length,
                                               max_prefix.begin(), max_prefix.end())))) {
                        max_prefix_length = prefix_length;
                        max_prefix.clear();
                        for (size_t k = 0; k < prefix_length; k++) {
                            max_prefix.push_back(rules[i].rhs[k]);
                        }
                        
                        rules_with_prefix.clear();
                        // Collect all rules with this prefix
                        for (const Rule& rule : rules) {
                            if (rule.rhs.size() >= prefix_length) {
                                bool matches = true;
                                for (size_t k = 0; k < prefix_length; k++) {
                                    if (rule.rhs[k] != rules[i].rhs[k]) {
                                        matches = false;
                                        break;
                                    }
                                }
                                if (matches) {
                                    rules_with_prefix.push_back(rule);
                                }
                            }
                        }
                    }
                }
            }
            
            // If we found a common prefix, left factor it
            if (max_prefix_length > 0) {
                changed = true;
                
                // Create a new non-terminal
                string new_nt = lhs + to_string(counters[lhs]);
                counters[lhs]++;
                
                // Create the left-factored rule
                Rule factored_rule;
                factored_rule.lhs = lhs;
                for (size_t i = 0; i < max_prefix_length; i++) {
                    factored_rule.rhs.push_back(rules_with_prefix[0].rhs[i]);
                }
                factored_rule.rhs.push_back(new_nt);
                
                // Remove the old rules with the common prefix and add the new one
                vector<Rule> new_result;
                for (const Rule& rule : result) {
                    bool found = false;
                    for (const Rule& r : rules_with_prefix) {
                        if (rule == r) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        new_result.push_back(rule);
                    }
                }
                new_result.push_back(factored_rule);
                
                // Add rules for the new non-terminal
                for (const Rule& rule : rules_with_prefix) {
                    Rule new_rule;
                    new_rule.lhs = new_nt;
                    for (size_t i = max_prefix_length; i < rule.rhs.size(); i++) {
                        new_rule.rhs.push_back(rule.rhs[i]);
                    }
                    new_result.push_back(new_rule);
                }
                
                result = new_result;
                break;  // Start over with the new grammar
            }
        }
    }
    
    // Sort the resulting grammar lexicographically
    sort(result.begin(), result.end(), compareLexicographically);
    
    // Print the result
    for (const Rule& rule : result) {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty()) {
            cout << "";
        } else {
            for (size_t i = 0; i < rule.rhs.size(); i++) {
                cout << rule.rhs[i] << " ";
            }
        }
        cout << "#" << endl;
    }
}

// Task 6: eliminate left recursion
void Task6() {
    // Start with the original grammar
    vector<Rule> result = grammar;
    
    // Sort non-terminals lexicographically
    vector<string> sorted_nt = non_terminals;
    sort(sorted_nt.begin(), sorted_nt.end());
    
    // Keep track of counter for new non-terminals
    unordered_map<string, int> counters;
    for (const string& nt : non_terminals) {
        counters[nt] = 1;
    }
    
    // For each non-terminal in the sorted order
    for (size_t i = 0; i < sorted_nt.size(); i++) {
        const string& A_i = sorted_nt[i];
        
        // Group rules by their LHS
        unordered_map<string, vector<Rule>> rulesByLHS;
        for (const Rule& rule : result) {
            rulesByLHS[rule.lhs].push_back(rule);
        }
        
        // For each non-terminal that precedes A_i
        for (size_t j = 0; j < i; j++) {
            const string& A_j = sorted_nt[j];
            
            // For each rule with A_i on the left-hand side
            vector<Rule> new_A_i_rules;
            for (const Rule& rule : rulesByLHS[A_i]) {
                // If the first symbol of the rule is A_j, replace it
                if (!rule.rhs.empty() && rule.rhs[0] == A_j) {
                    // For each rule with A_j on the left-hand side
                    for (const Rule& rule_j : rulesByLHS[A_j]) {
                        // Create a new rule by replacing A_j with its RHS
                        Rule new_rule;
                        new_rule.lhs = A_i;
                        
                        // Add RHS of rule_j
                        for (const string& symbol : rule_j.rhs) {
                            new_rule.rhs.push_back(symbol);
                        }
                        
                        // Add the rest of the original rule
                        for (size_t k = 1; k < rule.rhs.size(); k++) {
                            new_rule.rhs.push_back(rule.rhs[k]);
                        }
                        
                        new_A_i_rules.push_back(new_rule);
                    }
                } else {
                    // Keep the rule as is
                    new_A_i_rules.push_back(rule);
                }
            }
            
            // Replace the old rules with the new ones
            vector<Rule> temp_result;
            for (const Rule& rule : result) {
                if (rule.lhs != A_i) {
                    temp_result.push_back(rule);
                }
            }
            for (const Rule& rule : new_A_i_rules) {
                temp_result.push_back(rule);
            }
            result = temp_result;
            
            // Update rulesByLHS
            rulesByLHS.clear();
            for (const Rule& rule : result) {
                rulesByLHS[rule.lhs].push_back(rule);
            }
        }
        
        // Now eliminate direct left recursion from A_i
        vector<Rule> A_i_alpha;  // Rules A_i -> A_i alpha
        vector<Rule> A_i_beta;   // Rules A_i -> beta
        
        for (const Rule& rule : rulesByLHS[A_i]) {
            if (!rule.rhs.empty() && rule.rhs[0] == A_i) {
                A_i_alpha.push_back(rule);
            } else {
                A_i_beta.push_back(rule);
            }
        }
        
        // If there is direct left recursion
        if (!A_i_alpha.empty()) {
            // Create a new non-terminal A_i1
            string A_i1 = A_i + to_string(counters[A_i]);
            counters[A_i]++;
            
            // Replace the rules
            vector<Rule> temp_result;
            for (const Rule& rule : result) {
                if (rule.lhs != A_i) {
                    temp_result.push_back(rule);
                }
            }
            
            // Add A_i -> beta A_i1
            for (const Rule& rule : A_i_beta) {
                Rule new_rule;
                new_rule.lhs = A_i;
                for (const string& symbol : rule.rhs) {
                    new_rule.rhs.push_back(symbol);
                }
                new_rule.rhs.push_back(A_i1);
                temp_result.push_back(new_rule);
            }
            
            // Add A_i1 -> alpha A_i1
            for (const Rule& rule : A_i_alpha) {
                Rule new_rule;
                new_rule.lhs = A_i1;
                for (size_t j = 1; j < rule.rhs.size(); j++) {
                    new_rule.rhs.push_back(rule.rhs[j]);
                }
                new_rule.rhs.push_back(A_i1);
                temp_result.push_back(new_rule);
            }
            
            // Add A_i1 -> epsilon
            Rule epsilon_rule;
            epsilon_rule.lhs = A_i1;
            // Leave RHS empty for epsilon
            temp_result.push_back(epsilon_rule);
            
            result = temp_result;
        }
    }
    
    // Sort the resulting grammar lexicographically
    sort(result.begin(), result.end(), compareLexicographically);
    
    // Print the result
    for (const Rule& rule : result) {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty()) {
            cout << "";
        } else {
            for (size_t i = 0; i < rule.rhs.size(); i++) {
                cout << rule.rhs[i] << " ";
            }
        }
        cout << "#" << endl;
    }
}
    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: Task1();
            break;

        case 2: Task2();
            break;

        case 3: Task3();
            break;

        case 4: Task4();
            break;

        case 5: Task5();
            break;
        
        case 6: Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}
