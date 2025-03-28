/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 * Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "lexer.h"

using namespace std;

// Rule structure to represent a grammar rule
struct Rule {
    string lhs;
    vector<string> rhs;

    // For lexicographic comparison in Tasks 5 and 6
    bool operator<(const Rule& other) const {
        if (lhs != other.lhs) return lhs < other.lhs;
        
        // Compare RHS lexicographically
        size_t i = 0;
        while (i < rhs.size() && i < other.rhs.size()) {
            if (rhs[i] != other.rhs[i]) {
                return rhs[i] < other.rhs[i];
            }
            i++;
        }
        
        // If one is a prefix of the other, shorter one comes first
        return rhs.size() < other.rhs.size();
    }
};

// Global variables
vector<Rule> grammar;
vector<string> terminals;
vector<string> non_terminals;
unordered_set<string> terminalSet;
unordered_set<string> nonTerminalSet;

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
    vector<string> realTerminals;
    for (const string& terminal : terminals) {
        if (nonTerminalSet.find(terminal) == nonTerminalSet.end()) {
            realTerminals.push_back(terminal);
        }
    }
    terminals = realTerminals;
}

/*
 * Task 1:
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
 */
void Task1() {
    // Track terminals in order of appearance, without duplicates
    vector<string> ordered_terminals;
    unordered_set<string> seen_terminals;
   
    // Track non-terminals in order of appearance (already done in ReadGrammar)
   
    // First pass to collect terminals in order of appearance
    for (const Rule& rule : grammar) {
        // Check RHS symbols
        for (const string& symbol : rule.rhs) {
            // If it's not a non-terminal and not seen before
            if (nonTerminalSet.find(symbol) == nonTerminalSet.end() &&
                seen_terminals.find(symbol) == seen_terminals.end()) {
                ordered_terminals.push_back(symbol);
                seen_terminals.insert(symbol);
            }
        }
    }
   
    // Print terminals followed by non-terminals
    bool first = true;
   
    // Print terminals
    for (const string& terminal : ordered_terminals) {
        if (!first) {
            cout << " ";
        }
        cout << terminal;
        first = false;
    }
   
    // Print non-terminals
    for (const string& non_terminal : non_terminals) {
        if (!first) {
            cout << " ";
        }
        cout << non_terminal;
        first = false;
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
    cout << "Nullable = {";
    bool first = true;
    for (const string& non_terminal : non_terminals) {
        if (nullable.find(non_terminal) != nullable.end()) {
            if (first) {
                cout << " ";
                first = false;
            } else {
                cout << ", ";
            }
            cout << non_terminal;
        }
    }
    cout << " }" << endl;
}

/*
 * Task 3:
 * Compute and print FIRST sets for all non-terminals
 */
void Task3() {
    // Calculate nullable set (same as Task 2)
    set<string> nullable;
    for (const Rule& rule : grammar) {
        if (rule.rhs.empty()) {
            nullable.insert(rule.lhs);
        }
    }
    
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule& rule : grammar) {
            if (nullable.find(rule.lhs) != nullable.end()) continue;
            
            bool all_nullable = true;
            for (const string& symbol : rule.rhs) {
                if (nullable.find(symbol) == nullable.end()) {
                    all_nullable = false;
                    break;
                }
            }
            if (all_nullable && !rule.rhs.empty()) {
                nullable.insert(rule.lhs);
                changed = true;
            }
        }
    }

    // Initialize FIRST sets
    unordered_map<string, set<string>> FIRST;
    
    // Initialize FIRST sets for terminals
    for (const string& terminal : terminals) {
        FIRST[terminal].insert(terminal);
    }
    
    // Iteratively calculate FIRST sets
    changed = true;
    while (changed) {
        changed = false;
        for (const Rule& rule : grammar) {
            if (rule.rhs.empty()) continue;
            
            for (size_t i = 0; i < rule.rhs.size(); i++) {
                const string& symbol = rule.rhs[i];
                
                // Add FIRST(symbol) to FIRST(rule.lhs)
                for (const string& term : FIRST[symbol]) {
                    if (FIRST[rule.lhs].insert(term).second) {
                        changed = true;
                    }
                }
                
                // If symbol is not nullable, stop
                if (nullable.find(symbol) == nullable.end()) {
                    break;
                }
            }
        }
    }
    
    // Print FIRST sets in order
    for (const string& nt : non_terminals) {
        cout << "FIRST(" << nt << ") = {";
        
        // Collect terminals in order of appearance
        vector<string> ordered_terms;
        for (const string& terminal : terminals) {
            if (FIRST[nt].find(terminal) != FIRST[nt].end()) {
                ordered_terms.push_back(terminal);
            }
        }
        
        // Print with proper formatting
        if (!ordered_terms.empty()) {
            cout << " " << ordered_terms[0];
            for (size_t i = 1; i < ordered_terms.size(); i++) {
                cout << ", " << ordered_terms[i];
            }
        }
        cout << " }" << endl;
    }
}

