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

void form::ConnectSignals()
{
	connect(ui.listWidget_loca, &QHeaderView::customContextMenuRequested, this, &form::ShowLocaListContextMenu);
	connect(ui.listWidget_loca->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [&](const QModelIndex current, const QModelIndex previous)
	{
		QMap<int, QVariant> currentItem = ui.listWidget_loca->model()->itemData(current);
		if (currentItem.count() == 1)
		{
			LoadProvTabUi(currentItem[0].value<QString>());
		}
	});
	connect(ui.tableWidget, &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.tableWidget->verticalHeader(), &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.tableWidget->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &form::ShowProvTabContextMenu);
	connect(ui.buttonBox_save, &QDialogButtonBox::accepted, this, &form::save);
	connect(ui.buttonBox_save, &QDialogButtonBox::rejected, this, [&] { close(); });
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

//use to keep elements well formated by the user
void form::ManageChangeItem(QTableWidgetItem* index)
{
	int idLoca = ui.listWidget_loca->currentRow();
	int row = index->row();
	int col = index->column();

	switch (col)
	{
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

void form::save()
{
	qDebug() << "Save File(s) , need to be reimplemented" << endl;
	//for (int i = 0; i < listprov.size(); i++)
	//	listprov[i].saveFile(provFolder.toStdString(), ui.listWidget_loca->item(i)->text().toStdString());
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
		});
		contextMenu->exec(ui.tableWidget->verticalHeader()->viewport()->mapToGlobal(point));
	}
	else if (senderObject == ui.tableWidget->horizontalHeader())
	{
		contextMenu->exec(ui.tableWidget->horizontalHeader()->viewport()->mapToGlobal(point));
	}
}