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

#ifndef PARSING_H
#define PARSING_H

#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Contains the IRC_message struct
 */
namespace Parsing {
/**
 * @brief IRC message struct
 *
 * Represents the information for an IRC message including name (servername or
 * nickname), user, host, the command, and the params.
 */
struct IRC_message {
  public:
    /* prefix */
    /// servername or nickname
    std::string name;
    /// username on host
    std::string user;
    /// hostname of host
    std::string host;

    /* command */
    /// the IRC command (uppercased)
    std::string command;

    /* params */
    /// the command parameters
    std::vector<std::string> params;

    /**
     * @brief Constructor
     *
     * The IRC message will be parsed and the relevant fields of the IRC_message
     * struct will be filled in.
     *
     * Parsing is done according to RFC 1459 with a few exceptions.
     *
     * Exceptions include:
     *     - <host> isn't parsed exactly according to RFC 952
     *     - <servername> and <nick> are parsed in the same way
     *     - <nick> can contain a '.'
     *     - <user> cannot contain a '@'
     *
     * The message should end with backslash r backslash n.
     *
     * @param message an IRC message
     */
    IRC_message(const std::string& message);

  private:
    /// an istringstream for character by character processing
    std::istringstream iss;

    /**
     * @brief Parse <message>
     *
     * @return true if <message> parsed successfully, false otherwise
     */
    bool parse_message();

    /**
     * @brief Parse <prefix>
     *
     * @return true if <prefix> parsed successfully, false otherwise
     */
    bool parse_prefix();

    /**
     * @brief Parse <command>
     *
     * @return true if <command> parsed successfully, false otherwise
     */
    bool parse_command();

    /**
     * @brief Parse <space>
     *
     * @return true if <space> parsed successfully, false otherwise
     */
    bool parse_space();

    /**
     * @brief Parse <params>
     *
     * @return true if <params> parsed successfully, false otherwise
     */
    bool parse_params();

    /**
     * @brief Parse <middle>
     *
     * @param target the string for which to add <middle> characters
     *
     * @return true if <middle> parsed successfully, false otherwise
     */
    bool parse_middle(std::string& target);

    /**
     * @brief Parse <trailing>
     *
     * @param target the string for which to add <trailing> characters
     *
     * @return true if <trailing> parsed successfully, false otherwise
     */
    bool parse_trailing(std::string& target);

    /**
     * @brief Parse <crlf>
     *
     * @return true if <crlf> parsed successfully, false otherwise
     */
    bool parse_crlf();

    /**
     * @brief Parse <host>
     *
     * @return true if <host> parsed successfully, false otherwise
     */
    bool parse_host();

    /**
     * @brief Parse name (servername or nick)
     *
     * @return true if name parsed successfully, false otherwise
     */
    bool parse_name();

    /**
     * @brief Parse <user>
     *
     * @return true if <user> parsed successfully, false otherwise
     */
    bool parse_user();

    /**
     * @brief Parse <letter>
     *
     * @param target the string for which to add <letter>s
     *
     * @return true if <letter> parsed successfully, false otherwise
     */
    bool parse_letter(std::string& target);

    /**
     * @brief Parse <number>
     *
     * @param target the string for which to add <number>s
     *
     * @return true if <number> parsed successfully, false otherwise
     */
    bool parse_number(std::string& target);

    /**
     * @brief Parse <special>
     *
     * @param target the string for which to add <special>s
     *
     * @return true if <special> parsed successfully, false otherwise
     */
    bool parse_special(std::string& target);

    /**
     * @brief Determine if a character is <special>
     *
     * @return true if character is <special>, false otherwise
     */
    bool is_special(int ch) const;

    /**
     * @brief Determine if a character is <nonwhite>
     *
     * @return true if character is <nonwhite>, false otherwise
     */
    bool is_nonwhite(int ch) const;
};
}

#endif
