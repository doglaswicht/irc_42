#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../head.hpp"

enum EtatsClients
{
    ETAT_NOM,
    ETAT_CHOIX_CONNEXION,
    ETAT_MOT_DE_PASSE,
    ETAT_ANCIEN_MOT_DE_PASSE,
    ETAT_DISCUSSION
};

class Client
{
    private:

        std::set<std::string> _channels;
        std::string _name;
        std::string _username;
        std::string _realname;
        std::string _password;
        std::string _input_buffer;
        std::string _output_buffer;
        bool _output_overflow;
        bool _pass_accepted;
        bool _nick_received;
        bool _user_received;
        bool _registered;
        bool _closing;
        
        //faut regarder si on peut enlever ca
        //je crois je ne l utilise pas
        bool _nouveau;

        //je dois rajouter une string de se que le client a deja ecrit
        //pour le password et les questions
        //donc je vais faire 1 string pour les name et password
        //et une autre pour les messages
        //ou je met tout dans une seule
        //que je devrai clean
        //pttr c est la meilleure idee
        //std::string _message;

        Message _message;

        //si on lui a demander le nom
        int _ask_name;
        //si on lui demande un password
        int _ask_password;

        //ce int sera la pour dire l etats du client
        //0 traitement de nom
        //1 choix si le nom est deja pris
        
        //2 nouveau mot de passe
        //3 ancien mdp si relogin
        
        //4 discussion

        EtatsClients _etats;

    public:

        Client();


        ~Client();

         
        void    add_to_message(std::string entree);

        void append_input(const char *data, size_t length);
        bool pop_line(std::string &line);
        const std::string &get_input_buffer() const;
        void queue_output(const std::string &data);
        const std::string &get_output_buffer() const;
        bool has_output_overflow() const;
        void consume_output(size_t length);

        void    put_name(std::string name);


        void    put_password(std::string password);
        void set_username(const std::string &username);
        void set_realname(const std::string &realname);
        void set_pass_accepted(bool value);
        void set_nick_received(bool value);
        void set_user_received(bool value);
        void set_registered(bool value);
        void set_closing(bool value);


        //renvoie 0 si les deux password sont bon
        //sinon renvoie -1 si c est pas bon
        int check_password(std::string password);


        //cette fonciton va nous dire si c est un client
        //qui n a pas encore de passeword ni de name
        void    put_nouveau_to_false();


        void    clean_message();

        void    put_ask_name_to_one();


        void    put_ask_password_to_one();


        //avec cette fonction on va mettre les etats differents
        void    set_etats(int nombre);

//-----------------------------------------------------------------
        //GUETTER
        std::string get_name();


        std::string get_password();
        const std::string &get_username() const;
        const std::string &get_realname() const;
        bool is_pass_accepted() const;
        bool has_nick() const;
        bool has_user() const;
        bool is_registered() const;
        bool is_closing() const;


        std::string get_message();

        
        Message get_class_message();


        bool see_if_new();

        int get_ask_name();


        int get_ask_password();


        int get_etats();


//-----------------------------------------------------------------


        void join_channel(const std::string &channel_name);
        void leave_channel(const std::string &channel_name);
        bool is_in_channel(const std::string &channel_name) const;
        const std::set<std::string> &get_channels() const;

};

#endif
