#include "chooseElec.h"

chooseElec::chooseElec(eegContainer* eegCont, QWidget *parent) : QDialog(parent)
{
	containerEeg = eegCont;
	ui.setupUi(this);
	fillUIBipoles();
	connectSignals();
}

chooseElec::~chooseElec()
{

}

void chooseElec::fillUIBipoles()
{
	for (int i = 0; i < containerEeg->flatElectrodes.size(); i++)
	{
		QListWidgetItem *item;
		item = new QListWidgetItem(ui.listWidget);
		item->setText(containerEeg->flatElectrodes[i].c_str());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable); 
		item->setCheckState(Qt::Checked); 
		ui.listWidget->addItem(item);
	}
}

void chooseElec::connectSignals()
{
	connect(ui.listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(checkMultipleItems(QListWidgetItem *)));
	connect(ui.listWidget, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(checkOnEnter(QListWidgetItem *)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(createBipoles()));
}

void chooseElec::createBipoles()
{
	for (int i = 0; i < containerEeg->flatElectrodes.size(); i++)
	{
		if (ui.listWidget->item(i)->checkState() == false)
		{
			containerEeg->idElecToDelete.push_back(i);
		}
	}

	//containerEeg->deleteElectrodes(containerEeg->idElecToDelete);
	containerEeg = nullptr;
	done(1);
	close();
}

void chooseElec::checkMultipleItems(QListWidgetItem * item)
{
	QList<QListWidgetItem *> selecItem = item->listWidget()->selectedItems();

	for (int i = 0; i < selecItem.size(); i++)
	{
		selecItem.at(i)->setCheckState(item->checkState());
	}
}

void chooseElec::checkOnEnter(QListWidgetItem * item)
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