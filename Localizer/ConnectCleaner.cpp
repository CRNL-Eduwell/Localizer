#include "ConnectCleaner.h"

ConnectCleaner::ConnectCleaner(InsermLibrary::eegContainer* eegCont, QWidget *parent) : QDialog(parent)
{
	m_ElectrodesLabel = std::vector<std::string>(eegCont->flatElectrodes);

	ui.setupUi(this);
	//FillSelectedList(m_ElectrodesLabel);
	FillCorrectedList(m_ElectrodesLabel);
}

ConnectCleaner::~ConnectCleaner()
{

}

void ConnectCleaner::CheckMultipleItems(QListWidgetItem * item)
{
	QList<QListWidgetItem *> selectedItems = item->listWidget()->selectedItems();
	for (int i = 0; i < selectedItems.size(); i++)
	{
		selectedItems.at(i)->setCheckState(item->checkState());
	}
}

void ConnectCleaner::CheckOnEnter(QListWidgetItem * item)
{
	if (Qt::Key::Key_Return)
	{
		Qt::CheckState state = item->checkState();
		if (state == Qt::CheckState::Checked)
		{
			item->setCheckState(Qt::CheckState::Unchecked);
		}
		else
		{
			item->setCheckState(Qt::CheckState::Checked);
		}
	}
}

void ConnectCleaner::FillSelectedList(const std::vector<std::string> & labels)
{
	int LabelCount = m_ElectrodesLabel.size();
	for (int i = 0; i < LabelCount; i++)
	{
		QListWidgetItem *item = new QListWidgetItem(ui.SelectElectrodeList);
		item->setText(m_ElectrodesLabel[i].c_str());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		ui.SelectElectrodeList->addItem(item);
	}
}

void ConnectCleaner::FillCorrectedList(const std::vector<std::string> & labels)
{
	int LabelCount = m_ElectrodesLabel.size();
	for (int i = 0; i < LabelCount; i++)
	{
		QListWidgetItem *item = new QListWidgetItem(ui.SelectElectrodeList);
		ui.SelectElectrodeList->addItem(item);

		ElectrodeCleanerEdit *lineEditItem = new ElectrodeCleanerEdit();
		ui.SelectElectrodeList->setItemWidget(item, lineEditItem);
		item->setSizeHint(QSize(item->sizeHint().width(), lineEditItem->sizeHint().height()));
	}
}

void ConnectCleaner::ConnectSignals()
{
	connect(ui.SelectElectrodeList, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(checkMultipleItems(QListWidgetItem *)));
	connect(ui.SelectElectrodeList, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(checkOnEnter(QListWidgetItem *)));
}