#pragma once

#include <map>
#include <sstream>
#include <string>

#include "parser.h"
#include "tokenizer.h"

class Interpreter {
public:
    std::string Run(const std::string&);
};
