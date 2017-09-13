#include "optionsStats.h"

optionsStats::optionsStats(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
	readList();
}

optionsStats::~optionsStats()
{

}

void optionsStats::getStatOption(statOption *statOpt)
{
	statOpt->kruskall = ui.pCheckBoxKW->isChecked();
	statOpt->FDRkruskall = ui.FDRCheckBoxKW->isChecked();
	statOpt->pKruskall = atof(&ui.pValueLE_KW->text().toStdString()[0]);
	statOpt->locaKruskall = vector<QString>(wantedLocaKW);
	statOpt->wilcoxon = ui.pCheckBoxWil->isChecked();
	statOpt->FDRwilcoxon = ui.FDRCheckBoxWil->isChecked();
	statOpt->pWilcoxon = atof(&ui.pValueLE_Wil->text().toStdString()[0]);
	statOpt->locaWilcoxon = vector<QString>(wantedLocaWil);
}

void optionsStats::connectSignals()
{
	connect(ui.addKW, &QPushButton::clicked, this, [&] { toggle = true; });
	connect(ui.removeKW, &QPushButton::clicked, this, [&] { toggle = true; });
	connect(ui.addKW, SIGNAL(clicked()), this, SLOT(openProvWindow()));
	connect(ui.removeKW, SIGNAL(clicked()), this, SLOT(deleteProvFromList()));
	connect(ui.pValueLE_KW, SIGNAL(editingFinished()), this, SLOT(pValueKruskall()));
	connect(ui.pCheckBoxKW, SIGNAL(clicked()), this, SLOT(updateKWOpt()));
	//==
	connect(ui.addtestWil, &QPushButton::clicked, this, [&] { toggle = false; });
	connect(ui.removeWil, &QPushButton::clicked, this, [&] { toggle = false; });
	connect(ui.addtestWil, SIGNAL(clicked()), this, SLOT(openProvWindow()));
	connect(ui.removeWil, SIGNAL(clicked()), this, SLOT(deleteProvFromList()));
	connect(ui.pValueLE_Wil, SIGNAL(editingFinished()), this, SLOT(pValueWilcoxon()));
	connect(ui.pCheckBoxWil, SIGNAL(clicked()), this, SLOT(updateWilOpt()));
	//==
	connect(ui.okPushButton, &QPushButton::clicked, this, &optionsStats::saveListsandClose);
}

void optionsStats::displayLoca(QListWidget *uiList, vector<QString> wantedLoca)
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

	vector<string> locaFromFileKW = readTxtFile(kwFilePath.toStdString());
	for (int i = 0; i < locaFromFileKW.size(); i++)
	{
		wantedLocaKW.push_back(locaFromFileKW[i].c_str());
	}

	vector<string> locaFromFileWil = readTxtFile(wilFilePath.toStdString());
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

void optionsStats::getProvList(vector<QString> provList)
{
	if(toggle)
		wantedLocaKW = vector<QString>(provList);
	else
		wantedLocaWil = vector<QString>(provList);
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