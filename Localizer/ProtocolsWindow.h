#ifndef PROTOCOLSWINDOW_H
#define PROTOCOLSWINDOW_H

#include "ui_ProtocolsWindow.h"
#include <QtWidgets/QDialog>
#include <QDir>
#include <QList>
#include <QString>
#include <iostream>

#include "ProtocolWindow.h"

class ProtocolsWindow : public QDialog
{
	Q_OBJECT

public:
	ProtocolsWindow(QWidget *parent = nullptr);
    ~ProtocolsWindow();

private:
	QStringList GetProtocolsFileList();
	void LoadProtocols(QStringList protocols);

private slots:
	void OnProtocolDoubleClicked(QListWidgetItem* item);
	void AddElement();
	void RemoveElement();
	void OnProtocolWindowAccepted();
	void OnProtocolWindowRejected();

private:
	Ui::ProtocolsWindow ui;
	QString m_ProtocolFolder = QCoreApplication::applicationDirPath() + "/Resources/Config/Prov";
};

#endif
