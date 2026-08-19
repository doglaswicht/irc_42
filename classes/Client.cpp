 #include "../head.hpp"
 
 
        Client::Client()
        {
            this->_nouveau = true;
            this->_name = "";
            this->_username = "";
            this->_realname = "";
            this->_password = "";
            this->_input_buffer = "";
            this->_output_buffer = "";
            this->_output_overflow = false;
            this->_pass_accepted = false;
            this->_nick_received = false;
            this->_user_received = false;
            this->_registered = false;
            this->_closing = false;
            this->_message.add_to_message("");
            this->_ask_name = 0;
            this->_ask_password = 0;
            this->_etats = ETAT_NOM;
        }

        Client::~Client()
        {

        }
         
        void    Client::add_to_message(std::string entree)
        {
            this->_message.add_to_message(entree);
        }

        void Client::append_input(const char *data, size_t length)
        {
            this->_input_buffer.append(data, length);
        }

        bool Client::pop_line(std::string &line)
        {
            size_t newline = this->_input_buffer.find('\n');
            if (newline == std::string::npos)
                return (false);
            line = this->_input_buffer.substr(0, newline);
            this->_input_buffer.erase(0, newline + 1);
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);
            return (true);
        }

        const std::string &Client::get_input_buffer() const
        {
            return (this->_input_buffer);
        }

        void Client::queue_output(const std::string &data)
        {
            const size_t max_output_buffer = 1024 * 1024;
            if (data.size() > max_output_buffer - this->_output_buffer.size())
            {
                this->_output_overflow = true;
                return;
            }
            this->_output_buffer += data;
        }

        const std::string &Client::get_output_buffer() const
        {
            return (this->_output_buffer);
        }

        bool Client::has_output_overflow() const
        {
            return (this->_output_overflow);
        }

        void Client::consume_output(size_t length)
        {
            this->_output_buffer.erase(0, length);
        }

        void    Client::put_name(std::string name)
        {
            this->_name = name;
        }

        void    Client::put_password(std::string password)
        {
            this->_password = password;
        }

        void Client::set_username(const std::string &username) { this->_username = username; }
        void Client::set_realname(const std::string &realname) { this->_realname = realname; }
        void Client::set_pass_accepted(bool value) { this->_pass_accepted = value; }
        void Client::set_nick_received(bool value) { this->_nick_received = value; }
        void Client::set_user_received(bool value) { this->_user_received = value; }
        void Client::set_registered(bool value) { this->_registered = value; }
        void Client::set_closing(bool value) { this->_closing = value; }

        //renvoie 0 si les deux password sont bon
        //sinon renvoie -1 si c est pas bon
        int Client::check_password(std::string password)
        {
            if (this->_password == password)
                return (0);
            return (-1);
        }

        //cette fonciton va nous dire si c est un client
        //qui n a pas encore de passeword ni de name
        void    Client::put_nouveau_to_false()
        {
            this->_nouveau = false;
        }

        void    Client::clean_message()
        {
            this->_message.set_message("");
        }

        void    Client::put_ask_name_to_one()
        {
            this->_ask_name = 1;
        }

        void    Client::put_ask_password_to_one()
        {
            this->_ask_password = 1;
        }

        //avec cette fonction on va mettre les etats differents
        void    Client::set_etats(int nombre)
        {
            this->_etats = (EtatsClients)nombre;
        }

//-----------------------------------------------------------------
        //GUETTER
        std::string Client::get_name()
        {
            return (this->_name);
        }

        std::string Client::get_password()
        {
            return (this->_password);
        }

        const std::string &Client::get_username() const { return (this->_username); }
        const std::string &Client::get_realname() const { return (this->_realname); }
        bool Client::is_pass_accepted() const { return (this->_pass_accepted); }
        bool Client::has_nick() const { return (this->_nick_received); }
        bool Client::has_user() const { return (this->_user_received); }
        bool Client::is_registered() const { return (this->_registered); }
        bool Client::is_closing() const { return (this->_closing); }

        std::string Client::get_message()
        {
            return (this->_message.get_message());
        }
        
        Message Client::get_class_message()
        {
            return (this->_message);
        }

        bool Client::see_if_new()
        {
            return (this->_nouveau);
        }

        int Client::get_ask_name()
        {
            return (this->_ask_name);
        }

        int Client::get_ask_password()
        {
            return (this->_ask_password);
        }


        int Client::get_etats()
        {
            return (this->_etats);
        }


        void Client::join_channel(const std::string &channel_name)
        {
            this->_channels.insert(channel_name);
        }

        void Client::leave_channel(const std::string &channel_name)
        {
            this->_channels.erase(channel_name);
        }

        bool Client::is_in_channel(const std::string &channel_name) const
        {
            return (this->_channels.find(channel_name) != this->_channels.end());
        }

        const std::set<std::string> &Client::get_channels() const
        {
            return (this->_channels);
        }
