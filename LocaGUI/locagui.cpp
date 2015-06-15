#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qcheckbox.h>

#include <windows.h>
#include <fstream>
#include <vector>
#include "locagui.h"

LocaGUI::LocaGUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setUpGUI(this);
	connectAllBordel();
	//testTextShell();
}

LocaGUI::~LocaGUI()
{

}

//Methods
void LocaGUI::setUpGUI(QMainWindow* QTGUIClass)
{
	/*               Frequency Widget               */
	readFreqFile("D:\\Users\\Florian\\Documents\\Arbeit\\INSERM\\Temp Files\\Localizer\\frequencyBand.txt");
	freqTAB = new QWidget*[freqBandName.size()];
	freqCheckBox = new QCheckBox**[freqBandName.size()];
	for (int i = 0; i < freqBandName.size(); i++)
	{
		freqTAB[i] = new QWidget();
		freqTAB[i]->setObjectName(QString(freqBandName[i].c_str()));
		freqTAB[i]->setEnabled(true);

		freqCheckBox[i] = new QCheckBox*[5];
		for (int j = 0; j < 5; j++)
		{
			freqCheckBox[i][j] = new QCheckBox(freqTAB[i]);
			switch (j)
			{
			case 0:	freqCheckBox[i][j]->setText("Analyse this Frequency ?");
				break;
			case 1: freqCheckBox[i][j]->setText("eeg2erp ");
				break;
			case 2: freqCheckBox[i][j]->setText("eeg2env");
				break;
			case 3: freqCheckBox[i][j]->setText("env2plot");
				break;
			case 4: freqCheckBox[i][j]->setText("trialmat");
				break;
			}

			freqCheckBox[i][j]->setObjectName(QString("checkBox").append(i).append(j));
			freqCheckBox[i][j]->setProperty("X :", i);
			freqCheckBox[i][j]->setProperty("Y :", i);
			freqCheckBox[i][j]->setGeometry(QRect(10, 15 + (20 * j), 150, 20));
		}
		ui.frequencyTabWidget->addTab(freqTAB[i], QString(freqBandName[i].c_str()));
	}
	/*************************************************/
}

void LocaGUI::readFreqFile(std::string p_pathFreq)
{
	char buffer[256];
	int count = 0;

	std::ifstream fichierFreq(p_pathFreq, std::ios::out);  // patient.pos with data for samplingFrequency										  
	while (fichierFreq.getline(buffer, 256))
	{
		switch (count % 2)
		{
		case 0:
			freqBandName.push_back(buffer);
			count++;
			break;
		case 1:
			std::vector<int> tempFreqVal;

			std::vector<std::string> splitValue = split<std::string>(buffer, ":");
			int fMin = atoi(splitValue[0].c_str());
			int step = atoi(splitValue[1].c_str());
			int fMax = atoi(splitValue[2].c_str());

			for (int i = 0; i <= step; i++)
			{
				tempFreqVal.push_back(fMin + (i * step));
			}
			freqBandValue.push_back(tempFreqVal);
			count++;
			break;
		}
	}
}

template<typename T> std::vector<T> LocaGUI::split(const T & str, const T & delimiters)
{
	/*Exemple : vector<string> v = split<string>("Hello, there; World", ";,"); */
	std::vector<T> v;
	T::size_type start = 0;
	auto pos = str.find_first_of(delimiters, start);
	while (pos != T::npos) {
		if (pos != start) // ignore empty tokens																																															  
			v.emplace_back(str, start, pos - start);
		start = pos + 1;
		pos = str.find_first_of(delimiters, start);
	}
	if (start < str.length()) // ignore trailing delimiter																																													  
		v.emplace_back(str, start, str.length() - start); // add what's left of the string																																					  
	return v;
}

void LocaGUI::connectAllBordel()
{
	QObject::connect(ui.browseButton, SIGNAL(clicked()), this, SLOT(browsePatient()));

	for (int i = 0; i < freqBandName.size(); i++)
	{
		QObject::connect(freqCheckBox[i][0], SIGNAL(clicked(bool)), this, SLOT(FreqBandCheck(bool)));
	}

	QObject::connect(ui.addTRCButton, SIGNAL(clicked()), this, SLOT(addTRC2List()));
	QObject::connect(ui.removeTRCButton, SIGNAL(clicked()), this, SLOT(removeTRC2List()));

}

