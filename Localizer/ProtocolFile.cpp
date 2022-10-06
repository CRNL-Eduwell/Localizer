#include "ProtocolFile.h"

ProtocolFile::ProtocolFile(const QString & filePath)
{
    m_FileInfo = QFileInfo(filePath);
    if(m_FileInfo.exists())
    {
/*		m_Prov = new InsermLibrary::PROV(m_FileInfo.absoluteFilePath().toStdString());
        LoadProtocolInModel(*m_Prov)*/;
    }
    else
    {
        //m_Prov = nullptr;
        LoadEmptyModel();
    }
}

ProtocolFile::~ProtocolFile()
{
    //EEGFormat::Utility::DeleteAndNullify(m_Prov);
}

void ProtocolFile::Save()
{
  //  InsermLibrary::PROV newProv = InsermLibrary::PROV();
  //  newProv.filePath(m_FileInfo.absoluteFilePath().toStdString());

  //  int elementCount = m_ItemModel->rowCount();
  //  for(int i = 0; i < elementCount; i++)
  //  {
  //      InsermLibrary::BLOC currentBloc;
  //      //=========
  //      currentBloc.dispBloc.row(m_ItemModel->item(i, 0)->text().toInt());
  //      currentBloc.dispBloc.column(m_ItemModel->item(i, 1)->text().toInt());
  //      currentBloc.dispBloc.name(m_ItemModel->item(i, 2)->text().toStdString());
  //      currentBloc.dispBloc.path(m_ItemModel->item(i, 3)->text().toStdString());
  //      //== Window
  //      QStringList splitItem = m_ItemModel->item(i, 4)->text().split(":");
  //      currentBloc.dispBloc.window(splitItem[0].toInt(), splitItem[1].toInt());
  //      //== Baseline Window
  //      splitItem = m_ItemModel->item(i, 5)->text().split(":");
  //      currentBloc.dispBloc.baseLine(splitItem[0].toInt(), splitItem[1].toInt());
  //      //== MainEvent
  //      currentBloc.mainEventBloc.eventCode.push_back(m_ItemModel->item(i, 6)->text().toInt());
  //      currentBloc.mainEventBloc.eventLabel = m_ItemModel->item(i, 7)->text().toStdString();
  //      //== SecondaryEvents
  //      splitItem = m_ItemModel->item(i, 8)->text().split(":");
  //      currentBloc.secondaryEvents.push_back(InsermLibrary::EventBLOC());
  //      for (int j = 0; j < splitItem.count(); ++j)
  //      {
  //          currentBloc.secondaryEvents[0].eventCode.push_back(splitItem[j].toInt());
  //      }
  //      currentBloc.secondaryEvents[0].eventLabel = m_ItemModel->item(i, 9)->text().toStdString();
  //      //Sort
  //      currentBloc.dispBloc.sort(m_ItemModel->item(i, 10)->text().toStdString());
  //      //=========
  //      newProv.visuBlocs.push_back(currentBloc);
  //  }

  //  //== we cheat until there is some ui to manage that
  //  if(m_Prov != nullptr)
  //  {
  //      newProv.changeCodeFilePath = "";
		//if (m_Prov->changeCodeFilePath != "")
		//{
		//	std::vector<std::string> rootSplit = EEGFormat::Utility::Split<std::string>(InsermLibrary::GetCurrentWorkingDir(), "\\/");
		//	std::vector<std::string> chgCodeSplit = EEGFormat::Utility::Split<std::string>(m_Prov->changeCodeFilePath, "\\/");

		//	if (rootSplit[rootSplit.size() - 1] == chgCodeSplit[rootSplit.size() - 1])
		//	{
		//		chgCodeSplit.erase(chgCodeSplit.begin(), chgCodeSplit.begin() + static_cast<int>(rootSplit.size()));
		//		for (size_t i = 0; i < chgCodeSplit.size(); i++)
		//		{
		//			newProv.changeCodeFilePath += ("/" + chgCodeSplit[i]);
		//		}
		//	}
		//}

  //      newProv.invertmapsinfo = m_Prov->invertmapsinfo;
  //      newProv.invertmaps.baseLineWindow[0] = m_Prov->invertmaps.baseLineWindow[0];
  //      newProv.invertmaps.baseLineWindow[1] = m_Prov->invertmaps.baseLineWindow[1];
  //      newProv.invertmaps.epochWindow[0] = m_Prov->invertmaps.epochWindow[0];
  //      newProv.invertmaps.epochWindow[1] = m_Prov->invertmaps.epochWindow[1];
  //  }

  //  newProv.saveFile();
}

QStringList ProtocolFile::GetHeaderList()
{
    QStringList listeHeader;
    listeHeader << QString("Row") << QString("Col") << QString("Name") << QString("Path") <<
                   QString("Window") << QString("BaseLine") << QString("Main Event") <<
                   QString("Main Event Label") << QString("Secondary Events") <<
                   QString("Secondary Events Label") << QString("Sort");
    return listeHeader;
}

