#include "IRCCommand.hpp"
#include <cctype>

IRCCommand::IRCCommand() {}

bool IRCCommand::parse(const std::string &line)
{
    size_t pos = 0;
    size_t end;

    _prefix.clear();
    _command.clear();
    _parameters.clear();
    while (pos < line.size() && line[pos] == ' ')
        ++pos;
    if (pos == line.size())
        return (false);
    if (line[pos] == ':')
    {
        end = line.find(' ', pos);
        if (end == std::string::npos || end == pos + 1)
            return (false);
        _prefix = line.substr(pos + 1, end - pos - 1);
        pos = end + 1;
        while (pos < line.size() && line[pos] == ' ')
            ++pos;
    }
    end = line.find(' ', pos);
    _command = line.substr(pos, end == std::string::npos ? end : end - pos);
    if (_command.empty())
        return (false);
    for (size_t i = 0; i < _command.size(); ++i)
        _command[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(_command[i])));
    if (end == std::string::npos)
        return (true);
    pos = end + 1;
    while (pos < line.size())
    {
        while (pos < line.size() && line[pos] == ' ')
            ++pos;
        if (pos == line.size())
            break;
        if (line[pos] == ':')
        {
            _parameters.push_back(line.substr(pos + 1));
            break;
        }
        end = line.find(' ', pos);
        _parameters.push_back(line.substr(pos, end == std::string::npos ? end : end - pos));
        if (end == std::string::npos)
            break;
        pos = end + 1;
    }
    return (true);
}

const std::string &IRCCommand::get_prefix() const { return (_prefix); }
const std::string &IRCCommand::get_command() const { return (_command); }
const std::vector<std::string> &IRCCommand::get_parameters() const { return (_parameters); }
