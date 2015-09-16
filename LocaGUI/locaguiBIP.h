#ifndef LOCAGUIBIP_H
#define LOCAGUIBIP_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qcheckbox.h>
#include <QKeyEvent>
#include <vector>

#include "ui_locaguiBIP.h"
#include "LOCA.h"

class LocaGUIBIP : public QDialog
{
	Q_OBJECT

public:
	LocaGUIBIP(InsermLibrary::ELAN *p_elan, QWidget *parent = 0);
	~LocaGUIBIP();
	void fillGUIBipoles();
	void connectAllBordel();

public slots :
	void createBipoles();

private:
	InsermLibrary::ELAN *elan;
	Ui::Form::Ui_Form ui;
};

#endif // LOCAGUIBIP_H