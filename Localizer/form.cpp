#include "form.h"

using namespace InsermLibrary;

form::form(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	QStringList provFiles = GetFilesFromRootFolder("*.prov");
	ConnectSignals();
	InitUiParameters();
	InitProvListUi(provFiles);
}

form::~form()
{
	deleteAndNullify1D(m_currentProv);
}

void form::ConnectSignals()
{
	connect(ui.listWidget_loca, &QHeaderView::customContextMenuRequested, this, &form::ShowLocaListContextMenu);
	connect(ui.listWidget_loca->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [&](const QModelIndex current, const QModelIndex previous)
	{
		if (m_dataChanged)
		{
			QMessageBox::StandardButton reply = QMessageBox::question(this, "Data was modified", "Do you want to save the modifications to this experiment parameters ? ", QMessageBox::Yes | QMessageBox::No);
			if (reply == QMessageBox::Yes)
				Save();

			m_dataChanged = false;
		}
		
		QMap<int, QVariant> currentItem = ui.listWidget_loca->model()->itemData(current);
		if (currentItem.count() == 1)
		{
			LoadProvTabUi(currentItem[0].value<QString>());
		}
	});
	connect(ui.tableWidget, &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.tableWidget->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.tableWidget->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.buttonBox_save, &QDialogButtonBox::accepted, this, [&] { Save(); m_dataChanged = false; });
	connect(ui.buttonBox_save, &QDialogButtonBox::rejected, this, [&] { close(); });
}

void form::InitUiParameters()
{
	ui.tableWidget->setColumnCount(11);
	DefineHorizontalHeader(ui.tableWidget);

	ui.listWidget_loca->setContextMenuPolicy(Qt::CustomContextMenu);

	ui.tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.tableWidget->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	ui.tableWidget->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
}

void form::InitProvListUi(QStringList provList)
{
	ui.listWidget_loca->clear();
	for (int i = 0; i < provList.size(); i++)
	{
		QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
		currentPROV->setText(provList[i].split(".prov", QString::SplitBehavior::SkipEmptyParts)[0]);
		currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	}
	connect(ui.tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(ManageChangeItem(QTableWidgetItem*)));
}

void form::DefineHorizontalHeader(QTableWidget *tabWidget)
{
	QStringList listeHeader;
	listeHeader << QString("Row") << QString("Col") << QString("Name") << QString("Path") <<
		QString("Window") << QString("BaseLine") << QString("Main Event") <<
		QString("Main Event Label") << QString("Secondary Events") <<
		QString("Secondary Events Label") << QString("Sort");

	tabWidget->setHorizontalHeaderLabels(listeHeader);
}

QStringList form::GetFilesFromRootFolder(QString fileExt)
{
	QDir currentDir(m_provFolder);
	currentDir.setFilter(QDir::Files);
	currentDir.setNameFilters(QStringList() << fileExt);
	return currentDir.entryList();
}

void form::LoadProvTabUi(QString provName)
{
	QString currentPath = m_provFolder + "/" + provName + ".prov";
	deleteAndNullify1D(m_currentProv);
	m_currentProv = new PROV(currentPath.toStdString());

	m_isLoadingView = true;

	ui.tableWidget->setRowCount(m_currentProv->nbRow());
	m_listHeaderProv.clear();
	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
		m_listHeaderProv << QString(m_currentProv->visuBlocs[i].dispBloc.name().c_str());
	ui.tableWidget->verticalHeader()->hide();
	ui.tableWidget->setVerticalHeaderLabels(m_listHeaderProv);
	ui.tableWidget->verticalHeader()->show();

	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[i].dispBloc.row())));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[i].dispBloc.column())));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(m_currentProv->visuBlocs[i].dispBloc.name().c_str()));
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(m_currentProv->visuBlocs[i].dispBloc.path().c_str()));
		ui.tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[i].dispBloc.windowMin()) + ":" + QString::number(m_currentProv->visuBlocs[i].dispBloc.windowMax())));
		ui.tableWidget->setItem(i, 5, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[i].dispBloc.baseLineMin()) + ":" + QString::number(m_currentProv->visuBlocs[i].dispBloc.baseLineMax())));
		ui.tableWidget->setItem(i, 6, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[i].mainEventBloc.eventCode[0])));
		ui.tableWidget->setItem(i, 7, new QTableWidgetItem(m_currentProv->visuBlocs[i].mainEventBloc.eventLabel.c_str()));

		QString eventString;
		for (int j = 0; j < m_currentProv->visuBlocs[i].secondaryEvents.size(); j++)
		{
			eventString += QString::number(m_currentProv->visuBlocs[i].secondaryEvents[j].eventCode[0]);

			if (j < m_currentProv->visuBlocs[i].secondaryEvents.size() - 1)
				eventString += ":";
		}
		ui.tableWidget->setItem(i, 8, new QTableWidgetItem(eventString));
		ui.tableWidget->setItem(i, 9, new QTableWidgetItem(m_currentProv->visuBlocs[i].secondaryEvents[0].eventLabel.c_str()));
		ui.tableWidget->setItem(i, 10, new QTableWidgetItem(m_currentProv->visuBlocs[i].dispBloc.sort().c_str()));
	}

	m_isLoadingView = false;
}

