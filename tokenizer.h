#pragma once

#include <istream>
#include <optional>
#include <unordered_set>
#include <variant>

const int kDefaultSGN = 1;
const int kSingularSGN = -1;
const int kNextDischarge = 10;
const int kStartDigitsSegm = 48;
const int kFinishDigitsSegm = 57;

const std::unordered_set<char> kStartSymbols = {
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '=', '>', '*', '/', '#'};

const std::unordered_set<char> kInternalSymbols = {
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
    'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '=',
    '>', '*', '/', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '?', '!', '-'};

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const = default;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const = default;
};

struct DotToken {
    bool operator==(const DotToken&) const = default;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const = default;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    void TryParse();

    Tokenizer(std::istream* in) : flow_(in) {
        Next();
    }

    bool IsEnd() {
        return is_end_;
    }

    void Next() {
        TryParse();
    }

    Token GetToken() {
        return last_tokens_;
    }

private:
    bool IsDigit(int input) {
        return (kStartDigitsSegm <= input && input <= kFinishDigitsSegm);
    }
    bool IsValid(int input) {
        return IsDigit(input) || (input == '(') || (input == ')') || (input == '\'') ||
               (input == '.') || (input == '+') || (input == '-') ||
               (kStartSymbols.find(input) != kStartSymbols.end());
    }
    bool is_end_ = false;
    int sgn_ = kDefaultSGN;
    std::istream* flow_;
    Token last_tokens_;
};