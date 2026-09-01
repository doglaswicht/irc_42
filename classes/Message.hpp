#ifndef MESSAGE_HPP
#define MESSAGE_HPP


#include "../head.hpp"

class Message
{
    private:
        std::string _message;
    public:

        Message();

        Message(std::string message);
        void    add_to_message(std::string message);
        void    set_message(std::string message);
        std::string get_message();


};

#endif