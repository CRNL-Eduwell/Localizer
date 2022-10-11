#include "Icon.h"

InsermLibrary::Icon::Icon()
{
	m_name = "New Icon";
	m_path = "";
	m_window = Window(-300, 300);
}

InsermLibrary::Icon::Icon(std::string name, std::string path, Window window)
{
	m_name = name;
	m_path = path;
	m_window = Window(window);
}

InsermLibrary::Icon::~Icon()
{

}