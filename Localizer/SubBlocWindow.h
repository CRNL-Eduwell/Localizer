#ifndef SUBBLOCWINDOW_H
#define SUBBLOCWINDOW_H

#include "ui_SubBlocWindow.h"
#include <QtWidgets/QDialog>

#include "SubBloc.h"

class SubBlocWindow : public QDialog
{
	Q_OBJECT

public:
    SubBlocWindow(InsermLibrary::SubBloc& subbloc, QWidget* parent = nullptr);
	~SubBlocWindow();

private:
    void LoadInUi();
    void UpdateEventDisplay(int index, std::string name);
	void LoadEvents();
	void LoadIcons();

private slots:
	void OnEventDoubleClicked();
	void OnIconDoubleClicked();
    void OnEventWindowAccepted();
	void OnEventWindowRejected();
	void ValidateModifications();

private:
	Ui::SubBlocWindow ui;
	InsermLibrary::SubBloc* m_subbloc = nullptr;
    InsermLibrary::Event m_memoryEvent;
    int m_IndexOfEvent = -1;
};

#endif
