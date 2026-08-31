#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstddef>
#include <set>
#include <string>

class Client
{
    private:
        std::set<std::string> _channels;
        std::string _name;
        std::string _username;
        std::string _realname;
        std::string _input_buffer;
        std::string _output_buffer;
        bool _output_overflow;
        bool _pass_accepted;
        bool _nick_received;
        bool _user_received;
        bool _registered;
        bool _closing;

    public:
        Client();

        void append_input(const char *data, size_t length);
        bool pop_line(std::string &line);
        const std::string &get_input_buffer() const;

        void queue_output(const std::string &data);
        const std::string &get_output_buffer() const;
        bool has_output_overflow() const;
        void consume_output(size_t length);

        void put_name(const std::string &name);
        void set_username(const std::string &username);
        void set_realname(const std::string &realname);
        void set_pass_accepted(bool value);
        void set_nick_received(bool value);
        void set_user_received(bool value);
        void set_registered(bool value);
        void set_closing(bool value);

        const std::string &get_name() const;
        const std::string &get_username() const;
        const std::string &get_realname() const;
        bool is_pass_accepted() const;
        bool has_nick() const;
        bool has_user() const;
        bool is_registered() const;
        bool is_closing() const;

        void join_channel(const std::string &channel_name);
        void leave_channel(const std::string &channel_name);
        bool is_in_channel(const std::string &channel_name) const;
        const std::set<std::string> &get_channels() const;
};

#endif
