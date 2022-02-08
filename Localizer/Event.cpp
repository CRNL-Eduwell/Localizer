#include "Event.h"

InsermLibrary::Event::Event()
{
    m_name = "New Event";
    m_codes = std::vector<int>();
    m_type = MainSecondaryEnum::Main;
}

InsermLibrary::Event::Event(std::string name, std::vector<int> codes, MainSecondaryEnum type)
{
    m_name = name;
    m_codes = std::vector<int>(codes);
    m_type = type;
}

InsermLibrary::Event::~Event()
{

}
