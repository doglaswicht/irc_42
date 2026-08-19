#ifndef IRCCOMMAND_HPP
#define IRCCOMMAND_HPP

#include <string>
#include <vector>

class IRCCommand
{
    private:
        std::string _prefix;
        std::string _command;
        std::vector<std::string> _parameters;

    public:
        IRCCommand();

        bool parse(const std::string &line);
        const std::string &get_prefix() const;
        const std::string &get_command() const;
        const std::vector<std::string> &get_parameters() const;
};

#endif