InsermLibrary::PROV* form::GetProvTabUi()
{
	PROV* newProv = new PROV();
	newProv->filePath(m_currentProv->filePath());

	for (int i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		BLOC currentBloc;
		//=========
		currentBloc.dispBloc.row(ui.tableWidget->item(i, 0)->text().toInt());
		currentBloc.dispBloc.column(ui.tableWidget->item(i, 1)->text().toInt());
		currentBloc.dispBloc.name(ui.tableWidget->item(i, 2)->text().toStdString());
		currentBloc.dispBloc.path(ui.tableWidget->item(i, 3)->text().toStdString());
		//== Window
		QStringList splitItem = ui.tableWidget->item(i, 4)->text().split(":");
		currentBloc.dispBloc.window(splitItem[0].toInt(), splitItem[1].toInt());
		//== Baseline Window
		splitItem = ui.tableWidget->item(i, 5)->text().split(":");
		currentBloc.dispBloc.baseLine(splitItem[0].toInt(), splitItem[1].toInt());
		//== MainEvent
		currentBloc.mainEventBloc.eventCode.push_back(ui.tableWidget->item(i, 6)->text().toInt());
		currentBloc.mainEventBloc.eventLabel = ui.tableWidget->item(i, 7)->text().toStdString();
		//== SecondaryEvents
		splitItem = ui.tableWidget->item(i, 8)->text().split(":");
		currentBloc.secondaryEvents.push_back(EventBLOC());
		for (int j = 0; j < splitItem.count(); ++j)
		{
			currentBloc.secondaryEvents[0].eventCode.push_back(splitItem[j].toInt());
		}
		currentBloc.secondaryEvents[0].eventLabel = ui.tableWidget->item(i, 9)->text().toStdString();
		//Sort
		currentBloc.dispBloc.sort(ui.tableWidget->item(i, 10)->text().toStdString());
		//=========
		newProv->visuBlocs.push_back(currentBloc);
	}

	return newProv;
}

void form::ShowLocaListContextMenu(QPoint point)
{
	QModelIndex index = ui.tableWidget->indexAt(point);
	QMenu* contextMenu = new QMenu();
	connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
	QObject* senderObject = sender();

	QAction* addLocalizerAction = contextMenu->addAction("Add localizer", [this]
	{
		bool ok;
		QString localizerName = QInputDialog::getText(ui.tableWidget, "New Name", "Choose Localizer Name", QLineEdit::Normal, "New Localizer", &ok);
		if (ok && !localizerName.isEmpty())
		{
			QListWidgetItem *currentPROV = new QListWidgetItem(ui.listWidget_loca);
			currentPROV->setText(localizerName);
			currentPROV->setFlags(currentPROV->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		}
	});
	QAction* removeLocalizerAction = contextMenu->addAction("Remove localizer", [this]
	{
		QModelIndexList indexes = ui.listWidget_loca->selectionModel()->selectedIndexes();
		while (!indexes.isEmpty())
		{
			ui.listWidget_loca->model()->removeRows(indexes.last().row(), 1);
			indexes.removeLast();
		}
	});

	// Position
	if (senderObject == ui.listWidget_loca)
		contextMenu->exec(ui.listWidget_loca->viewport()->mapToGlobal(point));
}

//use to keep elements well formated by the user
void form::ManageChangeItem(QTableWidgetItem* index)
{
	int idLoca = ui.listWidget_loca->currentRow();
	int row = index->row();
	int col = index->column();

	if (!m_isLoadingView)
		m_dataChanged = true;

	switch (col)
	{
		case 2 :
		{
			m_listHeaderProv[row] = ui.tableWidget->item(row, col)->text();	
			ui.tableWidget->verticalHeader()->hide();
			ui.tableWidget->setVerticalHeaderLabels(m_listHeaderProv);
			ui.tableWidget->verticalHeader()->show();
			break;
		}
		case 4:
		{
			QStringList splitData = ui.tableWidget->item(row, col)->text().split(":");
			if (splitData.count() != 2)
			{
				QMessageBox *error = new QMessageBox(QMessageBox::Warning, "ERROR", "The epoch window should be put as such : 0:500", QMessageBox::Ok);
				error->exec();
				ui.tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[row].dispBloc.windowMin()) + ":" + QString::number(m_currentProv->visuBlocs[row].dispBloc.windowMax())));
			}
			break;
		}
		case 5:
		{
			QStringList splitData = ui.tableWidget->item(row, col)->text().split(":");
			if (splitData.count() != 2)
			{
				QMessageBox *error = new QMessageBox(QMessageBox::Warning, "ERROR", "The baseline window should be put as such : 0:500", QMessageBox::Ok);
				error->exec();
				ui.tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(m_currentProv->visuBlocs[row].dispBloc.baseLineMin()) + ":" + QString::number(m_currentProv->visuBlocs[row].dispBloc.baseLineMax())));
			}
			break;
		}
	}
}

