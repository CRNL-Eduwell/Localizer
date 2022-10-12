#ifndef EVENTWINDOW_H
#define EVENTWINDOW_H

#include "ui_EventWindow.h"
#include <QtWidgets/QDialog>

#include "Event.h"

class EventWindow : public QDialog
{
	Q_OBJECT

public:
	EventWindow(InsermLibrary::Event sbevent, QWidget* parent = nullptr);
	~EventWindow();

private:
	void SetupComboBoxType();

private slots:
	void ValidateModifications();

signals:
	void SendModifiedData(std::string name, std::vector<int> codes, InsermLibrary::MainSecondaryEnum type);

private:
	Ui::EventWindow ui;
};

#endif
