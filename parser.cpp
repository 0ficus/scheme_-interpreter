#include <parser.h>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {

    if (tokenizer->IsEnd()) {
        throw SyntaxError("");
    }

    Token token = tokenizer->GetToken();
    tokenizer->Next();
    size_t cnt_open = 0;
    BracketToken* bracket_token = std::get_if<BracketToken>(&token);
    if (bracket_token && *bracket_token == BracketToken::OPEN) {
        ++cnt_open;
        std::shared_ptr<Cell> cell = std::make_shared<Cell>();
        std::shared_ptr<Cell> curr_cell(cell);
        size_t size = 0;
        while (!tokenizer->IsEnd()) {
            token = tokenizer->GetToken();
            DotToken* dot_token = std::get_if<DotToken>(&token);
            if (dot_token) {
                throw SyntaxError("dot can't be after open scope\n");
            }
            bracket_token = std::get_if<BracketToken>(&token);
            if (bracket_token && *bracket_token == BracketToken::CLOSE) {
                if (cnt_open) {
                    --cnt_open;
                } else {
                    throw SyntaxError("");
                }
                if (size == 0) {
                    tokenizer->Next();
                    if (!cnt_open) {
                        return nullptr;
                    }
                    throw SyntaxError("");
                }
                break;
            }

            bool is_closed = false;
            std::shared_ptr<Object> object = nullptr;
            while (!object && !tokenizer->IsEnd()) {
                ++size;
                token = tokenizer->GetToken();
                bracket_token = std::get_if<BracketToken>(&token);
                if (bracket_token && *bracket_token == BracketToken::CLOSE) {
                    is_closed = true;
                    break;
                }
                object = Read(tokenizer);
            }
            if (object && !is_closed) {
                ++size;
                curr_cell->SetFirst(object);
            } else if (!is_closed) {
                if (!cnt_open) {
                    return cell;
                }
                throw SyntaxError("an expression has not closed bracket to open bracket\n");
            } else if (!cnt_open) {
                throw SyntaxError("an expression has not open bracket to close bracket\n");
            }

            if (tokenizer->IsEnd() && cnt_open) {
                throw SyntaxError("an expression has not close bracket to some open bracket\n");
            }
            token = tokenizer->GetToken();
            bracket_token = std::get_if<BracketToken>(&token);
            if (bracket_token && *bracket_token == BracketToken::CLOSE) {
                if (cnt_open) {
                    --cnt_open;
                } else {
                    throw SyntaxError("");
                }
                tokenizer->Next();
                if (!cnt_open) {
                    return cell;
                }
                throw SyntaxError("");
            }

            dot_token = std::get_if<DotToken>(&token);
            if (dot_token) {
                tokenizer->Next();
                if (tokenizer->IsEnd()) {
                    throw SyntaxError("an expression ends with a dot\n");
                }
                token = tokenizer->GetToken();
                bracket_token = std::get_if<BracketToken>(&token);
                if (bracket_token && *bracket_token == BracketToken::CLOSE) {
                    throw SyntaxError(
                        "an expression has not argument between dot and close bracket\n");
                }
                curr_cell->SetSecond(Read(tokenizer));
                if (tokenizer->IsEnd()) {
                    throw SyntaxError(
                        "an expression has not close bracket for some open bracket\n");
                }
                token = tokenizer->GetToken();
                bracket_token = std::get_if<BracketToken>(&token);
                if (!bracket_token || *bracket_token != BracketToken::CLOSE) {
                    throw SyntaxError("an expression has vide of pair, but it is not pair\n");
                }
                tokenizer->Next();
                if (cnt_open == 1) {
                    return cell;
                }
                throw SyntaxError("an expression has not close bracket for some open bracket\n");
            }

            curr_cell->SetSecond(std::make_shared<Cell>());
            curr_cell = As<Cell>(curr_cell->GetSecond());
        }
        if (!cnt_open) {
            return cell;
        }
        throw SyntaxError("");
    }

    SymbolToken* symbol_token = std::get_if<SymbolToken>(&token);
    if (symbol_token) {
        return std::make_shared<Symbol>(symbol_token->name);
    }

    ConstantToken* constant_token = std::get_if<ConstantToken>(&token);
    if (constant_token) {
        return std::make_shared<Number>(constant_token->value);
    }

    DotToken* dot_token = std::get_if<DotToken>(&token);
    if (dot_token) {
        return std::make_shared<Dot>();
    }

    QuoteToken* quote_token = std::get_if<QuoteToken>(&token);
    if (quote_token) {
        return std::make_shared<Cell>(std::make_shared<Symbol>("quote"), Read(tokenizer));
    }

    return nullptr;
}