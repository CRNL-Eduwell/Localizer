#include "ProvFile.h"
#include <QDebug>

InsermLibrary::ProvFile::ProvFile(const std::string& filePath)
{
    try
    {
        Load(filePath);
    }
    catch (std::exception& e)
    {
        std::cerr << "[Exception Caught] : " + std::string(e.what()) << std::endl;
    }
}

InsermLibrary::ProvFile::~ProvFile()
{

}

void InsermLibrary::ProvFile::Load(const std::string& filePath)
{
    std::ifstream jsonFile(filePath);
    if (jsonFile.is_open())
    {
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        jsonFile.close();

        nlohmann::json rawData = nlohmann::json::parse(buffer.str())["Blocs"];
        FillProtocolInformations(rawData);
    }
    else
    {
        throw std::runtime_error("ProvFile Load => : Could not open json file");
    }
}

void InsermLibrary::ProvFile::Save()
{

}

void InsermLibrary::ProvFile::FillProtocolInformations(nlohmann::json jsonObject)
{
    for (nlohmann::json::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it)
    {
        nlohmann::json jsonArea = it.value();
        if (!(jsonArea["Name"].is_null())){ qDebug() << jsonArea["Name"].get<std::string>().c_str(); }
        if (!(jsonArea["IllustrationPath"].is_null())){ qDebug() << jsonArea["IllustrationPath"].get<std::string>().c_str(); }
        if (!(jsonArea["Order"].is_null())){ qDebug() << jsonArea["Order"].get<int>(); }
        if (!(jsonArea["Sort"].is_null())){ qDebug() << jsonArea["Sort"].get<std::string>().c_str(); }
        if (!(jsonArea["SubBlocs"].is_null()))
        {
            for (nlohmann::json::iterator it2 = jsonArea["SubBlocs"].begin(); it2 != jsonArea["SubBlocs"].end(); ++it2)
            {
                nlohmann::json jsonArea2 = it2.value();

                qDebug() << jsonArea2["Name"].get<std::string>().c_str();
                qDebug() << jsonArea2["Order"].get<int>();
                qDebug() << jsonArea2["Type"].get<int>();
                qDebug() << jsonArea2["Window"]["Start"].get<int>();
                qDebug() << jsonArea2["Window"]["End"].get<int>();
                qDebug() << jsonArea2["Baseline"]["Start"].get<int>();
                qDebug() << jsonArea2["Baseline"]["End"].get<int>();

                for (nlohmann::json::iterator it3 = jsonArea2["Events"].begin(); it3 != jsonArea2["Events"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();

                    qDebug() << jsonArea3["Name"].get<std::string>().c_str();
                    int codeCount = jsonArea3["Codes"].size();
                    for(int i = 0; i < codeCount; i++)
                    {
                        qDebug() << jsonArea3["Codes"][i].get<int>();
                    }
                    qDebug() << jsonArea3["Type"].get<int>();
                }

                for (nlohmann::json::iterator it3 = jsonArea2["Icons"].begin(); it3 != jsonArea2["Icons"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();
                    qDebug() << jsonArea3["ImagePath"].get<std::string>().c_str();
                    qDebug() << jsonArea3["Name"].get<std::string>().c_str();
                    qDebug() << jsonArea3["Window"]["Start"].get<int>();
                    qDebug() << jsonArea3["Window"]["End"].get<int>();
                }
            }
        }
    }
}

