#ifndef LOCAGUIOPT_H
#define LOCAGUIOPT_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include "ui_locaguiOpt.h"
#include "LOCA.h"

using namespace InsermLibrary;

class locaguiOpt : public QDialog	
{
	Q_OBJECT

public:
	locaguiOpt(QWidget *parent = 0);
	~locaguiOpt();

public slots :
	void createOptionStruct();
	void adjustSizeRatioWidth2Heigth();
	void adjustSizeRatioHeigth2Width();
	void wilcoxonOptions();
	void FDRwilcoxonOptions();
	void pValueWilcoxon();
	void kruskallOptions();
	void FDRkruskallOptions();
	void pValueKruskall();

signals:
	void sendOptMenu(OptionLOCA *);

public:
	OptionLOCA *optionLOCAOpt;
private:
	Ui::FormForm form;
	int memoryWidth , memoryHeigth;
};
#endif // LOCAGUIOPT_H