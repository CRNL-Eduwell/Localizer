#ifndef BLOC_H
#define BLOC_H

#include <iostream>
#include "SubBloc.h"

namespace InsermLibrary
{
    class Bloc
    {
    public:
        Bloc();
        Bloc(std::string name, int order, std::string illustrationPath, std::string sort, std::vector<SubBloc> subBlocs);
        ~Bloc();

        //Getter
        inline const std::string Name() { return m_name; }
        inline int Order() { return m_order; }
        inline const std::string IllustrationPath() { return m_illustrationPath; }
        inline const std::string Sort() { return m_sort; }
        inline const std::vector<SubBloc>& SubBlocs() { return m_subBlocs; }
        inline SubBloc MainSubBloc()
        {
            auto it = std::find_if(m_subBlocs.begin(), m_subBlocs.end(), [&](SubBloc a){ return a.Type() == MainSecondaryEnum::Main; });
            return it != m_subBlocs.end() ? *it : SubBloc();
        }
        // Setter
        inline void Name(std::string name) { m_name = name; }
        inline void Order(int order) { m_order = order; }
        inline void IllustrationPath(std::string illustrationPath) { m_illustrationPath = illustrationPath; }
        inline void Sort(std::string sort) { m_sort = sort; }
        inline void SubBlocs(std::vector<SubBloc> subBlocs) { m_subBlocs = std::vector<SubBloc>(subBlocs); }

        //list subblocs sorted by order
        //IsVisualizable
    private:
        std::string m_name = "";
        int m_order = 0;
        std::string m_illustrationPath = "";
        std::string m_sort = "";
        std::vector<SubBloc> m_subBlocs;
    };
}
#endif // BLOC_H
