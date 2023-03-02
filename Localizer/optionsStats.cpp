#include "optionsStats.h"

optionsStats::optionsStats(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
}

optionsStats::~optionsStats()
{

}

InsermLibrary::statOption optionsStats::getStatOption()
{
    InsermLibrary::statOption statOpt;
	statOpt.kruskall = ui.pCheckBoxKW->isChecked();
	statOpt.FDRkruskall = ui.FDRCheckBoxKW->isChecked();
    statOpt.pKruskall = ui.pValueLE_KW->text().toFloat();
    //===
    statOpt.wilcoxon = ui.pCheckBoxWil->isChecked();
	statOpt.FDRwilcoxon = ui.FDRCheckBoxWil->isChecked();
    statOpt.pWilcoxon = ui.pValueLE_Wil->text().toFloat();
    //===
    statOpt.AverageTrials = ui.AverageTrialsCheckBox->isChecked();
    //===
    return statOpt;
}

void optionsStats::connectSignals()
{
    connect(ui.pValueLE_KW, &QLineEdit::editingFinished, this, &optionsStats::pValueKruskall);
    connect(ui.pCheckBoxKW, &QPushButton::clicked, this, &optionsStats::updateKWOpt);
    connect(ui.pValueLE_Wil, &QLineEdit::editingFinished, this, &optionsStats::pValueWilcoxon);
    connect(ui.pCheckBoxWil, &QPushButton::clicked, this, &optionsStats::updateWilOpt);
    connect(ui.okPushButton, &QPushButton::clicked, this, [&]{ close(); });
}

void optionsStats::pValueKruskall()
{
    QLocale locale(QLocale::English, QLocale::UnitedStates);

    bool parseOk = false;
    float value = locale.toFloat(ui.pValueLE_KW->text(), &parseOk);
    if(parseOk)
    {
        if(value <= 0.0f)
        {
            ui.pValueLE_KW->setText("0.05");
        }
    }
    else //probably using , separator for decimal numbers
    {
        ui.pValueLE_KW->setText(ui.pValueLE_KW->text().replace(QString(","), QString(".")));
        float value = locale.toFloat(ui.pValueLE_KW->text(), &parseOk);
        if(value <= 0.0f)
        {
            ui.pValueLE_KW->setText("0.05");
        }
    }
}

void optionsStats::pValueWilcoxon()
{
    QLocale locale(QLocale::English, QLocale::UnitedStates);

    bool parseOk = false;
    float value = locale.toFloat(ui.pValueLE_Wil->text(), &parseOk);
    if(parseOk)
    {
        if(value <= 0.0f)
        {
            ui.pValueLE_Wil->setText("0.05");
        }
    }
    else //probably using , separator for decimal numbers
    {
        ui.pValueLE_Wil->setText(ui.pValueLE_Wil->text().replace(QString(","), QString(".")));
        float value = locale.toFloat(ui.pValueLE_Wil->text(), &parseOk);
        if(value <= 0.0f)
        {
            ui.pValueLE_Wil->setText("0.05");
        }
    }
}

void optionsStats::updateWilOpt()
{
	if (ui.pCheckBoxWil->isChecked() == false)
	{
		ui.FDRCheckBoxWil->setChecked(false);
	}

	if (ui.FDRCheckBoxWil->isChecked())
	{
		ui.pCheckBoxWil->setChecked(true);
	}
}

void optionsStats::updateKWOpt()
{
	if (ui.pCheckBoxKW->isChecked() == false)
	{
		ui.FDRCheckBoxKW->setChecked(false);
	}

	if (ui.FDRCheckBoxKW->isChecked())
	{
		ui.pCheckBoxKW->setChecked(true);
	}
}
