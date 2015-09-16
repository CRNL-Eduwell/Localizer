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
}

void LocaGUIBIP::createBipoles()
{
	std::vector<int> index_supp;

	for (int i = 0; i < elan->trc->nameElectrodePositiv.size(); i++)
	{
		if (ui.listWidget->item(i)->checkState() == false)
		{
			index_supp.push_back(i);
		}
	}

	index_supp.push_back(63);
	index_supp.push_back(64);
	index_supp.push_back(127);
	index_supp.push_back(128);
	index_supp.push_back(129);

	for (int i = 179; i < elan->trc->nameElectrodePositiv.size(); i++)
	{
		index_supp.push_back(i);
	}

	//On enlève les éléc qu'on veut pas																																																  //
	int pos = std::find(index_supp.begin(), index_supp.end(), 0) - index_supp.begin();																																				  //
	pos = pos - 1;																																																					  //

	if (pos < index_supp.size())																																																	  //
	{																																																								  //
		//array sort puis on vide																																																	  //
		sort(index_supp.begin(), index_supp.end());																																													  //
		for (int i = index_supp.size() - 1; i >= 0; i--)																																											  //
		{																																																							  //
			elan->trc->nameElectrodePositiv.erase(elan->trc->nameElectrodePositiv.begin() + index_supp[i]);																																		  //
			elan->trc->signalPosition.erase(elan->trc->signalPosition.begin() + index_supp[i]);																																					  //
			elan->trc->nameElectrodeNegativ.erase(elan->trc->nameElectrodeNegativ.begin() + index_supp[i]);																																		  //
			elan->trc->eegData.erase(elan->trc->eegData.begin() + index_supp[i]);
		}																																																							  //
	}																																																								  //

	//elan->eeg_loc_montage(elan->trc->nameElectrodePositiv, elan->trc->signalPosition);
	//accept();
	close();
}