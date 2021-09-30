#include "barsPlotsGenerator.h"

InsermLibrary::DrawbarsPlots::baseCanvas::baseCanvas(InsermLibrary::PROV *myprovFile, int width, int heigth)
{
	Width = width;
	Heigth = heigth;
	drawTemplate(myprovFile);
}

InsermLibrary::DrawbarsPlots::baseCanvas::~baseCanvas()
{

}

void InsermLibrary::DrawbarsPlots::baseCanvas::drawTemplate(InsermLibrary::PROV *myprovFile)
{
	//570 = 0.296875 * Width (Scr width = 1920)
	int nbRow = myprovFile->nbRow();
	pixmapTemplate = QPixmap(Width, Heigth);
	pixmapTemplate.fill(QColor(Qt::white));
	QPainter painter(&pixmapTemplate);
	painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
	painter.setFont(QFont("Arial", 12, 1, false));

	nbColLegend = ceil((double)nbRow / 3);

	int widthLeg = 0.234375 * Width;
	int heigthLeg = 0.0352112676 * Heigth;
	for (int i = 0; i < nbElecPerFigure; i++)
	{
		//Write legend
		int s_x = (Width * 0.0586) + ((0.296875 * Width) * (i % 3));
		int s_y = Heigth * 0.0325;

		for (int j = 0; j < nbRow; j++)
		{
			switch (j % 3)
			{
			case 0:
				painter.setPen(QColor(0, 0, 255, 255)); //blue
				break;
			case 1:
				painter.setPen(QColor(255, 0, 0, 255)); //red
				break;
			case 2:
				painter.setPen(QColor(0, 255, 0, 255)); //green
				break;
			default:
				painter.setPen(QColor(0, 0, 0, 255)); //black
				break;
			}
			painter.drawText(s_x, s_y, widthLeg, heigthLeg, Qt::AlignLeft | Qt::AlignTop, myprovFile->visuBlocs[j].mainEventBloc.eventLabel.c_str());
			s_y = s_y + (0.021994147797 * Heigth);   //25; Heig = 1136

			if ((j + 1) % 3 == 0)
			{
				s_x = s_x + ((0.25 * Width) / nbColLegend); //480 = 0.25 * Width (Scr width = 1920)
				s_y = Heigth * 0.0325;
			}
		}

		//On repasse en noir
		painter.setPen(QColor(0, 0, 0, 255));

		QRect recElec((Width * 0.028) + ((0.296875 * Width) * (i % 3)), Heigth * 0.094, Width * 0.0105, Heigth * 0.812);
		painter.drawRect(recElec);
		painter.fillRect(recElec, QColor(255, 255, 255, 128));

		for (int j = 0; j < nbSite; j++)
		{
			painter.fillRect((Width * 0.028) + ((0.296875 * Width) * (i % 3)), (Heigth * 0.094) + (Heigth * 0.0203) + 
							 (Heigth * 0.0406 * (j)), Width * 0.0105, Heigth * 0.0203, QColor(0, 0, 0, 255));
			painter.drawText((Width * 0.015) + ((0.296875 * Width) * (i % 3)), (Heigth * 0.107) + (Heigth * 0.0203) + 
							 (Heigth * 0.0406 * (j)), QString().setNum(j + 1));
		}
	}
}

InsermLibrary::DrawbarsPlots::drawBars::drawBars(InsermLibrary::PROV *myprovFile, std::string outputFolder, QSize size) : baseCanvas(myprovFile,
																				   size.width(), 
																				   size.height())
{
	this->myprovFile = myprovFile;
	picOutputFolder = outputFolder;
}

InsermLibrary::DrawbarsPlots::drawBars::~drawBars()
{

}

