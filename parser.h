#pragma once

#include <queue>
#include <map>

#include "error.h"
#include "object.h"
#include "tokenizer.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer);