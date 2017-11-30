#include "form.h"

using namespace InsermLibrary;

form::form(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	initsize = width();

	loadProvListUI(getFilesFromRootFolder("*.prov"));
	load();
}

form::~form()
{

}

void form::load()
{
	ui.tableWidget->setColumnCount(11);
	defineHorizontalHeader(ui.tableWidget);
	
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	connect(ui.button_addBloc, &QPushButton::clicked, this, &form::addBloc);
	connect(ui.button_removeBloc, &QPushButton::clicked, this, &form::removeBloc);
	connect(ui.button_addLoca, &QPushButton::clicked, this, &form::addLoca);
	connect(ui.button_removeLoca, &QPushButton::clicked, this, &form::removeLoca);
	connect(ui.buttonBox_save, &QDialogButtonBox::accepted, this, &form::save);
	connect(ui.buttonBox_save, &QDialogButtonBox::rejected, this, [&] { close(); });
}

void form::defineHorizontalHeader(QTableWidget *tabWidget)
{
	QStringList listeHeader;
	listeHeader << QString("Row") << QString("Col") << QString("Name") << QString("Path") <<
				   QString("Window") << QString("BaseLine") << QString("Main Event") <<
				   QString("Main Event Label") << QString("Secondary Events") <<
				   QString("Secondary Events Label") << QString("Sort");

	tabWidget->setHorizontalHeaderLabels(listeHeader);
}

QStringList form::getFilesFromRootFolder(QString fileExt)
{
	QDir currentDir(provFolder);
	currentDir.setFilter(QDir::Files);
	currentDir.setNameFilters(QStringList() << fileExt);
	return currentDir.entryList();
}

void form::loadProvListUI(QStringList provList)
{
	ui.listWidget_loca->clear();
	listprov.clear();
	for (int i = 0; i < provList.size(); i++)
	{
		QString currentPath = provFolder + "/" + provList[i];
		listprov.push_back(PROV(currentPath.toStdString()));
		//==
		QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
		currentPROV->setText(provList[i].split(".prov", QString::SplitBehavior::SkipEmptyParts)[0]);
		currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	}
	connect(ui.listWidget_loca, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(displaySelectedProv(QListWidgetItem *)));
	connect(ui.tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(manageChangeItem(QTableWidgetItem*)));
}

void form::displaySelectedProv(QListWidgetItem* item)
{
	int idLoca = item->listWidget()->row(item);

	ui.tableWidget->setRowCount(listprov[idLoca].nbRow());
	listeHeaderLoca.clear();
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
		listeHeaderLoca << QString(listprov[idLoca].visuBlocs[i].dispBloc.name().c_str());
	ui.tableWidget->verticalHeader()->hide();
	ui.tableWidget->setVerticalHeaderLabels(listeHeaderLoca);
	ui.tableWidget->verticalHeader()->show();
	setMinimumWidth(initsize + ui.tableWidget->verticalHeader()->width());
	
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[i].dispBloc.row())));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[i].dispBloc.column())));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(listprov[idLoca].visuBlocs[i].dispBloc.name().c_str()));
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(listprov[idLoca].visuBlocs[i].dispBloc.path().c_str()));
		ui.tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[i].dispBloc.windowMin()) + ":" + QString::number(listprov[idLoca].visuBlocs[i].dispBloc.windowMax())));
		ui.tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[i].dispBloc.baseLineMin()) + ":" + QString::number(listprov[idLoca].visuBlocs[i].dispBloc.baseLineMax())));	
		ui.tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[i].mainEventBloc.eventCode[0])));
		ui.tableWidget->setItem(i, 7, new QTableWidgetItem(listprov[idLoca].visuBlocs[i].mainEventBloc.eventLabel.c_str()));

		QString eventString;
		for (int j = 0; j < listprov[idLoca].visuBlocs[i].secondaryEvents.size(); j++)
		{
			eventString += QString::number(listprov[idLoca].visuBlocs[i].secondaryEvents[j].eventCode[0]);

			if( j < listprov[idLoca].visuBlocs[i].secondaryEvents.size() - 1)
				eventString += ":";
		}
		ui.tableWidget->setItem(i, 8, new QTableWidgetItem(eventString));
		ui.tableWidget->setItem(i, 9, new QTableWidgetItem(listprov[idLoca].visuBlocs[i].secondaryEvents[0].eventLabel.c_str()));
		ui.tableWidget->setItem(i, 10, new QTableWidgetItem(listprov[idLoca].visuBlocs[i].dispBloc.sort().c_str()));
	}
}