void LocaGUI::loadTRCFromFolder(std::string p_pathFolder)
{
	std::string locaFolder;

	/****************************** Free Memory if reloading TRC again ********************************/																																	  //		
	if (trcList.size() != 0)																	  /*||*/																																	  //		
	{																							  /*||*/																																	  //	
		for (int i = 0; i < trcList.size(); i++)												  /*||*/																																	  //	
		{																						  /*||*/																																	  //	
			std::vector<std::string>().swap(trcList[i]);										  /*||*/																																	  //	
		}																						  /*||*/																																	  //	
		std::vector<int>().swap(indexTRCList);													  /*||*/																																	  //	
		std::vector<std::string>().swap(directoryList);											  /*||*/																																	  //	
		std::vector<std::vector<std::string>>().swap(trcList);									  /*||*/																																	  //	
	}																							  /*||*/																																	  //	
	/**************************************************************************************************/

	//Get every folder corresponding to one LOCALIZER exam
	QDir currentDir(p_pathFolder.c_str());
	currentDir.setFilter(QDir::Dirs);

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != tr(".") && dirname != tr(".."))
		{
			directoryList.push_back(dirname.toStdString());
		}
	}

	//In everyone of this folder retrieve the .TRC files to load them in the GUI
	for (int i = 0; i < directoryList.size(); i++)
	{
		locaFolder = p_pathFolder + "/" + directoryList[i];

		QDir currentDir2(locaFolder.c_str());
		currentDir2.setNameFilters(QStringList() << "*.TRC");

		QStringList trcFile = currentDir2.entryList();
		std::vector<std::string> trcFromLoca;
		for (int j = 0; j < trcFile.size(); j++)
		{
			trcFromLoca.push_back(trcFile[j].toStdString());
		}
		trcList.push_back(trcFromLoca);
	}

	loadTRCListWidget(trcList);
}

void LocaGUI::loadTRCListWidget(std::vector<std::vector<std::string>> p_trcList)
{
	int fullSize = 0, actualIndex = 0, numberItem = 0;

	for (int i = 0; i < p_trcList.size(); i++)
	{
		fullSize += p_trcList[i].size();
	}

	if (listTRCWidget == nullptr)
	{
		listTRCWidget = new QListWidgetItem*[fullSize];
		for (int i = 0; i < p_trcList.size(); i++)
		{
			for (int j = 0; j < p_trcList[i].size(); j++)
			{
				actualIndex += j;
				listTRCWidget[i] = new QListWidgetItem(ui.TRCListWidget);
				listTRCWidget[i]->setText(p_trcList[i][j].c_str());
			}
		}
	}
	else
	{
		/****************************** Free Memory if reloading TRC again ********************************/
		for (int i = 0; i < ui.TRCListWidget->count(); i++)											  /*||*/
		{																							  /*||*/
			listTRCWidget[i]->~QListWidgetItem();													  /*||*/
		}																							  /*||*/
		delete[] listTRCWidget;																		  /*||*/
		/*||*/
		ui.TRCListWidget->clear();																	  /*||*/
		ui.chosenTRClistWidget->clear();															  /*||*/
		/**************************************************************************************************/

		listTRCWidget = new QListWidgetItem*[fullSize];
		for (int i = 0; i < p_trcList.size(); i++)
		{
			for (int j = 0; j < p_trcList[i].size(); j++)
			{
				actualIndex += j;
				listTRCWidget[i] = new QListWidgetItem(ui.TRCListWidget);
				listTRCWidget[i]->setText(p_trcList[i][j].c_str());
			}
		}
	}
}

void LocaGUI::testTextShell()
{
	for (int i = 0; i < 15; i++)
	{
		ui.shellTextBrowser->append(QString("Test"));
	}
}
//Slots
void LocaGUI::browsePatient()
{
	QFileDialog *fileDial = new QFileDialog(this);
	fileDial->setFileMode(QFileDialog::FileMode::DirectoryOnly);
	QString fileName = fileDial->getExistingDirectory(this, tr("Choose Patient Folder"));
	ui.patientPathlineEdit->setText(fileName);
	loadTRCFromFolder(fileName.toStdString());
}

void LocaGUI::FreqBandCheck(bool isChecked)
{
	int indexClicked = sender()->property("X :").toInt();

	if (isChecked)
	{
		freqCheckBox[indexClicked][1]->setChecked(true);
		freqCheckBox[indexClicked][2]->setChecked(true);
		freqCheckBox[indexClicked][3]->setChecked(true);
		freqCheckBox[indexClicked][4]->setChecked(true);
	}
	else
	{
		freqCheckBox[indexClicked][1]->setChecked(false);
		freqCheckBox[indexClicked][2]->setChecked(false);
		freqCheckBox[indexClicked][3]->setChecked(false);
		freqCheckBox[indexClicked][4]->setChecked(false);
	}
}

void LocaGUI::addTRC2List()
{
	QListWidgetItem *itemToAdd;
	itemToAdd = new QListWidgetItem(*ui.TRCListWidget->currentItem());
	ui.chosenTRClistWidget->addItem(itemToAdd);
	indexTRCList.push_back(ui.TRCListWidget->currentIndex().row());
	ui.TRCListWidget->currentItem()->setHidden(true);

	std::sort(indexTRCList.begin(), indexTRCList.end());
	ui.chosenTRClistWidget->sortItems(Qt::SortOrder::AscendingOrder);
}

void LocaGUI::removeTRC2List()
{
	int rowIndex = ui.chosenTRClistWidget->selectionModel()->currentIndex().row();

	if (ui.TRCListWidget->item(indexTRCList[rowIndex])->isHidden() == true)
	{
		ui.TRCListWidget->item(indexTRCList[rowIndex])->setHidden(false);
	}

	indexTRCList.erase(indexTRCList.begin() + ui.chosenTRClistWidget->currentIndex().row());
	ui.chosenTRClistWidget->currentItem()->~QListWidgetItem();

	std::sort(indexTRCList.begin(), indexTRCList.end());
	ui.TRCListWidget->sortItems(Qt::SortOrder::AscendingOrder);
}