void InsermLibrary::DrawbarsPlots::drawBars::drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, vec1<PVALUECOORD> significantValue,
								  eegContainer* myeegContainer)
{
	int idCurrentElecDrawn = 0, nbFigureDrawn = 0, countBipole = 0;
	QPainter *painter = nullptr;
	QPixmap *pixmap = nullptr;
	QColor drawRectColor;

	int *windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());
	int nbRow = myprovFile->nbRow();

	QString tifName = "";
	vec1<float> erp = vec1<float>(nbRow);							vec1<float> lim = vec1<float>(nbRow);
	vec1<float> erpP = vec1<float>(nbRow);							vec1<float> erpM = vec1<float>(nbRow);
	vec1<float> erpPMax = vec1<float>(nbRow);						vec1<float> erpMMax = vec1<float>(nbRow);
	vec1<bool> sizeSigCheck = vec1<bool>(significantValue.size());  vec2<float> data = vec2<float>(triggerContainer->ProcessedTriggerCount(), 
																					   vec1<float>(windowSam[1] - windowSam[0]));

	std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();
	std::vector<Trigger> Triggers = triggerContainer->ProcessedTriggers();
	for (int i = 0; i < myeegContainer->electrodes.size(); i++)
	{
		idCurrentElecDrawn = i % nbElecPerFigure; 
		if (idCurrentElecDrawn == 0)
		{
			if (nbFigureDrawn > 0)
			{
				pixmap->save(tifName, "JPG");
			}
			nbFigureDrawn++;
			//==
			deleteAndNullify1D(painter);
			deleteAndNullify1D(pixmap);
			pixmap = new QPixmap(pixmapTemplate);
			painter = new QPainter(pixmap);
			painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
			painter->setFont(QFont("Arial", 12, 1, false));
			//==
			tifName = createPicPath(picOutputFolder, myeegContainer, i);
		}

		//Write Electrode Name
		painter->drawText((Width * 0.028) + 8 + ((0.296875 * Width) * (i % 3)), Heigth * 0.090, 
						   myeegContainer->electrodes[i].label.c_str());

		for (int j = 0; j < nbSite; j++)
		{
			if (j + 1 < myeegContainer->electrodes[i].id.size())
			{
				if (myeegContainer->BipolePositivLabel(countBipole) ==
                    myeegContainer->electrodes[i].label + std::to_string(myeegContainer->electrodes[i].id[j + 1]))
				{
					//Extract Data
					for (int k = 0; k < triggerContainer->ProcessedTriggerCount(); k++)
					{
						for (int l = 0; l < windowSam[1] - windowSam[0]; l++)
						{
							data[k][l] = bigData[k][countBipole][l];
						}
					}
					countBipole++;

					//Loop accross conditions
					for (int k = 0; k < nbRow; k++)
					{
						int lowTrigg = SubGroupStimTrials[k];
						int highTrigg = SubGroupStimTrials[k + 1] - 1;
						int currentMainCode = myprovFile->visuBlocs[k].mainEventBloc.eventCode[0];

						if (Triggers[lowTrigg].MainCode() == currentMainCode && Triggers[highTrigg].MainCode() == currentMainCode)
						{
							vec1<float> dataCond = vec1<float>(highTrigg - lowTrigg);
							for (int l = 0; l < (highTrigg - lowTrigg); l++)
							{
								float sum = 0.0;
								for (int m = 0; m < windowSam[1] - windowSam[0]; m++)
								{
									sum += data[lowTrigg + l][m];
								}
								dataCond[l] = sum / (windowSam[1] - windowSam[0]);
							}

							float mean = mean1DArray(&dataCond[0], dataCond.size());
							float devStd = stdDeviation(&dataCond[0], dataCond.size(), mean);
							float sem = devStd / sqrt(dataCond.size());
							float limCond = 1.96 * sem;

							erp[k] = mean;
							lim[k] = limCond;
						}
						else
						{
                            std::cout << "ATTENTION, PLEASE : NO EVENT WITH TYPE = "
                                << myprovFile->visuBlocs[k].mainEventBloc.eventCode[0] << std::endl;
						}
					}

					for (int k = 0; k < nbRow; k++)
					{
						erpP[k] = erp[k] + lim[k];
						erpM[k] = erp[k] - lim[k];
					}

					for (int k = 0; k < nbRow; k++)
					{
						erpPMax[k] = *(max_element(erpP.begin(), erpP.begin() + nbRow));
						erpMMax[k] = abs(*(min_element(erpM.begin(), erpM.begin() + nbRow)));
					}

					float maxP = *(max_element(erpPMax.begin(), erpPMax.begin() + nbRow));
					float maxM = *(max_element(erpMMax.begin(), erpMMax.begin() + nbRow));
                    float maxCurveLegend = std::max(maxP, maxM);

					painter->setPen(QColor(255, 0, 255, 255)); //pink petant
					painter->drawText((Width * 0.04) + ((0.296875 * Width) * (i % 3)), (Heigth * 0.107) + (Heigth * 0.0203) + 
									  (Heigth * 0.0406 * (j)), QString().setNum(round(maxCurveLegend)));
					painter->setPen(QColor(0, 0, 0, 255)); //noir

				   // 20 => (0.01041666 * Width) Scr width 1920
					float widthRect = (0.01041666 * Width);
                    float scaleFactor = widthRect / std::max((maxP / maxCurveLegend), (maxM / maxCurveLegend));

					//450 = (0.234375 * Width) Scr width 1920
					int coeffEsp = ceil((0.234375 * Width) / nbRow);
					double x = (Width * 0.0586) + ((0.296875 * Width) * (i % 3));
					double y = (Heigth * 0.107) + (Heigth * 0.0173) + (Heigth * 0.0406 * (j));

					bool firstE = false;
					for (int k = 0; k < nbRow; k++)
					{
						erp[k] = erp[k] / maxCurveLegend;
						lim[k] = lim[k] / maxCurveLegend;

						switch (k % 3) 
						{
						case 0:
							drawRectColor = Qt::GlobalColor::blue;
							break;
						case 1:
							drawRectColor = Qt::GlobalColor::red;
							break;
						case 2:
							drawRectColor = Qt::GlobalColor::green;
							break;
						default:
							break;
						}
						
						painter->fillRect(x + (coeffEsp * k), y, widthRect, -scaleFactor * (erp[k] - lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, widthRect, -scaleFactor * (erp[k] + lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, widthRect, -scaleFactor * erp[k], drawRectColor);

						for (int l = 0; l < significantValue.size(); l++)
						{
							//Stats draw
							if (((significantValue[l].elec == countBipole - 1) && (significantValue[l].window == k)))
							{
								if ((sizeSigCheck[l] == false))
								{
									if (firstE == false)
									{
										painter->setBrush(Qt::white);
										painter->drawEllipse(x - (0.028645833 * Width), y - (0.00615655233069 * Heigth), 
															(0.00677089 * Width), (0.0114335971855 * Heigth));
										firstE = true;
									}

									float x_Krus = (0.058333 * Width) + ((0.296875 * Width) * (i % 3)) + (coeffEsp * k);
									//if barplot is positiv , we need to move y up
									//otherwise it's just above the 0 line
									float y_Krus = y + (-scaleFactor * (erp[k] + lim[k])) - (0.00351802990325 * Heigth);
                                    y_Krus = std::min(y_Krus, (float)y - ((float)0.00351802990325 * Heigth));

									painter->setPen(QColor(255, 0, 255, 255)); //pink petant
									painter->drawLine(x_Krus, y_Krus, x_Krus + (0.00989583 * Width), y_Krus);
									painter->setPen(QColor(0, 0, 0, 255)); //noir

									sizeSigCheck[l] = true;
								}
							}
						}
					}

					painter->drawLine(x, y, x + (0.234375 * Width), y);
					//on remet en noir	
					painter->setPen(QColor(0, 0, 0, 255));
				}
			}
		}
	}

	pixmap->save(tifName, "JPG");

	delete windowSam;
}

QString InsermLibrary::DrawbarsPlots::drawBars::createPicPath(std::string picFolder, eegContainer* myeegContainer, int idElec)
{
	QString tifName = QString::fromStdString(picOutputFolder.c_str());
	if (myeegContainer->electrodes[idElec].label != "")
	{
		tifName.append(myeegContainer->electrodes[idElec].label.c_str()).append("_");
	}
	if (idElec + 1 < myeegContainer->electrodes.size())
	{
		if (myeegContainer->electrodes[idElec + 1].label != "")
		{
			tifName.append(myeegContainer->electrodes[idElec + 1].label.c_str()).append("_");
		}
	}
	if (idElec + 2 < myeegContainer->electrodes.size())
	{
		if (myeegContainer->electrodes[idElec + 2].label != "")
		{
			tifName.append(myeegContainer->electrodes[idElec + 2].label.c_str()).append("_");
		}
	}
	return tifName.append(".jpg");
}

InsermLibrary::DrawbarsPlots::drawPlots::drawPlots(InsermLibrary::PROV *myprovFile, std::string outputFolder, QSize size) : baseCanvas(myprovFile,
																					 size.width(),
																					 size.height())
{
	this->myprovFile = myprovFile;
	picOutputFolder = outputFolder;
}

InsermLibrary::DrawbarsPlots::drawPlots::~drawPlots()
{

}

//0 for mono, 1 for bipo , 2 for env2plot
void InsermLibrary::DrawbarsPlots::drawPlots::drawDataOnTemplate(vec3<float> &bigData, TriggerContainer* triggerContainer, eegContainer* myeegContainer, int card2Draw)
{
	int idCurrentElecDrawn = 0, nbFigureDrawn = 0, countBipole = 0;
	QPainter *painter = nullptr;
	QPixmap *pixmap = nullptr;

	int *windowSam = nullptr; 
	
	if (card2Draw == 2)
		windowSam = myprovFile->getBiggestWindowSam(myeegContainer->DownsampledFrequency());
	else
		windowSam = myprovFile->getBiggestWindowSam(myeegContainer->SamplingFrequency());
	
	int nbRow = myprovFile->nbRow();

	QString tifName = "";
	vec1<float> erp = vec1<float>(windowSam[1] - windowSam[0]);		vec1<float> erpPMax = vec1<float>(nbRow);
	vec1<float> std = vec1<float>(windowSam[1] - windowSam[0]);		vec1<float> erpMMax = vec1<float>(nbRow);
	vec1<float> sem = vec1<float>(windowSam[1] - windowSam[0]);
	vec1<float> lim = vec1<float>(windowSam[1] - windowSam[0]);

	vec2<float> data = vec2<float>(triggerContainer->ProcessedTriggerCount(), vec1<float>(windowSam[1] - windowSam[0]));
	vec2<float> m_erp = vec2<float>(nbRow, vec1<float>(windowSam[1] - windowSam[0]));
	vec2<float> m_lim = vec2<float>(nbRow, vec1<float>(windowSam[1] - windowSam[0]));
	vec2<float> m_erpP = vec2<float>(nbRow, vec1<float>(windowSam[1] - windowSam[0]));
	vec2<float> m_erpM = vec2<float>(nbRow, vec1<float>(windowSam[1] - windowSam[0]));


	std::vector<int> SubGroupStimTrials = triggerContainer->SubGroupStimTrials();
	std::vector<Trigger> Triggers = triggerContainer->ProcessedTriggers();
	for (int i = 0; i < myeegContainer->electrodes.size(); i++)
	{
		idCurrentElecDrawn = i % nbElecPerFigure;
		if (idCurrentElecDrawn == 0)
		{
			if (nbFigureDrawn > 0)
			{
				pixmap->save(tifName, "JPG");
			}
			nbFigureDrawn++;
			//==
			deleteAndNullify1D(painter);
			deleteAndNullify1D(pixmap);
			pixmap = new QPixmap(pixmapTemplate);
			painter = new QPainter(pixmap);
			painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
			painter->setFont(QFont("Arial", 12, 1, false));
			//==
			tifName = createPicPath(picOutputFolder, myeegContainer, card2Draw, nbFigureDrawn);
		}

		//Write Electrode Name
		painter->drawText((Width * 0.028) + 8 + ((0.296875 * Width) * (i % 3)), Heigth * 0.090,
			myeegContainer->electrodes[i].label.c_str());

		for (int j = 0; j < nbSite; j++)
		{
			bool isElecHere = false;
			if (card2Draw == 0)
			{
				isElecHere = j /*+ 1*/ <= myeegContainer->electrodes[i].id.size();
			}
			else
			{
				isElecHere = j + 1 < myeegContainer->electrodes[i].id.size();
			}

			if (isElecHere)
			{
				bool goIn = false;
				if (card2Draw == 0)
				{
					goIn = myeegContainer->flatElectrodes[countBipole] ==
                           myeegContainer->electrodes[i].label + std::to_string(myeegContainer->electrodes[i].id[j]);
				}
				else
				{
					goIn = myeegContainer->BipolePositivLabel(countBipole) ==
                        myeegContainer->electrodes[i].label + std::to_string(myeegContainer->electrodes[i].id[j + 1]);
				}

				if (goIn)
				{
					//Extract Data
					for (int k = 0; k < triggerContainer->ProcessedTriggerCount(); k++)
					{
						for (int l = 0; l < windowSam[1] - windowSam[0]; l++)
						{
							data[k][l] = bigData[k][countBipole][l];
						}
					}
					countBipole++;

					//Loop accross conditions
					for (int k = 0; k < nbRow; k++)
					{
						int lowTrigg = SubGroupStimTrials[k];
						int highTrigg = SubGroupStimTrials[k + 1] - 1;
						int currentMainCode = myprovFile->visuBlocs[k].mainEventBloc.eventCode[0];

						if (Triggers[lowTrigg].MainCode() == currentMainCode && Triggers[highTrigg].MainCode() == currentMainCode)
						{
							vec2<float> dataCond = vec2<float>(highTrigg - lowTrigg, vec1<float>(windowSam[1] - windowSam[0]));
							for (int l = 0; l < dataCond.size(); l++)
							{
								for (int m = 0; m < dataCond[l].size(); m++)
								{
									dataCond[l][m] = data[lowTrigg + l][m];
								}
							}

							for (int l = 0; l < windowSam[1] - windowSam[0]; l++)
							{
								float sumValues = 0.0;
								for (int m = 0; m < dataCond.size(); m++)
								{
									sumValues += dataCond[m][l];
								}
								erp[l] = sumValues / dataCond.size();

								float sumStd = 0;
								for (int m = 0; m < highTrigg - lowTrigg; m++)
								{
									sumStd += (dataCond[m][l] - erp[l]) * (dataCond[m][l] - erp[l]);
								}
								std[l] = sqrt(sumStd / (highTrigg - lowTrigg - 1));
								sem[l] = std[l] / sqrt(highTrigg - lowTrigg);
								lim[l] = 1.96 * sem[l];
								m_erp[k][l] = erp[l];
								m_lim[k][l] = lim[l];
							}
						}
						else
						{
                            std::cout << "ATTENTION, PLEASE : NO EVENT WITH TYPE = "
                                << myprovFile->visuBlocs[k].mainEventBloc.eventCode[0] << std::endl;
						}
					}


					//we remove the same value to all the erp's, 
					//so that it is more or less centered on zero at the first sample
					float meanErp = 0.0;
					for (int k = 0; k < m_erp.size(); k++)
					{
						meanErp += m_erp[k][0];
					}
					meanErp /= m_erp.size();

					for (int k = 0; k < windowSam[1] - windowSam[0]; k++)
					{
						for (int l = 0; l < nbRow; l++)
						{
							m_erp[l][k] -= meanErp;
						}
					}

					for (int k = 0; k < nbRow; k++)
					{
						for (int l = 0; l < windowSam[1] - windowSam[0]; l++)
						{
							m_erpP[k][l] = m_erp[k][l] + m_lim[k][l];
							m_erpM[k][l] = m_erp[k][l] - m_lim[k][l];
						}
					}

					for (int k = 0; k < nbRow; k++)
					{
						erpPMax[k] = *(max_element(m_erpP[k].begin(), m_erpP[k].begin() + (windowSam[1] - windowSam[0])));
						erpMMax[k] = *(max_element(m_erpM[k].begin(), m_erpM[k].begin() + (windowSam[1] - windowSam[0])));
					}

					float maxP = *(max_element(erpPMax.begin(), erpPMax.begin() + nbRow));
					float maxM = *(max_element(erpMMax.begin(), erpMMax.begin() + nbRow));
                    float maxCurveLegend = std::max(maxP, maxM);

					painter->setPen(QColor(255, 0, 255, 255)); //pink petant
					painter->drawText((Width * 0.04) + ((0.296875 * Width) * (i % 3)), (Heigth * 0.107) + (Heigth * 0.0203) +
						(Heigth * 0.0406 * (j)), QString().setNum(round(maxCurveLegend)));
					painter->setPen(QColor(0, 0, 0, 255)); //noir

					for (int k = 0; k < nbRow; k++)
					{
						QPainterPath qpath, qpathP, qpathM;
						float maxRow = *(max_element(m_erp[k].begin(), m_erp[k].begin() + (windowSam[1] - windowSam[0])));
						float minRow = *(min_element(m_erp[k].begin(), m_erp[k].begin() + (windowSam[1] - windowSam[0])));

						for (int l = 0; l < windowSam[1] - windowSam[0]; l++)
						{
							erp[l] = m_erp[k][l] / maxCurveLegend;
							lim[l] = m_lim[k][l] / maxCurveLegend;

							float beginningPosition = (Width * 0.0586) + ((0.296875 * Width) * (i % 3));
							float nextElec = (0.25 * Width / nbColLegend) * (k / 3);
									  /*beg Pos       + next elec Pos  +  data (next) column*/
							float x = beginningPosition + nextElec + ((l *(0.25 * Width / nbColLegend)) / (windowSam[1] - windowSam[0]));

							/*y = beginning pos - data + plot elec*/
							float y = ((0.1143359718557 * Heigth) - ((0.021987686895 * Heigth) * erp[l])) + (Heigth * 0.0406 * (j));
							float yP = ((0.1143359718557 * Heigth) - ((0.021987686895 * Heigth)  * (erp[l] + lim[l])) + (Heigth * 0.0406 * (j)));
							float yM = ((0.1143359718557 * Heigth) - ((0.021987686895 * Heigth)  * (erp[l] - lim[l])) + (Heigth * 0.0406 * (j)));

							if (l == 0)
							{
								qpath.moveTo(QPointF(x, y));
								qpathP.moveTo(QPointF(x, yP));
								qpathM.moveTo(QPointF(x, yM));
							}
							else
							{
								qpath.lineTo(QPointF(x, y));
								qpathP.lineTo(QPointF(x, yP));
								qpathM.lineTo(QPointF(x, yM));
							}

							switch (k % 3)
							{
							case 0:
								painter->setPen(QColor(0, 0, 255, 255)); //blue	
								break;
							case 1:
								painter->setPen(QColor(255, 0, 0, 255)); //red		
								break;
							case 2:
								painter->setPen(QColor(0, 255, 0, 255)); //green
								break;
							default:
								painter->setPen(QColor(0, 0, 0, 255)); //black
								break;
							}

							painter->drawPath(qpath);
							painter->drawPath(qpathP);
							painter->drawPath(qpathM);
						}
					}
					painter->setPen(QColor(0, 0, 0, 255));
				}
			}
		}
	}

	pixmap->save(tifName, "JPG");

	delete windowSam;
}

QString InsermLibrary::DrawbarsPlots::drawPlots::createPicPath(std::string picFolder, eegContainer* myeegContainer, int cards2Draw, int nbFigureDrawn)
{
	QString tifName = QString::fromStdString(picOutputFolder.c_str());

	switch (cards2Draw)
	{
	case 0:
        tifName.append("_erp_mono_f").append(std::to_string(nbFigureDrawn).c_str());
		break;
	case 1:
        tifName.append("_erp_bipo_f").append(std::to_string(nbFigureDrawn).c_str());
		break;
	case 2:
        tifName.append("f").append(std::to_string(nbFigureDrawn).c_str());
		break;
	}

	return tifName.append(".jpg");
}
