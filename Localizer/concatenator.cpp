#include "concatenator.h"

using namespace std;

concatenator::concatenator(string rootFolder, QWidget *parent) : QDialog(parent)
{
	m_rootFolder = rootFolder;
	ui.setupUi(this);
	updateTRCList();
	connect(ui.listWidget_TRC, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(checkOnEnter(QListWidgetItem *)));
	connect(ui.concatenateButton, SIGNAL(clicked()), this, SLOT(concatenateFiles()));
}

concatenator::~concatenator()
{

}

void concatenator::checkOnEnter(QListWidgetItem * item)
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

void concatenator::concatenateFiles()
{
	if (ui.outputLineEdit->text().toStdString() == "")
		QMessageBox::information(this, "Error", "You Need To Select to indicate an output file name");

	//Récupérer les élements selectionné
	int nbFile = ui.listWidget_TRC->count();
	int nbChecked = 0;
	QList<int> indexes;
	for (int i = 0; i < nbFile; i++)
	{
		if (ui.listWidget_TRC->item(i)->checkState() == Qt::Checked)
		{
			nbChecked++;
			indexes.push_back(i);
		}
	}

	if (nbChecked != 2)
	{
		QMessageBox::information(this, "Error", "You Need To Select Two Files");
	}
	else
	{
		if (isAlreadyRunning == false)
		{
			string myFirstFile = m_rootFolder + "/" + ui.listWidget_TRC->item(indexes[0])->text().toStdString();
			string mySecondFile = m_rootFolder + "/" + ui.listWidget_TRC->item(indexes[1])->text().toStdString();
			string myOutputFile = m_rootFolder + "/" + ui.outputLineEdit->text().toStdString();

			thread = new QThread;
			worker = new Worker(myFirstFile, mySecondFile, myOutputFile);

			//Update info
			connect(worker, &Worker::sendLogInfo, this, [&] (QString info) { emit sendLogInfo(info); });

			//=== Event From worker and thread
			connect(thread, SIGNAL(started()), worker, SLOT(processConcatenation()));
			connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
			connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(worker, &Worker::finished, this, [&] { isAlreadyRunning = false; close(); });

			//=== Launch Thread and lock possible second launch
			worker->moveToThread(thread);
			thread->start();
			isAlreadyRunning = true;
		}
		else
		{
			QMessageBox::information(this, "Error", "Concatenation already running");
		}
	}
}

void concatenator::updateTRCList()
{
	QDir currentDir(m_rootFolder.c_str());
	currentDir.setNameFilters(QStringList() << "*.trc");

	QStringList entries = currentDir.entryList();
	for (QStringList::ConstIterator entry = entries.begin(); entry != entries.end(); ++entry)
	{
		QString dirname = *entry;
		if (dirname != QObject::tr(".") && dirname != QObject::tr(".."))
		{
			QListWidgetItem *currentTRC = new QListWidgetItem(ui.listWidget_TRC);
			currentTRC->setText(dirname);
			currentTRC->setFlags(currentTRC->flags() | Qt::ItemIsUserCheckable); // set checkable flag
			currentTRC->setCheckState(Qt::Unchecked); // AND initialize check state
		}
	}
}