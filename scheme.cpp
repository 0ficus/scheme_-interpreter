#include "scheme.h"
#include <deque>

std::deque<int> a;

std::map<std::string, std::shared_ptr<Object>> apply_function = {
    {"quote", nullptr},
    {"and", std::make_shared<NonTypeBinaryBoolFunction>()},
    {"or", std::make_shared<NonTypeBinaryBoolFunction>()},
    {"not", std::make_shared<OnlyUnaryBoolFunction>()},
    {"boolean?", std::make_shared<UnaryBoolFunction>()},
    {"number?", std::make_shared<UnaryBoolFunction>()},
    {"pair?", std::make_shared<UnaryBoolFunction>()},
    {"null?", std::make_shared<UnaryBoolFunction>()},
    {"list?", std::make_shared<UnaryBoolFunction>()},
    {"cons", std::make_shared<ConstructorFunction>()},
    {"list", std::make_shared<ConstructorFunction>()},
    {"car", std::make_shared<GetterFunction>()},
    {"cdr", std::make_shared<GetterFunction>()},
    {"list-ref", std::make_shared<GetterFunction>()},
    {"list-tail", std::make_shared<GetterFunction>()},
    {"=", std::make_shared<BinaryBoolFunction>()},
    {"<", std::make_shared<BinaryBoolFunction>()},
    {">", std::make_shared<BinaryBoolFunction>()},
    {"<=", std::make_shared<BinaryBoolFunction>()},
    {">=", std::make_shared<BinaryBoolFunction>()},
    {"+", std::make_shared<BinaryIntegerFunction>()},
    {"-", std::make_shared<BinaryIntegerFunction>()},
    {"*", std::make_shared<BinaryIntegerFunction>()},
    {"/", std::make_shared<BinaryIntegerFunction>()},
    {"max", std::make_shared<BinaryIntegerFunction>()},
    {"min", std::make_shared<BinaryIntegerFunction>()},
    {"abs", std::make_shared<UnaryIntegerFunction>()}};

std::shared_ptr<Object> DefinitePointer(const std::shared_ptr<Object>& object) {
    if (Is<Symbol>(object) &&
        (As<Symbol>(object)->GetName() == "#t" || As<Symbol>(object)->GetName() == "#f")) {
        return std::make_shared<Boolean>(As<Symbol>(object)->GetName() == "#t");
    }
    return object;
}

std::shared_ptr<Object> Calc(std::shared_ptr<Object> object) {
    if (object && Is<Cell>(object) && As<Cell>(object)->GetFirst()) {
        if (Is<Symbol>(As<Cell>(object)->GetFirst()) &&
            As<Symbol>(As<Cell>(object)->GetFirst())->GetName() == "quote") {
            return As<Cell>(object)->GetSecond();
        }
    }
    if (!object || Is<Number>(object) || Is<Symbol>(object) || Is<Dot>(object)) {
        return object;
    }
    if (Is<Cell>(object)) {
        As<Cell>(object)->SetFirst(Calc(As<Cell>(object)->GetFirst()));
        As<Cell>(object)->SetSecond(Calc(As<Cell>(object)->GetSecond()));
    }
    if (Is<Symbol>(As<Cell>(object)->GetFirst()) &&
        apply_function.find(As<Symbol>(As<Cell>(object)->GetFirst())->GetName()) !=
            apply_function.end()) {
        std::vector<std::shared_ptr<Object>> argument_collector;
        std::shared_ptr<Object> argument_jumper = As<Cell>(object)->GetSecond();
        while (argument_jumper) {
            if (Is<Cell>(argument_jumper)) {
                argument_collector.emplace_back(
                    DefinitePointer(As<Cell>(argument_jumper)->GetFirst()));
                argument_jumper = As<Cell>(argument_jumper)->GetSecond();
                continue;
            }
            argument_collector.emplace_back(DefinitePointer(argument_jumper));
            break;
        }
        return apply_function[As<Symbol>(As<Cell>(object)->GetFirst())->GetName()]->Apply(
            As<Symbol>(As<Cell>(object)->GetFirst())->GetName(), argument_collector);
    }
    return object;
}

std::string Interpreter::Run(const std::string& expression) {

    std::stringstream flow(expression);
    Tokenizer tokenizer(&flow);

    std::shared_ptr<Object> ast = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("an expression can't be read in full size\n");
    }
    if (!ast) {
        throw RuntimeError("empty expression\n");
    }
    std::shared_ptr<Object> check_operations = ast;
    if (Is<Cell>(check_operations)) {
        if (!Is<Symbol>(As<Cell>(check_operations)->GetFirst()) ||
            apply_function.find(As<Symbol>(As<Cell>(check_operations)->GetFirst())->GetName()) ==
                apply_function.end()) {
            throw RuntimeError("this expression has not operations\n");
        }
    }
    ast = Calc(ast);
    if (!ast) {
        return std::make_shared<Nullptr>()->ToString();
    }
    return ast->ToString();
}