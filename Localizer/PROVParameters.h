#ifndef _PROV_PARAMETERS_H
#define _PROV_PARAMETERS_H

#include <iostream>
#include <vector>

using namespace std;

struct invertMap
{
	int epochWindow[2];
	int baseLineWindow[2];
};

struct displayBLOC
{
	int row;
	int col;
	string name;
	string path;
	string sort;
	int epochWindow[2];
	int baseLineWindow[2];
};

struct EventBLOC
{
	vector<int> eventCode;
	string eventLabel;
};

struct BLOC
{
	displayBLOC dispBloc;
	EventBLOC mainEventBloc;
	vector<EventBLOC> secondaryEvents;
};

#endif