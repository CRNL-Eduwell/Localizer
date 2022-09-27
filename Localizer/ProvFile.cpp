#include "ProvFile.h"

InsermLibrary::ProvFile::ProvFile(const std::string& filePath)
{
    try
    {
        m_filePath = std::string(filePath);
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

void InsermLibrary::ProvFile::FillProtocolInformations(nlohmann::json jsonObject)
{
    for (nlohmann::json::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it)
    {
        std::vector<SubBloc> subBlocs;
        std::string blocName = "", blocIllustrationPath = "", blocSort = "";
        int blocOrder = 0;

        nlohmann::json jsonArea = it.value();
        if (!(jsonArea["Name"].is_null())){ blocName = jsonArea["Name"].get<std::string>(); }
        if (!(jsonArea["IllustrationPath"].is_null())){ blocIllustrationPath = jsonArea["IllustrationPath"].get<std::string>(); }
        if (!(jsonArea["Order"].is_null())){ blocOrder = jsonArea["Order"].get<int>(); }
        if (!(jsonArea["Sort"].is_null())){ blocSort = jsonArea["Sort"].get<std::string>(); }
        if (!(jsonArea["SubBlocs"].is_null()))
        {
            for (nlohmann::json::iterator it2 = jsonArea["SubBlocs"].begin(); it2 != jsonArea["SubBlocs"].end(); ++it2)
            {
                nlohmann::json jsonArea2 = it2.value();
                std::string subBlocName = jsonArea2["Name"].get<std::string>();
                int subBlocOrder = jsonArea2["Order"].get<int>();
                MainSecondaryEnum type = (MainSecondaryEnum)jsonArea2["Type"].get<int>();
                //==
                int begWindow = jsonArea2["Window"]["Start"].get<int>();
                int endWindow = jsonArea2["Window"]["End"].get<int>();
                Window subBlocWIndow(begWindow, endWindow);
                //==
                begWindow = jsonArea2["Baseline"]["Start"].get<int>();
                endWindow = jsonArea2["Baseline"]["End"].get<int>();
                Window baseLineWindow(begWindow, endWindow);

                std::vector<Event> events;
                for (nlohmann::json::iterator it3 = jsonArea2["Events"].begin(); it3 != jsonArea2["Events"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();
                    std::string name = jsonArea3["Name"].get<std::string>();
                    int codeCount = jsonArea3["Codes"].size();
                    std::vector<int> codes;
                    for(int i = 0; i < codeCount; i++)
                    {
                        codes.push_back(jsonArea3["Codes"][i].get<int>());
                    }
                    MainSecondaryEnum type = (MainSecondaryEnum)jsonArea3["Type"].get<int>();

                    events.push_back(Event(name, codes, type));
                }

                std::vector<Icon> icons;
                for (nlohmann::json::iterator it3 = jsonArea2["Icons"].begin(); it3 != jsonArea2["Icons"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();
                    std::string imagePath = jsonArea3["ImagePath"].get<std::string>();
                    std::string imageName = jsonArea3["Name"].get<std::string>();
                    int begWindow = jsonArea3["Window"]["Start"].get<int>();
                    int endWindow = jsonArea3["Window"]["End"].get<int>();

                    icons.push_back(Icon(imageName, imagePath, Window(begWindow, endWindow)));
                }

                subBlocs.push_back(SubBloc(subBlocName, subBlocOrder, type, subBlocWIndow, baseLineWindow, events, icons));
            }
        }

        m_blocs.push_back(Bloc(blocName, blocOrder, blocIllustrationPath, blocSort, subBlocs));
    }
}

void InsermLibrary::ProvFile::Save()
{
    throw std::runtime_error("ProvFile Save => : Not implemented yet, oupsi");
}
