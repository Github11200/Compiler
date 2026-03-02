#ifndef UTILS_H
#define UTILS_H

#include "token.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>
#include <string>
#include <vector>

std::vector<std::string> splitString(std::string &input, std::set<std::string> &delimiters);
bool isInteger(std::string &input);
bool isDouble(std::string &input);
bool isOperator(TokenType tokenType);

template <class T> void printVector(const std::vector<T> &inputVector);

#endif