/*
 * Task 4:
 * Compute and print FOLLOW sets for all non-terminals
 */
void Task4() {
    // Calculate nullable and FIRST sets (same as Task 3)
    // ... (same as Task 3 implementation)

    // Initialize FOLLOW sets
    unordered_map<string, set<string>> FOLLOW;
    
    // Add $ to FOLLOW of start symbol
    if (!non_terminals.empty()) {
        FOLLOW[non_terminals[0]].insert("$");
    }
    
    // Iteratively calculate FOLLOW sets
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const Rule& rule : grammar) {
            for (int i = rule.rhs.size() - 1; i >= 0; i--) {
                const string& symbol = rule.rhs[i];
                
                if (nonTerminalSet.find(symbol) == nonTerminalSet.end()) {
                    continue; // Skip terminals
                }
                
                // Case 1: Last symbol in rule
                if (i == rule.rhs.size() - 1) {
                    for (const string& term : FOLLOW[rule.lhs]) {
                        if (FOLLOW[symbol].insert(term).second) {
                            changed = true;
                        }
                    }
                }
                // Case 2: Not last symbol
                else {
                    bool all_nullable = true;
                    for (size_t j = i + 1; j < rule.rhs.size(); j++) {
                        const string& next = rule.rhs[j];
                        
                        // Add FIRST(next) to FOLLOW(symbol)
                        for (const string& term : FIRST[next]) {
                            if (FOLLOW[symbol].insert(term).second) {
                                changed = true;
                            }
                        }
                        
                        // If next is not nullable, stop
                        if (nullable.find(next) == nullable.end()) {
                            all_nullable = false;
                            break;
                        }
                    }
                    
                    // If all remaining symbols are nullable
                    if (all_nullable) {
                        for (const string& term : FOLLOW[rule.lhs]) {
                            if (FOLLOW[symbol].insert(term).second) {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Print FOLLOW sets
    for (const string& nt : non_terminals) {
        cout << "FOLLOW(" << nt << ") = {";
        
        // Check for $
        bool has_dollar = FOLLOW[nt].find("$") != FOLLOW[nt].end();
        
        // Collect other terminals in order
        vector<string> ordered_terms;
        for (const string& terminal : terminals) {
            if (terminal != "$" && FOLLOW[nt].find(terminal) != FOLLOW[nt].end()) {
                ordered_terms.push_back(terminal);
            }
        }
        
        // Print with proper formatting
        if (has_dollar) {
            cout << " $";
            if (!ordered_terms.empty()) {
                cout << ", ";
            }
        }
        
        for (size_t i = 0; i < ordered_terms.size(); i++) {
            if (i > 0 || has_dollar) {
                cout << ", ";
            }
            cout << ordered_terms[i];
        }
        
        cout << " }" << endl;
    }
}

/*
 * Task 5:
 * Left factoring the grammar
 */
void Task5() {
    vector<Rule> result = grammar;
    unordered_map<string, int> counters;
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        // Group rules by LHS
        unordered_map<string, vector<Rule>> rulesByLHS;
        for (const Rule& rule : result) {
            rulesByLHS[rule.lhs].push_back(rule);
        }
        
        // Process each non-terminal
        for (const auto& pair : rulesByLHS) {
            const string& lhs = pair.first;
            const vector<Rule>& rules = pair.second;
            
            // Find the longest common prefix among any rules
            size_t maxPrefixLen = 0;
            vector<Rule> rulesWithPrefix;
            vector<string> prefix;
            
            for (size_t i = 0; i < rules.size(); i++) {
                for (size_t j = i + 1; j < rules.size(); j++) {
                    // Find common prefix length between rules[i] and rules[j]
                    size_t prefixLen = 0;
                    while (prefixLen < rules[i].rhs.size() && 
                           prefixLen < rules[j].rhs.size() && 
                           rules[i].rhs[prefixLen] == rules[j].rhs[prefixLen]) {
                        prefixLen++;
                    }
                    
                    if (prefixLen > 0 && prefixLen > maxPrefixLen) {
                        maxPrefixLen = prefixLen;
                        prefix.clear();
                        for (size_t k = 0; k < prefixLen; k++) {
                            prefix.push_back(rules[i].rhs[k]);
                        }
                        
                        // Collect all rules with this prefix
                        rulesWithPrefix.clear();
                        for (const Rule& rule : rules) {
                            if (rule.rhs.size() >= prefixLen) {
                                bool matches = true;
                                for (size_t k = 0; k < prefixLen; k++) {
                                    if (rule.rhs[k] != prefix[k]) {
                                        matches = false;
                                        break;
                                    }
                                }
                                
                                if (matches) {
                                    rulesWithPrefix.push_back(rule);
                                }
                            }
                        }
                    }
                }
            }
            
            // If we found a common prefix, left factor it
            if (maxPrefixLen > 0) {
                changed = true;
                
                // Create a new non-terminal
                if (counters.find(lhs) == counters.end()) {
                    counters[lhs] = 1;
                }
                string newNT = lhs + to_string(counters[lhs]++);
                
                // Create the left-factored rule
                Rule factored;
                factored.lhs = lhs;
                for (size_t i = 0; i < maxPrefixLen; i++) {
                    factored.rhs.push_back(prefix[i]);
                }
                factored.rhs.push_back(newNT);
                
                // Create rules for the new non-terminal
                vector<Rule> newRules;
                for (const Rule& rule : rulesWithPrefix) {
                    Rule newRule;
                    newRule.lhs = newNT;
                    
                    // Add the suffix (or epsilon if no suffix)
                    if (rule.rhs.size() > maxPrefixLen) {
                        for (size_t i = maxPrefixLen; i < rule.rhs.size(); i++) {
                            newRule.rhs.push_back(rule.rhs[i]);
                        }
                    }
                    
                    newRules.push_back(newRule);
                }
                
                // Update the result
                vector<Rule> updatedResult;
                for (const Rule& rule : result) {
                    bool found = false;
                    for (const Rule& r : rulesWithPrefix) {
                        if (rule.lhs == r.lhs && rule.rhs == r.rhs) {
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        updatedResult.push_back(rule);
                    }
                }
                
                updatedResult.push_back(factored);
                for (const Rule& rule : newRules) {
                    updatedResult.push_back(rule);
                }
                
                result = updatedResult;
                break;
            }
        }
    }
    
    // Sort the resulting grammar lexicographically
    sort(result.begin(), result.end());
    
    // Print the left-factored grammar
    for (const Rule& rule : result) {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty()) {
            cout << "#" << endl;
        } else {
            for (size_t i = 0; i < rule.rhs.size(); i++) {
                cout << rule.rhs[i];
                if (i < rule.rhs.size() - 1) {
                    cout << " ";
                }
            }
            cout << " #" << endl;
        }
    }
}

/*
 * Task 6:
 * Eliminating left recursion
 */
void Task6() {
    vector<Rule> result = grammar;
    unordered_map<string, int> counters;
    
    // Sort non-terminals lexicographically
    vector<string> sortedNonTerminals = non_terminals;
    sort(sortedNonTerminals.begin(), sortedNonTerminals.end());
    
    // Process each non-terminal in order
    for (size_t i = 0; i < sortedNonTerminals.size(); i++) {
        const string& Ai = sortedNonTerminals[i];
        
        // First, eliminate indirect left recursion
        for (size_t j = 0; j < i; j++) {
            const string& Aj = sortedNonTerminals[j];
            
            // Find all rules Ai -> Aj γ
            vector<Rule> AiToAjRules;
            vector<Rule> otherAiRules;
            
            for (const Rule& rule : result) {
                if (rule.lhs == Ai) {
                    if (!rule.rhs.empty() && rule.rhs[0] == Aj) {
                        AiToAjRules.push_back(rule);
                    } else {
                        otherAiRules.push_back(rule);
                    }
                }
            }
            
            // If there are Ai -> Aj γ rules
            if (!AiToAjRules.empty()) {
                // Find all rules Aj -> δ
                vector<Rule> AjRules;
                for (const Rule& rule : result) {
                    if (rule.lhs == Aj) {
                        AjRules.push_back(rule);
                    }
                }
                
                // Replace Ai -> Aj γ with Ai -> δ γ for all δ in Aj's rules
                vector<Rule> newRules;
                for (const Rule& rule : result) {
                    if (rule.lhs != Ai) {
                        newRules.push_back(rule);
                    }
                }
                
                // Add the substituted rules
                for (const Rule& aiRule : AiToAjRules) {
                    for (const Rule& ajRule : AjRules) {
                        Rule newRule;
                        newRule.lhs = Ai;
                        
                        // Add δ
                        for (const string& symbol : ajRule.rhs) {
                            newRule.rhs.push_back(symbol);
                        }
                        
                        // Add γ (skip the first symbol which is Aj)
                        for (size_t k = 1; k < aiRule.rhs.size(); k++) {
                            newRule.rhs.push_back(aiRule.rhs[k]);
                        }
                        
                        newRules.push_back(newRule);
                    }
                }
                
                // Add the other Ai rules
                for (const Rule& rule : otherAiRules) {
                    newRules.push_back(rule);
                }
                
                result = newRules;
            }
        }
        
        // Now eliminate direct left recursion
        vector<Rule> alphaRules; // Ai -> Ai α
        vector<Rule> betaRules;  // Ai -> β
        
        for (const Rule& rule : result) {
            if (rule.lhs == Ai) {
                if (!rule.rhs.empty() && rule.rhs[0] == Ai) {
                    alphaRules.push_back(rule);
                } else {
                    betaRules.push_back(rule);
                }
            }
        }
        
        // If there's direct left recursion
        if (!alphaRules.empty()) {
            // Create a new non-terminal Ai'
            if (counters.find(Ai) == counters.end()) {
                counters[Ai] = 1;
            }
            string Ai_prime = Ai + to_string(counters[Ai]++);
            
            // Replace the rules
            vector<Rule> temp_result;
            for (const Rule& rule : result) {
                if (rule.lhs != Ai) {
                    temp_result.push_back(rule);
                }
            }
            
            // Add Ai -> β Ai' rules
            for (const Rule& rule : betaRules) {
                Rule newRule;
                newRule.lhs = Ai;
                
                // Copy β
                for (const string& symbol : rule.rhs) {
                    newRule.rhs.push_back(symbol);
                }
                
                // Add Ai' only if β is not empty
                if (!rule.rhs.empty()) {
                    newRule.rhs.push_back(Ai_prime);
                } else {
                    // For Ai -> ε, just add Ai -> Ai'
                    newRule.rhs.push_back(Ai_prime);
                }
                
                temp_result.push_back(newRule);
            }
            
            // If there are no β rules, add Ai -> Ai'
            if (betaRules.empty()) {
                Rule newRule;
                newRule.lhs = Ai;
                newRule.rhs.push_back(Ai_prime);
                temp_result.push_back(newRule);
            }
            
            // Add Ai' -> α Ai' rules
            for (const Rule& rule : alphaRules) {
                Rule newRule;
                newRule.lhs = Ai_prime;
                
                // Copy α (skip the first symbol which is Ai)
                for (size_t k = 1; k < rule.rhs.size(); k++) {
                    newRule.rhs.push_back(rule.rhs[k]);
                }
                
                // Add Ai'
                newRule.rhs.push_back(Ai_prime);
                
                temp_result.push_back(newRule);
            }
            
            // Add Ai' -> ε
            Rule epsilonRule;
            epsilonRule.lhs = Ai_prime;
            temp_result.push_back(epsilonRule);
            
            result = temp_result;
        }
    }
    
    // Sort the resulting grammar lexicographically
    sort(result.begin(), result.end());
    
    // Print the left-recursion-free grammar
    for (const Rule& rule : result) {
        cout << rule.lhs << " -> ";
        if (rule.rhs.empty()) {
            cout << "#" << endl;
        } else {
            for (size_t i = 0; i < rule.rhs.size(); i++) {
                cout << rule.rhs[i];
                if (i < rule.rhs.size() - 1) {
                    cout << " ";
                }
            }
            cout << " #" << endl;
        }
    }
}

int main(int argc, char* argv[]) {
    int task;
    
    if (argc < 2) {
        cout << "Error: missing task number\n";
        return 1;
    }
    
    task = atoi(argv[1]);
    
    ReadGrammar();
    
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
