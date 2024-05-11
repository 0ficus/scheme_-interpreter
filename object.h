#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "error.h"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::string ToString() = 0;
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) = 0;
};

class Dot : public Object {
public:
    virtual ~Dot() override = default;
    virtual std::string ToString() override {
        return ".";
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
};

class Number : public Object {
public:
    virtual ~Number() override = default;
    virtual std::string ToString() override {
        return std::to_string(value_);
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
    Number(int64_t value) : value_{value} {
    }
    int GetValue() const {
        return value_;
    }

private:
    int64_t value_;
};

class Symbol : public Object {
public:
    virtual ~Symbol() override = default;
    virtual std::string ToString() override {
        return name_;
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
    Symbol(const std::string& name) : name_{name} {
    }
    const std::string& GetName() const {
        return name_;
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    virtual ~Cell() override = default;

    virtual std::string ToString() override {
        std::shared_ptr<Cell> first_cell = std::dynamic_pointer_cast<Cell>(first_);
        std::shared_ptr<Cell> second_cell = std::dynamic_pointer_cast<Cell>(second_);
        if (GetFirst() && GetSecond() && !first_cell && !second_cell) {
            return "(" + first_->ToString() + " . " + second_->ToString() + ")";
        }
        if (!first_ && !second_) {
            return "(())";
        }
        if (!first_) {
            if (second_cell) {
                return second_->ToString();
            }
            return "(" + second_->ToString() + ")";
        }
        if (!second_) {
            if (first_cell) {
                return first_->ToString();
            }
            return "(" + first_->ToString() + ")";
        }
        if (second_cell) {
            std::shared_ptr<Cell> second_cell_first =
                std::dynamic_pointer_cast<Cell>(second_cell->GetFirst());
            std::shared_ptr<Cell> second_cell_second =
                std::dynamic_pointer_cast<Cell>(second_cell->GetSecond());
            if (!second_cell_second) {
                if (second_cell->GetSecond()) {
                    return "(" + first_->ToString() + " " + second_cell->GetFirst()->ToString() +
                           " . " + second_cell->GetSecond()->ToString() + ")";
                }
                return "(" + first_->ToString() + " " + second_cell->GetFirst()->ToString() + ")";
            }
            return "(" + first_->ToString() + " " + second_cell->GetFirst()->ToString() + " " +
                   second_cell_second->GetFirst()->ToString() + ")";
        }
        return "(" + first_->ToString() + " " + second_->ToString() + ")";
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
    Cell(std::shared_ptr<Object> first = nullptr, std::shared_ptr<Object> second = nullptr)
        : first_{nullptr}, second_{nullptr} {
        SetFirst(first);
        SetSecond(second);
    }
    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    template <typename T>
    void SetFirst(std::shared_ptr<T> object) {
        first_ = std::dynamic_pointer_cast<T>(first_);
        first_ = object;
    }

    template <typename T>
    void SetSecond(std::shared_ptr<T> object) {
        second_ = std::dynamic_pointer_cast<T>(second_);
        second_ = object;
    }

private:
    std::shared_ptr<Object> first_{nullptr};
    std::shared_ptr<Object> second_{nullptr};
};

class Nullptr : public Object {
public:
    virtual ~Nullptr() override = default;
    virtual std::string ToString() override {
        return "()";
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
};

class Boolean : public Object {
public:
    virtual ~Boolean() override = default;
    virtual std::string ToString() override {
        return (value_ ? "#t" : "#f");
    }
    virtual std::shared_ptr<Object> Apply(const std::string&,
                                          const std::vector<std::shared_ptr<Object>>&) override {
        return nullptr;
    }
    Boolean(bool value = false) : value_{value} {
    }
    Boolean(std::shared_ptr<Object> object) {
        if (std::dynamic_pointer_cast<Boolean>(object)) {
            std::shared_ptr<Boolean> bool_object = std::dynamic_pointer_cast<Boolean>(object);
            value_ = bool_object->Get();
        } else {
            value_ = true;
        }
    }
    void Set(bool value) {
        value_ = value;
    }
    bool Get() {
        return value_;
    }

private:
    bool value_;
};

///////////////////////////////////////////////////////////////////////////////

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj) != nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int64_t ApplyEmptyIntegerFunction(const std::string& function);
bool ApplyEmptyBoolMutableFunction(const std::string& function);

bool ApplyUnaryBoolFunction(const std::string& function, const std::shared_ptr<Object> object);
bool ApplyBinaryBoolFunction(const std::string& function, const std::shared_ptr<Object> lhs,
                             const std::shared_ptr<Object> rhs);

int64_t ApplyBinaryIntegerFunction(const std::string& function, const std::shared_ptr<Object> lhs,
                                   const std::shared_ptr<Object> rhs);
int64_t ApplyUnaryIntegerFunction(const std::string& function,
                                  const std::shared_ptr<Object> object);
std::shared_ptr<Object> ApplyMutableBoolFunction(const std::string& function,
                                                 const std::shared_ptr<Object> lhs,
                                                 const std::shared_ptr<Object> rhs);

std::shared_ptr<Object> ApplyConstructorFunction(const std::string& function,
                                                 const std::shared_ptr<Object> lhs,
                                                 const std::shared_ptr<Object> rhs);

std::shared_ptr<Object> ApplyGetterFunction(const std::string& function,
                                            const std::shared_ptr<Object> object);

std::shared_ptr<Object> ApplyGetterArgumentFunction(const std::string& function,
                                                    const std::shared_ptr<Object> lhs,
                                                    const std::shared_ptr<Object> rhs);

struct UnaryBoolFunction : public Object {
    virtual ~UnaryBoolFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        bool result = true;
        for (const std::shared_ptr<Object>& i : args) {
            result &= ApplyUnaryBoolFunction(func, i);
        }
        return std::make_shared<Boolean>(result);
    }
};

struct BinaryBoolFunction : public Object {
    virtual ~BinaryBoolFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (args.size() < 2) {
            return std::make_shared<Boolean>(true);
        }
        std::shared_ptr<UnaryBoolFunction> checker_to_same_type =
            std::make_shared<UnaryBoolFunction>();

        bool result =
            As<Boolean>(As<UnaryBoolFunction>(checker_to_same_type)->Apply("number?", args))->Get();
        if (!result) {
            throw RuntimeError("arguments are belong to different types\n");
        }
        for (size_t i = 1; i < args.size(); ++i) {
            result &= ApplyBinaryBoolFunction(func, args[i - 1], args[i]);
        }
        return std::make_shared<Boolean>(result);
    }
};

struct BinaryIntegerFunction : public Object {
    virtual ~BinaryIntegerFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (args.empty()) {
            return std::make_shared<Number>(ApplyEmptyIntegerFunction(func));
        }
        std::shared_ptr<UnaryBoolFunction> checker_to_same_type =
            std::make_shared<UnaryBoolFunction>();

