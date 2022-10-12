#ifndef PROTOCOLWINDOW_H
#define PROTOCOLWINDOW_H

#include "ui_ProtocolWindow.h"
#include <QtWidgets/QDialog>

#include <iostream>
#include <vector>
#include "ProvFile.h"
#include "BlocWindow.h"

class ProtocolWindow : public QDialog
{
	Q_OBJECT

public:
    ProtocolWindow(QString protocolPath, QWidget *parent = nullptr);
    ~ProtocolWindow();

private:
	void LoadBlocs();

private slots:
	void OnBlocDoubleClicked();
	void AddElement();
	void RemoveElement();
	void OnProtocolWindowAccepted();
	void OnProtocolWindowRejected();
	void ValidateModifications();

private:
	Ui::ProtocolWindow ui;
	InsermLibrary::ProvFile* m_file = nullptr;
};

#endif
