#include "../head.hpp"

        Message::Message()
        {
            this->_message = "";
        }

        Message::Message(std::string message)
        {
            this->_message = message;
        }

  
        void    Message::add_to_message(std::string message)
        {

            this->_message = this->_message + message;
        }
  
        //-------------------------------------------------
        //setter
        void    Message::set_message(std::string message)
        {
            this->_message = message;
        }
        //-------------------------------------------------
        

        //-------------------------------------------------
        //guetteur
        std::string Message::get_message()
        {
            return (this->_message);
        }
        //----------------------------------------------------