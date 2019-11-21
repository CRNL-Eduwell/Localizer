#include "optionsStats.h"

using namespace InsermLibrary;

optionsStats::optionsStats(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
	readList();
}

optionsStats::~optionsStats()
{

}

statOption optionsStats::getStatOption()
{
	statOption statOpt;
	statOpt.kruskall = ui.pCheckBoxKW->isChecked();
	statOpt.FDRkruskall = ui.FDRCheckBoxKW->isChecked();
	statOpt.pKruskall = atof(&ui.pValueLE_KW->text().toStdString()[0]);
	statOpt.locaKruskall = std::vector<QString>(wantedLocaKW);
	statOpt.wilcoxon = ui.pCheckBoxWil->isChecked();
	statOpt.FDRwilcoxon = ui.FDRCheckBoxWil->isChecked();
	statOpt.pWilcoxon = atof(&ui.pValueLE_Wil->text().toStdString()[0]);
	statOpt.locaWilcoxon = std::vector<QString>(wantedLocaWil);
	return statOpt;
}

void optionsStats::connectSignals()
{
	connect(ui.addKW, &QPushButton::clicked, this, [&] { toggle = true; });
	connect(ui.removeKW, &QPushButton::clicked, this, [&] { toggle = true; });
    connect(ui.addKW, &QPushButton::clicked, this, &optionsStats::openProvWindow);
    connect(ui.removeKW, &QPushButton::clicked, this, &optionsStats::deleteProvFromList);
    connect(ui.pValueLE_KW, &QLineEdit::editingFinished, this, &optionsStats::pValueKruskall);
    connect(ui.pCheckBoxKW, &QPushButton::clicked, this, &optionsStats::updateKWOpt);
	//==
	connect(ui.addtestWil, &QPushButton::clicked, this, [&] { toggle = false; });
	connect(ui.removeWil, &QPushButton::clicked, this, [&] { toggle = false; });
    connect(ui.addtestWil, &QPushButton::clicked, this, &optionsStats::openProvWindow);
    connect(ui.removeWil, &QPushButton::clicked, this, &optionsStats::deleteProvFromList);
    connect(ui.pValueLE_Wil, &QLineEdit::editingFinished, this, &optionsStats::pValueWilcoxon);
    connect(ui.pCheckBoxWil, &QPushButton::clicked, this, &optionsStats::updateWilOpt);
	//==
	connect(ui.okPushButton, &QPushButton::clicked, this, &optionsStats::saveListsandClose);
}

void optionsStats::displayLoca(QListWidget *uiList, std::vector<QString> wantedLoca)
{
	uiList->clear();

	for (int i = 0; i < wantedLoca.size(); i++)
	{
		QListWidgetItem *currentPROV = new QListWidgetItem(uiList);
		currentPROV->setText(wantedLoca[i]);
	}
}

void optionsStats::readList()
{
	wantedLocaKW.clear();
	wantedLocaWil.clear();

	std::vector<std::string> locaFromFileKW = readTxtFile(kwFilePath.toStdString());
	for (int i = 0; i < locaFromFileKW.size(); i++)
	{
		wantedLocaKW.push_back(locaFromFileKW[i].c_str());
	}

	std::vector<std::string> locaFromFileWil = readTxtFile(wilFilePath.toStdString());
	for (int i = 0; i < locaFromFileWil.size(); i++)
	{
		wantedLocaWil.push_back(locaFromFileWil[i].c_str());
	}
	
	displayLoca(ui.listKW, wantedLocaKW);
	displayLoca(ui.listWil, wantedLocaWil);
}

void optionsStats::openProvWindow()
{
	if (toggle)
	{
		prov = new displayProv(wantedLocaKW);
		connect(prov, &displayProv::sendProvList, this, &optionsStats::getProvList);
		prov->exec();
		delete prov;
		displayLoca(ui.listKW, wantedLocaKW);
	}
	else
	{
		prov = new displayProv(wantedLocaWil);
		connect(prov, &displayProv::sendProvList, this, &optionsStats::getProvList);
		prov->exec();
		delete prov;
		displayLoca(ui.listWil, wantedLocaWil);
	}
}

void optionsStats::deleteProvFromList()
{
	QModelIndexList listIdSelected;
	
	if (toggle)
	{
		listIdSelected = ui.listKW->selectionModel()->selectedIndexes();
		if (listIdSelected.size() > 0)
		{
			int rowToDelete = listIdSelected[0].row();
			ui.listKW->item(rowToDelete)->~QListWidgetItem();
			wantedLocaKW.erase(wantedLocaKW.begin() + rowToDelete);
		}
	}
	else
	{
		listIdSelected = ui.listWil->selectionModel()->selectedIndexes();
		if (listIdSelected.size() > 0)
		{
			int rowToDelete = listIdSelected[0].row();
			ui.listWil->item(rowToDelete)->~QListWidgetItem();
			wantedLocaWil.erase(wantedLocaWil.begin() + rowToDelete);
		}
	}

}

void optionsStats::getProvList(std::vector<QString> provList)
{
	if(toggle)
		wantedLocaKW = std::vector<QString>(provList);
	else
		wantedLocaWil = std::vector<QString>(provList);
}

void optionsStats::saveListsandClose()
{
	saveTxtFile(wantedLocaKW, kwFilePath.toStdString());
	saveTxtFile(wantedLocaWil, wilFilePath.toStdString());
	close();
}

void optionsStats::pValueKruskall()
{
	ui.pValueLE_KW->setText(ui.pValueLE_KW->text().replace(QString(","), QString(".")));
	if (atof(&ui.pValueLE_KW->text().toStdString()[0]) <= 0)
	{
		ui.pValueLE_KW->setText("0.01");
	}
}

void optionsStats::pValueWilcoxon()
{
	ui.pValueLE_Wil->setText(ui.pValueLE_Wil->text().replace(QString(","), QString(".")));
	if (atof(&ui.pValueLE_Wil->text().toStdString()[0]) <= 0)
	{
		ui.pValueLE_Wil->setText("0.01");
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