void form::manageChangeItem(QTableWidgetItem* index)
{
	int idLoca = ui.listWidget_loca->currentRow();
	int row = index->row();
	int col = index->column();

	switch (col)
	{
	case 0:
		listprov[idLoca].row(row, ui.tableWidget->item(row, col)->text().toStdString());
		break;
	case 1:
		listprov[idLoca].column(row, ui.tableWidget->item(row, col)->text().toStdString());
		break;
	case 2:
		listprov[idLoca].name(row, ui.tableWidget->item(row, col)->text().toStdString());
		break;
	case 3:
		listprov[idLoca].path(row, ui.tableWidget->item(row, col)->text().toStdString());
		break;
	case 4:
	{
		QStringList splitData = ui.tableWidget->item(row, col)->text().split(":");
		if (splitData.count() == 2)
		{
			listprov[idLoca].visuBlocs[row].dispBloc.window(splitData[0].toInt(), splitData[1].toInt());
		}
		else
		{
			QMessageBox *error = new QMessageBox(QMessageBox::Warning, "ERROR", "The epoch window should be put as such : 0:500", QMessageBox::Ok);
			error->exec();
			ui.tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(listprov[idLoca].visuBlocs[row].dispBloc.windowMin()) + ":" + QString::number(listprov[idLoca].visuBlocs[row].dispBloc.windowMax())));
		}
		break;
	}
	case 5:
		listprov[idLoca].name(row, ui.tableWidget->item(row, col)->text().toStdString());
		break;
	case 6:
		listprov[idLoca].visuBlocs[row].mainEventBloc.eventCode.push_back(ui.tableWidget->item(row, col)->text().toInt());
		break;
	case 7:
		listprov[idLoca].visuBlocs[row].mainEventBloc.eventLabel = ui.tableWidget->item(row, col)->text().toStdString();
		break;
	case 8:
	{
			QStringList splitData = ui.tableWidget->item(row, col)->text().split(":");
			if (splitData.count() > 0)
			{
				EventBLOC currentBloc;
				for (int i = 0; i < splitData.count(); i++)
				{
					int currentInt = splitData[i].toInt();
					currentBloc.eventCode.push_back(currentInt);
				}
				listprov[idLoca].visuBlocs[row].secondaryEvents.push_back(currentBloc);
			}
			break;
	}
	case 9:
		listprov[idLoca].visuBlocs[row].secondaryEvents[0].eventLabel = ui.tableWidget->item(row, col)->text().toStdString();
		break;
	case 10:
		listprov[idLoca].visuBlocs[row].dispBloc.sort(ui.tableWidget->item(row, col)->text().toStdString());
	}
}

void form::addLoca()
{
	listprov.push_back(PROV());
	QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
	currentPROV->setText("NEW LOCA");
	currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
}

void form::removeLoca()
{
	int selectedID = ui.listWidget_loca->selectionModel()->currentIndex().row();
	if (selectedID < ui.listWidget_loca->count())
	{
		listprov.erase(listprov.begin() + selectedID);
		ui.listWidget_loca->item(selectedID)->~QListWidgetItem();
	}
}

void form::addBloc()
{
	ui.tableWidget->insertRow(ui.tableWidget->rowCount());
	listeHeaderLoca << QString(ui.lineEdit_blocName->text());
	ui.tableWidget->verticalHeader()->hide();
	ui.tableWidget->setVerticalHeaderLabels(listeHeaderLoca);
	ui.tableWidget->verticalHeader()->show();
	ui.tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	setMinimumWidth(initsize + ui.tableWidget->verticalHeader()->width());

	int selectedLOCA = ui.listWidget_loca->selectionModel()->currentIndex().row();
	BLOC newBlock = BLOC();
	newBlock.dispBloc.name(ui.lineEdit_blocName->text().toStdString());
	listprov[selectedLOCA].visuBlocs.push_back(newBlock);
}

void form::removeBloc()
{
	int selectedID = ui.tableWidget->selectionModel()->currentIndex().row();
	if (selectedID < ui.tableWidget->rowCount())
	{
		listeHeaderLoca.removeAt(selectedID);
		ui.tableWidget->verticalHeader()->hide();
		ui.tableWidget->removeRow(selectedID);
		ui.tableWidget->verticalHeader()->show();
		ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
		setMinimumWidth(initsize + ui.tableWidget->verticalHeader()->width());
	}
}

void form::save()
{
	for (int i = 0; i < listprov.size(); i++)
		listprov[i].saveFile(provFolder.toStdString(), ui.listWidget_loca->item(i)->text().toStdString());
}