void form::ShowProvTabContextMenu(QPoint point)
{
	QModelIndex index = ui.tableWidget->indexAt(point);
	QMenu* contextMenu = new QMenu();
	connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QMenu::deleteLater);
	QObject* senderObject = sender();

	// Position
	if (senderObject == ui.tableWidget)
	{
		contextMenu->exec(ui.tableWidget->viewport()->mapToGlobal(point));
	}
	else if (senderObject == ui.tableWidget->verticalHeader())
	{
		QAction* addBlocAction = contextMenu->addAction("Add a condition to this experiment", [&]
		{
			bool ok;
			QString conditionName = QInputDialog::getText(ui.tableWidget, "New Name", "Choose Condition Name", QLineEdit::Normal, "New Condition", &ok);
			if (ok && !conditionName.isEmpty())
			{
				ui.tableWidget->model()->insertRow(ui.tableWidget->model()->rowCount(), index);

				m_listHeaderProv.append(conditionName);						//Add to vertical list behind
				ui.tableWidget->setVerticalHeaderLabels(m_listHeaderProv);

				m_dataChanged = true;
			}
		});
		QAction* removeBlocAction = contextMenu->addAction("Remove selected conditions from this experiment", [this]
		{
			QModelIndexList indexes = ui.tableWidget->selectionModel()->selectedRows();
			while (!indexes.isEmpty())
			{
				ui.tableWidget->model()->removeRows(indexes.last().row(), 1); //remove from tab
				m_listHeaderProv.removeAt(indexes.last().row());				  //remove from vertical list behind
				indexes.removeLast();
			}
			m_dataChanged = true;
		});
		contextMenu->exec(ui.tableWidget->verticalHeader()->viewport()->mapToGlobal(point));
	}
	else if (senderObject == ui.tableWidget->horizontalHeader())
	{
		contextMenu->exec(ui.tableWidget->horizontalHeader()->viewport()->mapToGlobal(point));
	}
}

void form::Save()
{
	InsermLibrary::PROV *prov = GetProvTabUi();
	//== we cheat until there is some ui to manage that
	prov->changeCodeFilePath = "";
	std::vector<std::string> rootSplit = split<std::string>(GetCurrentWorkingDir(), "\\/");
	std::vector<std::string> chgCodeSplit = split<std::string>(m_currentProv->changeCodeFilePath, "\\/");

	if (rootSplit[rootSplit.size() - 1] == chgCodeSplit[rootSplit.size() - 1])
	{
		chgCodeSplit.erase(chgCodeSplit.begin(), chgCodeSplit.begin() + rootSplit.size());
		for each (std::string var in chgCodeSplit)
			prov->changeCodeFilePath += ("/" + var);
	}

	prov->invertmapsinfo = m_currentProv->invertmapsinfo;
	prov->invertmaps.baseLineWindow[0] = m_currentProv->invertmaps.baseLineWindow[0];
	prov->invertmaps.baseLineWindow[1] = m_currentProv->invertmaps.baseLineWindow[1];
	prov->invertmaps.epochWindow[0] = m_currentProv->invertmaps.epochWindow[0];
	prov->invertmaps.epochWindow[1] = m_currentProv->invertmaps.epochWindow[1];
	//===========================================================================
	prov->saveFile();
	deleteAndNullify1D(prov);
}