void ProtocolFile::LoadEmptyModel()
{
    QStringList listeHeader = GetHeaderList();

    m_ItemModel = new QStandardItemModel(static_cast<int>(1), 11);
    m_ItemModel->setHorizontalHeaderLabels(listeHeader);

    for (uint i = 0; i < 1; i++)
    {
        QStandardItem *row = new QStandardItem("Row");
        m_ItemModel->setItem(static_cast<int>(i), 0, row);

        QStandardItem *col = new QStandardItem("Column");
        m_ItemModel->setItem(static_cast<int>(i), 1, col);

        QStandardItem *Label = new QStandardItem("Bloc Label");
        m_ItemModel->setItem(static_cast<int>(i), 2, Label);

        QStandardItem *path = new QStandardItem("Pic Path");
        m_ItemModel->setItem(static_cast<int>(i), 3, path);

        QStandardItem *window = new QStandardItem("0 : 200");
        m_ItemModel->setItem(static_cast<int>(i), 4, window);

        QStandardItem *baseline = new QStandardItem("-200 : 0");
        m_ItemModel->setItem(static_cast<int>(i), 5, baseline);

        QStandardItem *mainCode = new QStandardItem("Main Code");
        m_ItemModel->setItem(static_cast<int>(i), 6, mainCode);

        QStandardItem *mainLabel = new QStandardItem("Main Code Label");
        m_ItemModel->setItem(static_cast<int>(i), 7, mainLabel);

        QStandardItem *secondaryCode = new QStandardItem("Secondary Code");
        m_ItemModel->setItem(static_cast<int>(i), 8, secondaryCode);

        QStandardItem *secondaryLabel = new QStandardItem("Secondary Code Label");
        m_ItemModel->setItem(static_cast<int>(i), 9, secondaryLabel);

        QStandardItem *sortCode = new QStandardItem("Sorting Codes");
        m_ItemModel->setItem(static_cast<int>(i), 10, sortCode);
    }
}

//void ProtocolFile::LoadProtocolInModel(InsermLibrary::PROV protocol)
//{
//    QStringList listeHeader = GetHeaderList();
//
//    size_t elementCount = protocol.visuBlocs.size();
//    m_ItemModel = new QStandardItemModel(static_cast<int>(elementCount), 11);
//    m_ItemModel->setHorizontalHeaderLabels(listeHeader);
//
//    for (uint i = 0; i < elementCount; i++)
//    {
//        QStandardItem *row = new QStandardItem(QString::number(protocol.visuBlocs[i].dispBloc.row()));
//        m_ItemModel->setItem(static_cast<int>(i), 0, row);
//
//        QStandardItem *col = new QStandardItem(QString::number(protocol.visuBlocs[i].dispBloc.column()));
//        m_ItemModel->setItem(static_cast<int>(i), 1, col);
//
//        QStandardItem *Label = new QStandardItem(QString::fromStdString(protocol.visuBlocs[i].dispBloc.name()));
//        m_ItemModel->setItem(static_cast<int>(i), 2, Label);
//
//        QStandardItem *path = new QStandardItem(protocol.visuBlocs[i].dispBloc.path().c_str());
//        m_ItemModel->setItem(static_cast<int>(i), 3, path);
//
//        QStandardItem *window = new QStandardItem(QString::number(protocol.visuBlocs[i].dispBloc.windowMin()) + ":" + QString::number(protocol.visuBlocs[i].dispBloc.windowMax()));
//        m_ItemModel->setItem(static_cast<int>(i), 4, window);
//
//        QStandardItem *baseline = new QStandardItem(QString::number(protocol.visuBlocs[i].dispBloc.baseLineMin()) + ":" + QString::number(protocol.visuBlocs[i].dispBloc.baseLineMax()));
//        m_ItemModel->setItem(static_cast<int>(i), 5, baseline);
//
//        QStandardItem *mainCode = new QStandardItem(QString::number(protocol.visuBlocs[i].mainEventBloc.eventCode[0]));
//        m_ItemModel->setItem(static_cast<int>(i), 6, mainCode);
//
//        QStandardItem *mainLabel = new QStandardItem(protocol.visuBlocs[i].mainEventBloc.eventLabel.c_str());
//        m_ItemModel->setItem(static_cast<int>(i), 7, mainLabel);
//
//        QString eventString;
//        for (size_t j = 0; j < protocol.visuBlocs[i].secondaryEvents.size(); j++)
//        {
//            eventString += QString::number(protocol.visuBlocs[i].secondaryEvents[j].eventCode[0]);
//
//            if (j < protocol.visuBlocs[i].secondaryEvents.size() - 1)
//                eventString += ":";
//        }
//        QStandardItem *secondaryCode = new QStandardItem(eventString);
//        m_ItemModel->setItem(static_cast<int>(i), 8, secondaryCode);
//
//        QStandardItem *secondaryLabel = new QStandardItem(protocol.visuBlocs[i].secondaryEvents[0].eventLabel.c_str());
//        m_ItemModel->setItem(static_cast<int>(i), 9, secondaryLabel);
//
//        QStandardItem *sortCode = new QStandardItem(protocol.visuBlocs[i].dispBloc.sort().c_str());
//        m_ItemModel->setItem(static_cast<int>(i), 10, sortCode);
//    }
//}
