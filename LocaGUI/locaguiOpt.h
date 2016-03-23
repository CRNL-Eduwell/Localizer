#ifndef LOCAGUIOPT_H
#define LOCAGUIOPT_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>

#include "ui_locaguiOpt.h"
#include "LOCA.h"

class locaguiOpt : public QDialog	
{
	Q_OBJECT

public:
	locaguiOpt(QWidget *parent = 0);
	~locaguiOpt();

public slots :
	void createOptionStruct();

signals:
	void sendOptMenu(InsermLibrary::OptionLOCA *);

public:
	InsermLibrary::OptionLOCA *optionLOCAOpt;
private:
	Ui::FormForm form;
};
#endif // LOCAGUIOPT_H