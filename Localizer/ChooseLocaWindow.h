#ifndef CHOOSE_LOCA_WINDOW_H
#define CHOOSE_LOCA_WINDOW_H

#include <QCoreApplication>
#include <QtWidgets/QDialog>
#include <QString>
#include "ui_ChooseLocaWindow.h"
#include <iostream>
#include <fstream>
#include "Utility.h"
#include <QDir>

class ChooseLocaWindow : public QDialog
{
	Q_OBJECT

public:
	ChooseLocaWindow(QWidget *parent = 0);
	~ChooseLocaWindow();

	inline std::vector<std::string> ElementsToLookFor() { return m_ElementsToLookFor; }

private:
	QStringList GetFilesFromRootFolder(QString fileExt);
	void LoadProtocolsInUI(QStringList protocols);
	Qt::CheckState GetDefaultState(QString label);
	void ValidateExperimentList();

private:
	std::vector<std::string> m_ElementsToLookFor;
	QString m_provFolder = QCoreApplication::applicationDirPath() + "/Resources/Config/Prov";
	Ui::ChooseLocaWindow ui;
};

#endif
