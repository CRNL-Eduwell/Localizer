#ifndef CHOOSEELEC_H
#define CHOOSEELEC_H

#include <QtWidgets/QDialog>
#include "ui_chooseelec.h"
#include "eegContainer.h"

class chooseElec : public QDialog
{
	Q_OBJECT

public:
	chooseElec(eegContainer* eegCont, QWidget *parent = 0);
	~chooseElec();
	void fillUIBipoles();
	void connectSignals();

public slots :
	void createBipoles();
	void checkMultipleItems(QListWidgetItem * item);
	void checkOnEnter(QListWidgetItem * item);

private:
	eegContainer* containerEeg = nullptr;
	Ui::FormElec ui;
};

#endif
