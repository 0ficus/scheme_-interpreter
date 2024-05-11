#include "object.h"

std::set<std::string> incorrect_empty_functions = {"-", "/", "max", "min", "not"};
std::map<std::string, int64_t> empty_integer_functions = {{"+", 0}, {"*", 1}};
std::map<std::string, bool> empty_bool_functions = {{"and", true}, {"or", false}};

std::map<std::string, std::function<std::shared_ptr<Object>(
                          std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>)>>
    construct_functions = {{"cons",
                            [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
                                As<Cell>(object.first)->SetSecond(object.second);
                                return object.first;
                            }},
                           {"list",
                            [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
                                std::shared_ptr<Object> result =
                                    std::make_shared<Cell>(As<Cell>(object.first)->GetFirst(),
                                                           As<Cell>(object.first)->GetSecond());
                                std::shared_ptr<Object> jumper = As<Cell>(result)->GetSecond();
                                if (!jumper) {
                                    As<Cell>(result)->SetSecond(
                                        std::make_shared<Cell>(object.second));
                                    return result;
                                }
                                while (Is<Cell>(jumper)) {
                                    if (!As<Cell>(jumper)->GetSecond()) {
                                        break;
                                    }
                                    if (Is<Cell>(As<Cell>(jumper)->GetSecond())) {
                                        jumper = As<Cell>(jumper)->GetSecond();
                                        continue;
                                    }
                                }
                                As<Cell>(jumper)->SetSecond(std::make_shared<Cell>(object.second));
                                return result;
                            }}};

std::map<std::string, std::function<std::shared_ptr<Object>(std::shared_ptr<Object>)>>
    getter_functions = {
        {"car", [](std::shared_ptr<Object> object) { return As<Cell>(object)->GetFirst(); }},
        {"cdr", [](std::shared_ptr<Object> object) { return As<Cell>(object)->GetSecond(); }}};

std::map<std::string, std::function<std::shared_ptr<Object>(
                          std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>)>>
    getter_argument_functions = {{"list-ref",
                                  [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                         object) {
                                      std::shared_ptr<Object> list = object.first;
                                      while (!(Is<Number>(As<Cell>(list)->GetFirst()) &&
                                               As<Number>(As<Cell>(list)->GetFirst())->GetValue() ==
                                                   As<Number>(object.second)->GetValue())) {
                                          if (!As<Cell>(list)->GetSecond()) {
                                              throw RuntimeError("list has not this element\n");
                                          }
                                          list = As<Cell>(list)->GetSecond();
                                      }
                                      if (!As<Cell>(list)->GetSecond()) {
                                          throw RuntimeError("list has not this element\n");
                                      }
                                      if (Is<Cell>(As<Cell>(list)->GetSecond())) {
                                          return As<Cell>(As<Cell>(list)->GetSecond())->GetFirst();
                                      }
                                      return As<Cell>(list)->GetSecond();
                                  }},
                                 {"list-tail",
                                  [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                         object) {
                                      std::shared_ptr<Object> list = object.first;
                                      for (int64_t i = 0; i < As<Number>(object.second)->GetValue();
                                           ++i) {
                                          if (!Is<Cell>(list)) {
                                              throw RuntimeError("tail is not exist\n");
                                          }
                                          list = As<Cell>(list)->GetSecond();
                                      }
                                      return list;
                                  }}};

std::map<std::string, std::function<bool(std::shared_ptr<Object>)>> unary_bool_functions = {
    {"number?", [](std::shared_ptr<Object> object) { return Is<Number>(object); }},
    {"boolean?", [](std::shared_ptr<Object> object) { return Is<Boolean>(object); }},
    {"not",
     [](std::shared_ptr<Object>(object)) {
         if (Is<Boolean>(object)) {
             return !As<Boolean>(object)->Get();
         }
         return false;
     }},
    {"pair?",
     [](std::shared_ptr<Object> object) {
         if (!Is<Cell>(object)) {
             return false;
         }
         return !Is<Cell>(As<Cell>(object)->GetSecond()) ||
                !As<Cell>(As<Cell>(object)->GetSecond())->GetSecond();
     }},
    {"null?", [](std::shared_ptr<Object> object) { return !object; }},
    {"list?", [](std::shared_ptr<Object> object) {
         if (!object) {
             return true;
         }
         if (!Is<Cell>(object)) {
             return false;
         }
         std::shared_ptr<Object> jumper = As<Cell>(object)->GetSecond();
         while (Is<Cell>(jumper)) {
             jumper = As<Cell>(jumper)->GetSecond();
         }
         return !jumper;
     }}};

std::map<std::string, std::function<int64_t(std::shared_ptr<Object>)>> unary_integer_functions = {
    {"abs", [](std::shared_ptr<Object> object) { return abs(As<Number>(object)->GetValue()); }}};

