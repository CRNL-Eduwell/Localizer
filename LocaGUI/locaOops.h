#ifndef LOCAOOPS_H
#define LOCAOOPS_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>

#include "ui_locaOops.h"

class locaOops : public QDialog
{
	Q_OBJECT

public:
	locaOops(QWidget *parent = 0);
	~locaOops();

	Ui::Dialog d;
};

#endif 