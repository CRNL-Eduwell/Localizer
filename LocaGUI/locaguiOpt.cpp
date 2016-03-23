#include "locaguiOpt.h"

locaguiOpt::locaguiOpt(QWidget *parent) : QDialog(parent)
{
	form.setupUi(this);

	QObject::connect(form.pushButtonOK, SIGNAL(clicked()), this, SLOT(createOptionStruct()));
	qRegisterMetaType<InsermLibrary::ELAN*>("InsermLibrary::::OptionLOCA*");
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
	optionLOCAOpt->statsOption.useKruskall = form.KruskallcheckBox->isChecked();
	optionLOCAOpt->statsOption.useFDRKrus = form.FDRKcheckBox->isChecked();

	emit sendOptMenu(optionLOCAOpt);
	close();
}