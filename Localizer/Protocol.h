#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <iostream>

namespace InsermLibrary
{
    class Protocol
    {
    public:
        Protocol(); //default name : new protocol + list blocs vide
        Protocol(const std::string& name); //+ List blocs
        ~Protocol();

        inline const std::string EXTENSION() { return  ".prov"; }
        inline std::string Name() { return m_name; }

        //List Blocs
        //Get List of Blocs ordered by main code
        //Is visualisable ? (bloc count > 0 and all blocs visualisable

    private:
        std::string m_name = "";
        //list blocs
    };
}
#endif // PROTOCOL_H
