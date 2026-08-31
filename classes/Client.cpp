#include "Client.hpp"

Client::Client()
    : _output_overflow(false), _pass_accepted(false),
      _nick_received(false), _user_received(false),
      _registered(false), _closing(false)
{
}

void Client::append_input(const char *data, size_t length)
{
    _input_buffer.append(data, length);
}

bool Client::pop_line(std::string &line)
{
    size_t newline = _input_buffer.find('\n');
    if (newline == std::string::npos)
        return (false);
    line = _input_buffer.substr(0, newline);
    _input_buffer.erase(0, newline + 1);
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    return (true);
}

const std::string &Client::get_input_buffer() const { return (_input_buffer); }

void Client::queue_output(const std::string &data)
{
    const size_t max_output_buffer = 1024 * 1024;
    if (data.size() > max_output_buffer - _output_buffer.size())
    {
        _output_overflow = true;
        return;
    }
    _output_buffer += data;
}

const std::string &Client::get_output_buffer() const { return (_output_buffer); }
bool Client::has_output_overflow() const { return (_output_overflow); }
void Client::consume_output(size_t length) { _output_buffer.erase(0, length); }

void Client::put_name(const std::string &name) { _name = name; }
void Client::set_username(const std::string &username) { _username = username; }
void Client::set_realname(const std::string &realname) { _realname = realname; }
void Client::set_pass_accepted(bool value) { _pass_accepted = value; }
void Client::set_nick_received(bool value) { _nick_received = value; }
void Client::set_user_received(bool value) { _user_received = value; }
void Client::set_registered(bool value) { _registered = value; }
void Client::set_closing(bool value) { _closing = value; }

const std::string &Client::get_name() const { return (_name); }
const std::string &Client::get_username() const { return (_username); }
const std::string &Client::get_realname() const { return (_realname); }
bool Client::is_pass_accepted() const { return (_pass_accepted); }
bool Client::has_nick() const { return (_nick_received); }
bool Client::has_user() const { return (_user_received); }
bool Client::is_registered() const { return (_registered); }
bool Client::is_closing() const { return (_closing); }

void Client::join_channel(const std::string &channel_name)
{
    _channels.insert(channel_name);
}

void Client::leave_channel(const std::string &channel_name)
{
    _channels.erase(channel_name);
}

bool Client::is_in_channel(const std::string &channel_name) const
{
    return (_channels.find(channel_name) != _channels.end());
}

const std::set<std::string> &Client::get_channels() const
{
    return (_channels);
}
