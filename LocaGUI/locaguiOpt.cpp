#include "locaguiOpt.h"
#include <QString>

locaguiOpt::locaguiOpt(QWidget *parent) : QDialog(parent)
{
	form.setupUi(this);

	connect(form.WidthlineEdit, SIGNAL(editingFinished()), this, SLOT(adjustSizeRatioWidth2Heigth()));
	connect(form.HeigthlineEdit, SIGNAL(editingFinished()), this, SLOT(adjustSizeRatioHeigth2Width()));

	connect(form.WilcoxoncheckBox, SIGNAL(clicked()), this, SLOT(wilcoxonOptions()));
	connect(form.FDRWcheckBox, SIGNAL(clicked()), this, SLOT(FDRwilcoxonOptions()));
	connect(form.WilcoxonLineEdit, SIGNAL(editingFinished()), this, SLOT(pValueWilcoxon()));

	connect(form.KruskallcheckBox, SIGNAL(clicked()), this, SLOT(kruskallOptions()));
	connect(form.FDRKcheckBox, SIGNAL(clicked()), this, SLOT(FDRkruskallOptions()));
	connect(form.KruskallLineEdit, SIGNAL(editingFinished()), this, SLOT(pValueKruskall()));

	connect(form.pushButtonOK, SIGNAL(clicked()), this, SLOT(createOptionStruct()));
	qRegisterMetaType<OptionLOCA *>("OptionLOCA *");
}

locaguiOpt::~locaguiOpt()
{

}

void locaguiOpt::createOptionStruct()
{
	optionLOCAOpt = new InsermLibrary::OptionLOCA(); //Dans loca.h

	optionLOCAOpt->picOption.height = atoi(&form.HeigthlineEdit->text().toStdString()[0]);
	optionLOCAOpt->picOption.width = atoi(&form.WidthlineEdit->text().toStdString()[0]);

	optionLOCAOpt->picOption.interpolationFactorX = atoi(&form.HorizonINTERPlineEdit->text().toStdString()[0]);
	if (optionLOCAOpt->picOption.interpolationFactorX < 1)
		optionLOCAOpt->picOption.interpolationFactorX = 1;

	optionLOCAOpt->picOption.interpolationFactorY = atoi(&form.VerticalINTERPlineEdit->text().toStdString()[0]);
	if (optionLOCAOpt->picOption.interpolationFactorY < 1)
		optionLOCAOpt->picOption.interpolationFactorY = 1;

	optionLOCAOpt->statsOption.useWilcoxon = form.WilcoxoncheckBox->isChecked();
	optionLOCAOpt->statsOption.useFDRWil = form.FDRWcheckBox->isChecked();
	optionLOCAOpt->statsOption.pWilcoxon = atof(&form.WilcoxonLineEdit->text().toStdString()[0]);

	optionLOCAOpt->statsOption.useKruskall = form.KruskallcheckBox->isChecked();
	optionLOCAOpt->statsOption.useFDRKrus = form.FDRKcheckBox->isChecked();
	optionLOCAOpt->statsOption.pKruskall = atof(&form.KruskallLineEdit->text().toStdString()[0]);

	emit sendOptMenu(optionLOCAOpt);
	close();
}

void locaguiOpt::adjustSizeRatioWidth2Heigth()
{
	int width = atoi(form.WidthlineEdit->text().toStdString().c_str());
	if (memoryWidth != width)
	{
		memoryWidth = width;
		int heigth = ceil(0.75 * width);
		char buffer[33];
		itoa(heigth, buffer, 10);
		form.HeigthlineEdit->setText(buffer);
	}
}

void locaguiOpt::adjustSizeRatioHeigth2Width()
{
	int heigth = atoi(form.HeigthlineEdit->text().toStdString().c_str());
	if (memoryHeigth != heigth)
	{
		memoryHeigth = heigth;
		int width = ceil(1.333 * heigth);
		char buffer[33];
		itoa(width, buffer, 10);
		form.WidthlineEdit->setText(buffer);
	}
}

void locaguiOpt::wilcoxonOptions()
{
	if (form.WilcoxoncheckBox->isChecked() == false)
	{
		form.FDRWcheckBox->setChecked(false);
	}
}

void locaguiOpt::FDRwilcoxonOptions()
{
	if (form.FDRWcheckBox->isChecked())
	{
		form.WilcoxoncheckBox->setChecked(true);
	}
}

void locaguiOpt::pValueWilcoxon()
{
	form.WilcoxonLineEdit->setText(form.WilcoxonLineEdit->text().replace(QString(","), QString(".")));
	if (atof(&form.WilcoxonLineEdit->text().toStdString()[0]) <= 0)
	{
		form.WilcoxonLineEdit->setText("0.01");
	}
}

void locaguiOpt::kruskallOptions()
{
	if (form.KruskallcheckBox->isChecked() == false)
	{
		form.FDRKcheckBox->setChecked(false);
	}
}

void locaguiOpt::FDRkruskallOptions()
{
	if (form.FDRKcheckBox->isChecked())
	{
		form.KruskallcheckBox->setChecked(true);
	}
}

void locaguiOpt::pValueKruskall()
{
	form.KruskallLineEdit->setText(form.KruskallLineEdit->text().replace(QString(","), QString(".")));
	if (atof(&form.KruskallLineEdit->text().toStdString()[0]) <= 0)
	{
		form.KruskallLineEdit->setText("0.01");
	}
}