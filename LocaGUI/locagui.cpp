#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qcheckbox.h>

#include <QTimer>
#include <QThread>
#include <QProcess>

#include <windows.h>
#include <fstream>
#include <vector>
#include <sstream>

#include "locagui.h"
#include "locaguiBIP.h"
#include "locaguiOpt.h"
#include "locaOops.h"

//#include "vld.h"

LocaGUI::LocaGUI(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	setUpGUI(this);
	connectAllBordel();
}

LocaGUI::~LocaGUI()
{
	for (int i = 0; i < freqBandName.size(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			delete freqCheckBox[i][j];
		}
		delete freqCheckBox[i];
		delete freqTAB[i];
	}
	delete freqCheckBox;
	delete freqTAB;
	freqCheckBox = nullptr;
	freqTAB = nullptr;

	delete listTRCWidget;
	listTRCWidget = nullptr;

	//delete optionLOCAGUI;
}

//===========================================  Methods  =========================================== 
void LocaGUI::setUpGUI(QMainWindow* QTGUIClass)
{
	/*               Frequency Widget               */
	//readFreqFile("D:\\Users\\Florian\\Documents\\Arbeit\\INSERM\\Temp Files\\Localizer\\frequencyBand.txt");
	readFreqFile("C:\\System98\\ExternalProgram\\Localizer\\Config\\frequencyBand.txt");
	freqTAB = new QWidget*[freqBandName.size()];
	freqCheckBox = new QCheckBox**[freqBandName.size()];
	for (int i = 0; i < freqBandName.size(); i++)
	{
		freqTAB[i] = new QWidget();
		freqTAB[i]->setObjectName(QString(freqBandName[i].c_str()));
		freqTAB[i]->setEnabled(true);

		freqCheckBox[i] = new QCheckBox*[4];
		for (int j = 0; j < 4; j++)
		{
			freqCheckBox[i][j] = new QCheckBox(freqTAB[i]);
			switch (j)
			{
			case 0:	freqCheckBox[i][j]->setText("Analyse this Frequency ?");
				break;
			case 1: freqCheckBox[i][j]->setText("eeg2env");
				break;
			case 2: freqCheckBox[i][j]->setText("env2plot");
				break;
			case 3: freqCheckBox[i][j]->setText("trialmat");
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

	QAction* Options = ui.menuConfiguration->actions().at(0);
	QAction* About = ui.menuAide->actions().at(0);

	QObject::connect(Options, SIGNAL(triggered()), this, SLOT(openOptions()));
	QObject::connect(About, SIGNAL(triggered()), this, SLOT(openAbout()));
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
			std::vector<double> tempFreqVal;

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

	QObject::connect(ui.PROVListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(provClicked(QListWidgetItem *)));

	QObject::connect(ui.processButton, SIGNAL(clicked()), this, SLOT(launchAnalysis()));

	//QObject::connect(pointeur de la classe voulu, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLogInfo(QString));
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
		ui.TRCListWidget->clear();																	  /*||*/
		ui.chosenTRClistWidget->clear();															  /*||*/
		ui.PROVListWidget->clear();																	  /*||*/
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

void LocaGUI::addProv2List(std::string p_locaName)
{
	std::string actualLoca = "";
	std::stringstream provFileStream, provName;

	std::vector<std::string> locaSplit = split<std::string>(p_locaName, "_.");
	actualLoca = locaSplit[locaSplit.size() - 2];
	provFileStream.str(std::string());
	provFileStream.clear();
	provName.str(std::string());
	provName.clear();
	provFileStream << "C:\\System98\\ExternalProgram\\Localizer\\Config\\Prov\\" + actualLoca + ".prov";
	provName << actualLoca + ".prov";

	QFile provFile(provFileStream.str().c_str());
	//QListWidgetItem *itemToAdd = new QListWidgetItem(provName.str().c_str());
	QListWidgetItem *itemToAdd = new QListWidgetItem(provFileStream.str().c_str());

	if (provFile.exists())
	{
		itemToAdd->setBackgroundColor(Qt::green);
	}
	else
	{
		itemToAdd->setBackgroundColor(Qt::red);
	}
	ui.PROVListWidget->addItem(itemToAdd);
	ui.PROVListWidget->sortItems(Qt::SortOrder::AscendingOrder);
}

void LocaGUI::removeProv2List(int p_index)
{
	ui.PROVListWidget->item(p_index)->~QListWidgetItem();
	ui.PROVListWidget->sortItems(Qt::SortOrder::AscendingOrder);
}

//===========================================  Slots  =========================================== 
void LocaGUI::launchAnalysis()
{
	std::stringstream filePath, displayText;
	std::vector<std::string> trcFiles, provFiles, tasks, exp_tasks;

	if (alreadyRunning == false)
	{
		std::string patientFolder = ui.patientPathlineEdit->text().toStdString();
		for (int i = 0; i < indexTRCList.size(); i++)
		{
			std::stringstream().swap(filePath);

			trcFiles.push_back(ui.chosenTRClistWidget->item(i)->text().toStdString());
			provFiles.push_back(ui.PROVListWidget->item(i)->text().toStdString());

			std::vector<std::string> locaSplit = split<std::string>(trcFiles[i], "_.");

			tasks.push_back(locaSplit[locaSplit.size() - 2]);
			exp_tasks.push_back(split<std::string>(trcFiles[i], ".")[0]);
		}

		std::vector <std::vector<bool>> anaDetails;
		std::vector<bool> tempDetails;

		tempDetails.push_back(ui.ERPcheckBox->isChecked());
		anaDetails.push_back(tempDetails);

		for (int i = 0; i < freqBandName.size(); i++)
		{
			std::vector<bool>().swap(tempDetails);
			for (int j = 0; j < 4; j++)
			{
				if (freqCheckBox[i][j]->isChecked())
				{
					tempDetails.push_back(true);
				}
				else
				{
					tempDetails.push_back(false);
				}
			}
			anaDetails.push_back(tempDetails);
		}

		thread = new QThread;
		worker = new Worker(optionLOCAGUI, freqBandValue, anaDetails, trcFiles, provFiles, patientFolder, tasks, exp_tasks);

		//Function to return pointer to subclasses for message sending
		InsermLibrary::LOCA *test = worker->returnLoca();
		QObject::connect(test, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
		//======================================================================================
		QObject::connect(worker, SIGNAL(upScroll(int)), this, SLOT(upDateProgressBar(int)));       //connecte l'update dans le worker à la barre d'avancement GUI


		QObject::connect(thread, SIGNAL(started()), worker, SLOT(process()));
		QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
		QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
		QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

		QObject::connect(worker, SIGNAL(sendElanPointer(InsermLibrary::ELAN*)), this, SLOT(receiveElanPointer(InsermLibrary::ELAN*)));
		QObject::connect(worker, SIGNAL(sendLogInfo(QString)), this, SLOT(displayLog(QString)));
		QObject::connect(this, SIGNAL(bipDone(bool)), worker, SLOT(BipDoneeee(bool)));

		QObject::connect(worker, SIGNAL(finished()), this, SLOT(finishedRuning()));


		worker->moveToThread(thread);
		thread->start();
		alreadyRunning = true;
	}
	else
	{
		//QMessageBox::information(this, "Woops !", "Une analyse est déjà en cours !");

		locaOops *oops = new locaOops(0);
		oops->exec();
		delete oops;
	}
}

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
	}
	else
	{
		freqCheckBox[indexClicked][1]->setChecked(false);
		freqCheckBox[indexClicked][2]->setChecked(false);
		freqCheckBox[indexClicked][3]->setChecked(false);
	}
}

void LocaGUI::addTRC2List()
{
	QListWidgetItem *itemToAdd;
	if (ui.TRCListWidget->currentItem() != nullptr)
	{
		itemToAdd = new QListWidgetItem(*ui.TRCListWidget->currentItem());
		ui.chosenTRClistWidget->addItem(itemToAdd);
		indexTRCList.push_back(ui.TRCListWidget->currentIndex().row());
		ui.TRCListWidget->currentItem()->setHidden(true);

		std::sort(indexTRCList.begin(), indexTRCList.end());
		ui.chosenTRClistWidget->sortItems(Qt::SortOrder::AscendingOrder);

		addProv2List(itemToAdd->text().toStdString());
	}
	else
	{
		QMessageBox::warning(this, "Attention", "No TRC File selected");
	}
}

void LocaGUI::removeTRC2List()
{
	if (ui.chosenTRClistWidget->count() > 0)
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
		removeProv2List(rowIndex);
	}
}

void LocaGUI::provClicked(QListWidgetItem *provItem)
{ 
	QFileDialog *fileDialProv = new QFileDialog(this);
	fileDialProv->setFileMode(QFileDialog::FileMode::ExistingFile);
	QString fileName = fileDialProv->getOpenFileName(this, tr("Chosse Wanted PROV File"), "C:\\", tr("PROV Files (*.prov *.PROV)"));

	QFile provFile(fileName.toStdString().c_str());
	if (provFile.exists())
	{
		provItem->setBackgroundColor(Qt::green);
		provItem->setText(fileName);
	}
	
}

void LocaGUI::displayLog(QString info)
{
	ui.shellTextBrowser->append(info);
}

void LocaGUI::receiveElanPointer(InsermLibrary::ELAN *p_elan)
{
	LocaGUIBIP *test = new LocaGUIBIP(p_elan, 0);
	test->exec();
	p_elan->eeg_loc_montage(p_elan->trc->nameElectrodePositiv, p_elan->trc->signalPosition);
	emit bipDone(true);
	delete test;
}

void LocaGUI::openOptions()
{
	locaguiOpt *test2 = new locaguiOpt(this);
	QObject::connect(test2, SIGNAL(sendOptMenu(InsermLibrary::OptionLOCA *)), this, SLOT(receiveOptionPointer(InsermLibrary::OptionLOCA *)));
	test2->exec();
}

void LocaGUI::openAbout()
{
	QMessageBox::about(this, "This Software", "has been developed at Dycog - INSERM Lyon");
}

void LocaGUI::receiveOptionPointer(InsermLibrary::OptionLOCA *optionLOCA)
{
	if (optionLOCAGUI != nullptr)
	{
		delete optionLOCAGUI;
	}
	optionLOCAGUI = optionLOCA;
}

void LocaGUI::upDateProgressBar(int value)
{
	if ((value >= 0) && (value <= 100))
	{
		ui.progressBar->setValue(value);
	}
	if (value > 100)
	{
		ui.progressBar->setValue(100);
	}
}

void LocaGUI::finishedRuning()
{
	alreadyRunning = false;
}