        bool is_numbers =
            As<Boolean>(As<UnaryBoolFunction>(checker_to_same_type)->Apply("number?", args))->Get();
        if (!is_numbers) {
            throw RuntimeError("arguments are belong to different types\n");
        }
        int64_t result = As<Number>(args[0])->GetValue();
        for (size_t i = 1; i < args.size(); ++i) {
            result = ApplyBinaryIntegerFunction(func, std::make_shared<Number>(result), args[i]);
        }

        return std::make_shared<Number>(result);
    }
};

struct UnaryIntegerFunction : public Object {
    virtual ~UnaryIntegerFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (args.size() != 1) {
            throw RuntimeError("this function must have only one argument\n");
        }
        std::shared_ptr<UnaryBoolFunction> checker_to_same_type =
            std::make_shared<UnaryBoolFunction>();
        bool is_numbers =
            As<Boolean>(As<UnaryBoolFunction>(checker_to_same_type)->Apply("number?", args))->Get();
        if (!is_numbers) {
            throw RuntimeError("arguments are belong to different types\n");
        }
        return std::make_shared<Number>(ApplyUnaryIntegerFunction(func, args[0]));
    }
};

struct OnlyUnaryBoolFunction : public Object {
public:
    virtual ~OnlyUnaryBoolFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (args.size() != 1) {
            throw RuntimeError("this function must have only one argument\n");
        }
        return std::make_shared<Boolean>(ApplyUnaryBoolFunction(func, args[0]));
    }
};

struct NonTypeBinaryBoolFunction : public Object {
public:
    virtual ~NonTypeBinaryBoolFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        std::shared_ptr<Object> result =
            std::make_shared<Boolean>(ApplyEmptyBoolMutableFunction(func));
        for (size_t i = 0; i < args.size(); ++i) {
            result = ApplyMutableBoolFunction(func, std::make_shared<Boolean>(result), args[i]);
        }
        return result;
    }
};

struct ConstructorFunction : public Object {
public:
    virtual ~ConstructorFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (args.empty()) {
            return std::make_shared<Symbol>("()");
        }
        if (args.size() == 1) {
            return std::make_shared<Cell>(args[0]);
        }
        std::shared_ptr<Cell> result = std::make_shared<Cell>(args[0]);
        for (size_t i = 1; i < args.size(); ++i) {
            result = As<Cell>(ApplyConstructorFunction(func, result, args[i]));
        }
        return result;
    }
};

struct GetterFunction : public Object {
public:
    virtual ~GetterFunction() override = default;
    virtual std::string ToString() override {
        return "can't print a function\n";
    }
    virtual std::shared_ptr<Object> Apply(
        const std::string& func, const std::vector<std::shared_ptr<Object>>& args) override {
        if (!args[0]) {
            throw RuntimeError("can't do this operation with empty object\n");
        }
        if (args.size() == 1) {
            return ApplyGetterFunction(func, args[0]);
        }
        return ApplyGetterArgumentFunction(func, args[0], args[1]);
    }
};