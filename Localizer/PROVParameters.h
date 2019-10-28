#ifndef _PROV_PARAMETERS_H
#define _PROV_PARAMETERS_H

#include <iostream>
#include <vector>
#include "Utility.h"

namespace InsermLibrary
{
	struct invertMap
	{
		int epochWindow[2]{ 0, 0 };
		int baseLineWindow[2]{ 0, 0 };
	};

	struct displayBLOC
	{
		inline int row() const { return m_row; };
		inline void row(int rowValue) { m_row = rowValue; };
		inline int column() const { return m_col; };
		inline void column(int columnValue) { m_col = columnValue; };
		inline std::string name() const { return m_name; };
		inline void name(std::string nameValue) { m_name = nameValue; };
		inline std::string path() const { return m_path; };
		inline std::string relativPath(std::string root) const 
		{ 
			std::string::size_type n;
			n = m_path.find_first_not_of(root);
			if (n != std::string::npos)
				return "/" + m_path.substr(n);
			else
				return "";
		};
		inline void path(std::string pathValue) { m_path = pathValue; };
		inline std::string sort() const { return m_sort; };
		inline void sort(std::string sortValue) { m_sort = sortValue; };
		inline int windowMin() const { return m_epochWindow[0]; };
		inline int windowMax() const { return m_epochWindow[1]; };
		inline void window(int minValue, int maxValue) { m_epochWindow[0] = minValue; m_epochWindow[1] = maxValue; };
		inline int baseLineMin() const { return m_baseLineWindow[0]; };
		inline int baseLineMax() const { return m_baseLineWindow[1]; };
		inline void baseLine(int minValue, int maxValue) { m_baseLineWindow[0] = minValue; m_baseLineWindow[1] = maxValue; };

	private:
		int m_row;
		int m_col;
		std::string m_name;
		std::string m_path;
		std::string m_sort;
		int m_epochWindow[2];
		int m_baseLineWindow[2];
	};

	struct EventBLOC
	{
		std::vector<int> eventCode;
		std::string eventLabel;
	};

	struct BLOC
	{
		displayBLOC dispBloc;
		EventBLOC mainEventBloc;
		std::vector<EventBLOC> secondaryEvents;
	};
}
#endif