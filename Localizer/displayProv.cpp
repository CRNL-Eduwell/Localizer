#include "displayProv.h"

displayProv::displayProv(std::vector<QString> currentList, QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	getProvList();
	displayProvList(currentList);
	connectSignals();
}

displayProv::~displayProv()
{

}

void displayProv::getProvList()
{
	prov.clear();

    QString rootProvFolder(QCoreApplication::applicationDirPath() + "/Resources/Config/Prov");

	QDir currentDir(rootProvFolder);
	currentDir.setFilter(QDir::Files);
	currentDir.setNameFilters(QStringList() << "*.prov");

	QStringList provList = currentDir.entryList();
	for (int i = 0; i < provList.size(); i++)
	{
		prov.push_back(QString(provList[i]).split(QRegExp(".prov"), QString::SplitBehavior::SkipEmptyParts).at(0));
	}
	prov.erase(unique(prov.begin(), prov.end()), prov.end());
}

void displayProv::displayProvList(std::vector<QString> currentList)
{
	ui.listWidget->clear();

	for (int i = 0; i < prov.size(); i++)
	{
		QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget);
		currentPROV->setText(prov[i]);
		currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsUserCheckable); // set checkable flag

		if(std::find(currentList.begin(), currentList.end(), prov[i]) != currentList.end())
			currentPROV->setCheckState(Qt::Checked);
		else
			currentPROV->setCheckState(Qt::Unchecked); // AND initialize check state
	}
}

void displayProv::connectSignals()
{
	connect(ui.listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(checkMultipleItems(QListWidgetItem *)));
	connect(ui.listWidget, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(checkOnEnter(QListWidgetItem *)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(sendListAndClose()));
}

void displayProv::checkMultipleItems(QListWidgetItem * item)
{
	QList<QListWidgetItem *> selecItem = item->listWidget()->selectedItems();

	for (int i = 0; i < selecItem.size(); i++)
	{
		selecItem.at(i)->setCheckState(item->checkState());
	}
}

void displayProv::checkOnEnter(QListWidgetItem * item)
{
	if (Qt::Key::Key_Return)
	{
		Qt::CheckState currentState = item->checkState();
		if (currentState == Qt::CheckState::Checked)
		{
			item->setCheckState(Qt::CheckState::Unchecked);
		}
		else
		{
			item->setCheckState(Qt::CheckState::Checked);
		}
	}
}

void displayProv::sendListAndClose()
{
	std::vector<QString> provWanted;
	for (int i = 0; i < prov.size(); i++)
	{
		if (ui.listWidget->item(i)->checkState() == Qt::CheckState::Checked)
		{
			provWanted.push_back(prov[i]);
		}
	}

	sendProvList(provWanted);
	close();
}
