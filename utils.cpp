#include "head.hpp"

//renvoie 1 si la string est finis
//sinon elle renvoie 0
int string_finished(std::string str)
{
    long unsigned int i = 0;

    while (i < str.size())
    {
        if (str[i] == '\n')
            return (1);
        i++;
    }
    return (0);
}
