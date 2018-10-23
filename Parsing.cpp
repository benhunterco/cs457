/*
 * Copyright 2018 Darik Harter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Parsing.h"
using namespace Parsing;

#include <cctype>

/*
 * IRC_message
 * public
 */

IRC_message::IRC_message(const std::string& message) : iss{message}
{
    parse_message();

    // make command uppercase
    for (auto& ch : command) {
        ch = static_cast<char>(std::toupper(ch));
    }
}

/*
 * IRC_message
 * private
 *
 * Grammar comments adapted from RFC 1459 by Jarkko Oikarinen and Darren Reed.
 */

bool IRC_message::parse_message()
{
    // [ ':' <prefix> <space> ]
    if (iss.peek() == ':') {
        iss.get();
        if (!parse_prefix() || !parse_space()) {
            return false;
        }
    }

    // <command> <params> <crlf>
    return parse_command() && parse_params() && parse_crlf();
}

bool IRC_message::parse_prefix()
{
    // <servername> | <nick>
    if (!parse_name()) {
        return false;
    }

    // [ '!' <user> ]
    if (iss.peek() == '!') {
        iss.get();
        if (!parse_user()) {
            return false;
        }
    }

    // [ '@' <host> ]
    if (iss.peek() == '@') {
        iss.get();
        if (!parse_host()) {
            return false;
        }
    }

    return true;
}

bool IRC_message::parse_command()
{
    // <letter> { <letter> }
    if (parse_letter(command)) {
        while (parse_letter(command)) {
        }
        return true;
    }

    // <number> <number> <number>
    if (parse_number(command)) {
        for (auto i = 0; i < 2; i++) {
            if (!parse_number(command)) {
                return false;
            }
        }
        return true;
    }

    return false;
}

bool IRC_message::parse_space()
{
    // ' ' { ' ' }
    if (iss.peek() != ' ') {
        return false;
    }
    while (iss.peek() == ' ') {
        iss.get();
    }
    return true;
}

bool IRC_message::parse_params()
{
    // <space>
    if (!parse_space()) {
        return false;
    }

    // [ ':' <trailing> ]
    std::string param;
    if (iss.peek() == ':') {
        iss.get();
        if (parse_trailing(param)) {
            params.push_back(param);
            return true;
        }
        return false;
    }

    // [ <middle> <params> ]
    if (parse_middle(param)) {
        params.push_back(param);
        return parse_params();
    }

    return false;
}

bool IRC_message::parse_middle(std::string& target)
{
    // at least one character must be added to target
    // space, null character, carriage return, line feed not allowed
    // first character cannot be a colon
    switch (iss.peek()) {
    case ':':
    case ' ':
    case '\0':
    case '\r':
    case '\n':
        return false;
    default:
        while (iss.peek() != ' ' && iss.peek() != '\0' && iss.peek() != '\r' &&
               iss.peek() != '\n') {
            target += static_cast<char>(iss.get());
        }
        return true;
    }
}

bool IRC_message::parse_trailing(std::string& target)
{
    // characters don't have to be added to target
    // null character, carriage return, and line feed not allowed
    while (iss.peek() != '\0' && iss.peek() != '\r' && iss.peek() != '\n') {
        target += static_cast<char>(iss.get());
    }
    return true;
}

bool IRC_message::parse_crlf()
{
    // carriage return followed by line feed
    if (iss.peek() == '\r') {
        iss.get();
        if (iss.peek() == '\n') {
            iss.get();
            return true;
        }
    }
    return false;
}

bool IRC_message::parse_host()
{
    // host must be given at least one character
    // only letters, digits, '-', and '.' are allowed
    if (!isalnum(iss.peek()) && !isdigit(iss.peek()) && iss.peek() != '-' &&
        iss.peek() != '.') {
        return false;
    }
    while (isalnum(iss.peek()) || isdigit(iss.peek()) || iss.peek() == '-' ||
           iss.peek() == '.') {
        host += static_cast<char>(iss.get());
    }
    return true;
}

bool IRC_message::parse_name()
{
    // <letter>
    if (!parse_letter(name)) {
        return false;
    }

    // { <letter> | <number> | <special> }
    while (parse_letter(name) || parse_number(name) || parse_special(name)) {
    }
    return true;
}

bool IRC_message::parse_user()
{
    // '@' not allowed (see is_nonwhite())
    // <nonwhite>
    if (!is_nonwhite(iss.peek())) {
        return false;
    }

    // { <nonwhite> }
    while (is_nonwhite(iss.peek())) {
        user += static_cast<char>(iss.get());
    }

    return true;
}

bool IRC_message::parse_letter(std::string& target)
{
    // 'a' ... 'z' | 'A' ... 'Z'
    if (!std::isalpha(iss.peek())) {
        return false;
    }
    target += static_cast<char>(iss.get());
    return true;
}

bool IRC_message::parse_number(std::string& target)
{
    // '0' ... '9'
    if (!std::isdigit(iss.peek())) {
        return false;
    }
    target += static_cast<char>(iss.get());
    return true;
}

bool IRC_message::parse_special(std::string& target)
{
    // '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
    // '.' also allowed (see is_special())
    if (!is_special(iss.peek())) {
        return false;
    }
    target += static_cast<char>(iss.get());
    return true;
}

bool IRC_message::is_special(int ch) const
{
    // '.' is included to allow <servername> and <nick> to both be parsed with
    // parse_name().  Otherwise it is more difficult to parse <prefix> since
    // backtracking might be required if a '!' or '@' are found.
    switch (ch) {
    case '-':
    case '[':
    case ']':
    case '\\':
    case '`':
    case '^':
    case '{':
    case '}':
    case '.':
        return true;
    default:
        return false;
    }
}

bool IRC_message::is_nonwhite(int ch) const
{
    // '@' is included so it can't appear in <user>.  This makes it easier to
    // tell where <host> begins.
    switch (ch) {
    case ' ':
    case '\0':
    case '\r':
    case '\n':
    case '@':
        return false;
    default:
        return true;
    }
}
