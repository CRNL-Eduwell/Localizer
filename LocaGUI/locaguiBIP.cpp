#include "locaguiBIP.h"

LocaGUIBIP::LocaGUIBIP(InsermLibrary::ELAN *p_elan, QWidget *parent) : QDialog(parent)
{
	elan = p_elan;

	ui.setupUi(this);
	fillGUIBipoles();
	connectAllBordel();
}

LocaGUIBIP::~LocaGUIBIP()
{

}

void LocaGUIBIP::fillGUIBipoles()
{
	for (int i = 0; i < elan->trc->nameElectrodePositiv.size(); i++)
	{
		QListWidgetItem *item;
		item = new QListWidgetItem(ui.listWidget);
		item->setText(elan->trc->nameElectrodePositiv[i].c_str());
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
		item->setCheckState(Qt::Checked); // AND initialize check state
		ui.listWidget->addItem(item);

	}
}

void LocaGUIBIP::connectAllBordel()
{
	QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(createBipoles()));
	//QObject::connect(ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(checkMulti()));
	QObject::connect(ui.listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(checkMulti(QListWidgetItem *)));
}

void LocaGUIBIP::createBipoles()
{
	//std::vector<int> index_supp;

	for (int i = 0; i < elan->trc->nameElectrodePositiv.size(); i++)
	{
		if (ui.listWidget->item(i)->checkState() == false)
		{
			elan->index_supp.push_back(i);
		}
	}

	//index_supp.push_back(63);
	//index_supp.push_back(64);
	//index_supp.push_back(127);
	//index_supp.push_back(128);
	//index_supp.push_back(129);

	//for (int i = 179; i < elan->trc->nameElectrodePositiv.size(); i++)
	//{
	//	index_supp.push_back(i);
	//}

	//On enlève les éléc qu'on veut pas																																																  //
	int pos = std::find(elan->index_supp.begin(), elan->index_supp.end(), 0) - elan->index_supp.begin();																																				  //
	pos = pos - 1;																																																					  //

	if (pos < elan->index_supp.size())																																																	  //
	{																																																								  //
		//array sort puis on vide																																																	  //
		sort(elan->index_supp.begin(), elan->index_supp.end());																																													  //
		for (int i = elan->index_supp.size() - 1; i >= 0; i--)																																											  //
		{																																																							  //
			elan->trc->nameElectrodePositiv.erase(elan->trc->nameElectrodePositiv.begin() + elan->index_supp[i]);																																		  //
			elan->trc->signalPosition.erase(elan->trc->signalPosition.begin() + elan->index_supp[i]);																																					  //
			elan->trc->nameElectrodeNegativ.erase(elan->trc->nameElectrodeNegativ.begin() + elan->index_supp[i]);																																		  //
			elan->trc->eegData.erase(elan->trc->eegData.begin() + elan->index_supp[i]);
		}																																																							  //
	}																																																								  //

	//elan->eeg_loc_montage(elan->trc->nameElectrodePositiv, elan->trc->signalPosition);
	//accept();
	close();
}

void LocaGUIBIP::checkMulti(QListWidgetItem * item)
{
	QList<QListWidgetItem *> a = ui.listWidget->selectedItems();

	for (int i = 0; i < a.size(); i++)
	{
		a.at(i)->setCheckState(item->checkState());
	}
}