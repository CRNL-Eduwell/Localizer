#ifndef ANALYSISFOLDER_H
#define ANALYSISFOLDER_H

#include <iostream>

enum class MapType { UNKNOWN, TRIALS, TRIALS_STIM, TRIALS_RESP, BAR, PLOTS };

inline std::string MapTypeToString(MapType type)
{
    switch(type)
    {
    case MapType::TRIALS:
       return "trials";
    case MapType::TRIALS_STIM:
       return "trials_stim";
    case MapType::TRIALS_RESP:
       return "trials_resp";
    case MapType::PLOTS:
       return "plots";
    case MapType::BAR:
       return "bar";
    default:
       return "Invalid Map Type";
    }
}

inline MapType StringToMapType(std::string type)
{
    if(type == "trials")
    {
        return MapType::TRIALS;
    }
    else if(type == "trials_stim")
    {
        return MapType::TRIALS_STIM;
    }
    else if(type == "trials_resp")
    {
        return MapType::TRIALS_RESP;
    }
    else if(type == "plots")
    {
        return MapType::PLOTS;
    }
    else if(type == "bar")
    {
        return MapType::BAR;
    }
    else
    {
        return MapType::UNKNOWN;
    }
}

class AnalysisFolder
{
public:
    AnalysisFolder(std::string path);
    ~AnalysisFolder();
    inline std::string MapLabel() const { return MapTypeToString(m_MapType); };

private :
    void GetStatsInfoFromPath(std::string path);

private:
    std::string m_Path = "";
    MapType m_MapType;
    std::string m_StatAnalysis = "";
};



#endif // ANALYSISFOLDER_H
