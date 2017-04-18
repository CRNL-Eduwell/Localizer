#include "optionsPerf.h"

optionsPerf::optionsPerf(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connectSignals();
	readList();
}

optionsPerf::~optionsPerf()
{

}

void optionsPerf::getPerfLoca(vector<QString> &locaPerf)
{
	locaPerf = vector<QString>(wantedLoca);
}

void optionsPerf::connectSignals()
{
	connect(ui.addProv, SIGNAL(clicked()), this, SLOT(openProvWindow()));
	connect(ui.removeProv, SIGNAL(clicked()), this, SLOT(deleteProvFromList()));
	connect(ui.ValidatePushButton, SIGNAL(clicked()), this, SLOT(saveListandClose()));
}

void optionsPerf::displayLoca(vector<QString> wantedLoca)
{
	ui.listProv->clear();

	for (int i = 0; i < wantedLoca.size(); i++)
	{
		QListWidgetItem *currentPROV = new QListWidgetItem(ui.listProv);
		currentPROV->setText(wantedLoca[i]);
	}
}

void optionsPerf::readList()
{
	wantedLoca.clear();

	vector<string> locaFromFile = readTxtFile(perfFilePath.toStdString());
	for (int i = 0; i < locaFromFile.size(); i++)
	{
		wantedLoca.push_back(locaFromFile[i].c_str());
	}
	displayLoca(wantedLoca);
}

void optionsPerf::openProvWindow()
{
	prov = new displayProv();
	connect(prov, SIGNAL(sendProvList(vector<QString>)), this, SLOT(getProvList(vector<QString>)));
	prov->exec();
	delete prov;
	displayLoca(wantedLoca);
}

void optionsPerf::getProvList(vector<QString> provList)
{
	wantedLoca = vector<QString>(provList);
}

void optionsPerf::deleteProvFromList()
{
	QModelIndexList listIdSelected = ui.listProv->selectionModel()->selectedIndexes();

	if (listIdSelected.size() > 0)
	{
		int rowToDelete = listIdSelected[0].row();
		ui.listProv->item(rowToDelete)->~QListWidgetItem();
		wantedLoca.erase(wantedLoca.begin() + rowToDelete);
	}
}

void optionsPerf::saveListandClose()
{
	ofstream fichierPerf(perfFilePath.toStdString(), ios::out);
	for (int i = 0; i < wantedLoca.size(); i++)
	{
		fichierPerf << wantedLoca[i].toStdString() << endl;
	}
	fichierPerf.close();
	close();
}