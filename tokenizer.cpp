#include <tokenizer.h>

void Tokenizer::TryParse() {
    int input = flow_->get();
    if (!IsValid(input)) {
        do {
            input = flow_->get();
        } while (!IsValid(input) && flow_->peek() != EOF);
    }
    if (IsDigit(input)) {
        int int_buffer = input - '0';
        while (IsDigit(flow_->peek())) {
            int_buffer = int_buffer * kNextDischarge + flow_->get() - '0';
        }
        last_tokens_ = ConstantToken{sgn_ * int_buffer};
        sgn_ = kDefaultSGN;
    } else if (input == '(' || input == ')') {
        last_tokens_ = (input == '(' ? BracketToken::OPEN : BracketToken::CLOSE);
    } else if (input == '\'') {
        last_tokens_ = QuoteToken();
    } else if (input == '.') {
        last_tokens_ = DotToken();
    } else if (input == '+' || input == '-') {
        if (IsDigit(flow_->peek())) {
            sgn_ = (input == '-' ? kSingularSGN : kDefaultSGN);
            TryParse();
            return;
        }
        last_tokens_ = SymbolToken{(input == '+' ? "+" : "-")};
    } else if (kStartSymbols.find(input) != kStartSymbols.end()) {
        char symbol = input;
        std::string str;
        str += symbol;
        while (kInternalSymbols.find(flow_->peek()) != kInternalSymbols.end()) {
            symbol = flow_->get();
            str += symbol;
        }
        last_tokens_ = SymbolToken{str};
    } else {
        is_end_ = true;
    }
    if (!IsValid(input)) {
        do {
            input = flow_->get();
        } while (!IsValid(input) && flow_->peek() != EOF);
    }
}