std::map<std::string,
         std::function<bool(std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>)>>
    binary_bool_function = {
        {"=",
         [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
             return (As<Number>(object.first)->GetValue() == As<Number>(object.second)->GetValue());
         }},
        {"<",
         [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
             return (As<Number>(object.first)->GetValue() < As<Number>(object.second)->GetValue());
         }},
        {">",
         [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
             return (As<Number>(object.first)->GetValue() > As<Number>(object.second)->GetValue());
         }},
        {"<=",
         [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
             return (As<Number>(object.first)->GetValue() <= As<Number>(object.second)->GetValue());
         }},
        {">=",
         [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
             return (As<Number>(object.first)->GetValue() >= As<Number>(object.second)->GetValue());
         }},
};

std::map<std::string, std::function<std::shared_ptr<Object>(
                          std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>)>>
    or_and_function = {{"and",
                        [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
                            std::shared_ptr<Boolean> first_argument =
                                std::make_shared<Boolean>(object.first);
                            std::shared_ptr<Boolean> second_argument =
                                std::make_shared<Boolean>(object.second);
                            if (first_argument->Get() && second_argument->Get()) {
                                return object.second;
                            }
                            return As<Object>(std::make_shared<Boolean>(false));
                        }},
                       {"or",
                        [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> object) {
                            std::shared_ptr<Boolean> first_argument =
                                std::make_shared<Boolean>(object.first);
                            std::shared_ptr<Boolean> second_argument =
                                std::make_shared<Boolean>(object.second);
                            if (first_argument->Get() || second_argument->Get()) {
                                return object.second;
                            }
                            return As<Object>(std::make_shared<Boolean>(false));
                        }}};

std::map<std::string,
         std::function<int64_t(std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>)>>
    binary_integer_function = {{"+",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return As<Number>(object.first)->GetValue() +
                                           As<Number>(object.second)->GetValue();
                                }},
                               {"-",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return As<Number>(object.first)->GetValue() -
                                           As<Number>(object.second)->GetValue();
                                }},
                               {"*",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return As<Number>(object.first)->GetValue() *
                                           As<Number>(object.second)->GetValue();
                                }},
                               {"/",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return As<Number>(object.first)->GetValue() /
                                           As<Number>(object.second)->GetValue();
                                }},
                               {"max",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return std::max(As<Number>(object.first)->GetValue(),
                                                    As<Number>(object.second)->GetValue());
                                }},
                               {"min",
                                [](std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>>
                                       object) {
                                    return std::min(As<Number>(object.first)->GetValue(),
                                                    As<Number>(object.second)->GetValue());
                                }}};

int64_t ApplyEmptyIntegerFunction(const std::string& function) {
    if (incorrect_empty_functions.find(function) != incorrect_empty_functions.end()) {
        throw RuntimeError("function can't be apply to empty list arguments\n");
    }
    return empty_integer_functions[function];
}

bool ApplyEmptyBoolMutableFunction(const std::string& function) {
    return empty_bool_functions[function];
}

bool ApplyUnaryBoolFunction(const std::string& function, const std::shared_ptr<Object> object) {
    return unary_bool_functions[function](object);
}

bool ApplyBinaryBoolFunction(const std::string& function, const std::shared_ptr<Object> lhs,
                             const std::shared_ptr<Object> rhs) {
    return binary_bool_function[function](std::make_pair(lhs, rhs));
}

int64_t ApplyBinaryIntegerFunction(const std::string& function, const std::shared_ptr<Object> lhs,
                                   const std::shared_ptr<Object> rhs) {
    return binary_integer_function[function](std::make_pair(lhs, rhs));
}

int64_t ApplyUnaryIntegerFunction(const std::string& function,
                                  const std::shared_ptr<Object> object) {
    return unary_integer_functions[function](object);
}

std::shared_ptr<Object> ApplyMutableBoolFunction(const std::string& function,
                                                 const std::shared_ptr<Object> lhs,
                                                 const std::shared_ptr<Object> rhs) {
    return or_and_function[function](std::make_pair(lhs, rhs));
}

std::shared_ptr<Object> ApplyConstructorFunction(const std::string& function,
                                                 const std::shared_ptr<Object> lhs,
                                                 const std::shared_ptr<Object> rhs) {
    return construct_functions[function](std::make_pair(lhs, rhs));
}

std::shared_ptr<Object> ApplyGetterFunction(const std::string& function,
                                            const std::shared_ptr<Object> object) {
    return getter_functions[function](object);
}

std::shared_ptr<Object> ApplyGetterArgumentFunction(const std::string& function,
                                                    const std::shared_ptr<Object> lhs,
                                                    const std::shared_ptr<Object> rhs) {
    return getter_argument_functions[function](std::make_pair(lhs, rhs));
}