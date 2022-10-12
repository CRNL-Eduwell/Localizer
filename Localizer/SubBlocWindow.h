#ifndef SUBBLOCWINDOW_H
#define SUBBLOCWINDOW_H

#include "ui_SubBlocWindow.h"
#include <QtWidgets/QDialog>

//#include <iostream>
//#include <vector>
//#include "ProvFile.h"

#include "SubBloc.h"
#include "EventWindow.h"

class SubBlocWindow : public QDialog
{
	Q_OBJECT

public:
	SubBlocWindow(InsermLibrary::SubBloc subbloc, QWidget* parent = nullptr);
	~SubBlocWindow();

private:
	void LoadEvents();
	void LoadIcons();

private slots:
	void OnEventDoubleClicked();
	void OnIconDoubleClicked();
	void OnEventWindowAccepted(std::string name, std::vector<int> codes, InsermLibrary::MainSecondaryEnum type);
	void OnEventWindowRejected();
	void ValidateModifications();

private:
	Ui::SubBlocWindow ui;
	InsermLibrary::SubBloc* m_subbloc = nullptr;
};

#endif
