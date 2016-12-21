#include "barsGenerator.h"

InsermLibrary::DrawPlotsVisu::baseCanvas::baseCanvas(PROV *p_prov)
{
	drawTemplate(p_prov);
}

InsermLibrary::DrawPlotsVisu::baseCanvas::~baseCanvas()
{

}

void InsermLibrary::DrawPlotsVisu::baseCanvas::drawTemplate(PROV *p_prov)
{
	screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	screenHeigth = GetSystemMetrics(SM_CYFULLSCREEN);

	pixmapTemplate = QPixmap(screenWidth, screenHeigth);
	pixmapTemplate.fill(QColor(Qt::white));
	QPainter painter(&pixmapTemplate);
	painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
	painter.setFont(QFont("Arial", 12, 1, false));

	nbColLegend = ceil((double)p_prov->nbRow() / 3);
	for (int i = 0; i < nbElecPerFigure; i++)
	{
		//Write legend
		int s_x = screenWidth * 0.0586 + (570 * (i % 3));
		int s_y = screenHeigth * 0.0325;

		for (int j = 0; j < p_prov->nbRow(); j++)
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
			painter.drawText(s_x, s_y, p_prov->visuBlocs[j].mainEventBloc.eventLabel.c_str());
			s_y = s_y + 25;

			if ((j + 1) % 3 == 0)
			{
				s_x = s_x + (480 / nbColLegend); 
				s_y = screenHeigth * 0.0325;
			}
		}

		//On repasse en noir
		painter.setPen(QColor(0, 0, 0, 255)); //black

		//DrawElec
		QRect recElec((screenWidth * 0.028) + ((570) * (i % 3)), screenHeigth * 0.094, screenWidth * 0.0105, screenHeigth * 0.812);
		painter.drawRect(recElec);
		painter.fillRect(recElec, QColor(255, 255, 255, 128));

		for (int j = 0; j < nbSite; j++)
		{
			painter.fillRect((screenWidth * 0.028) + (570 * (i % 3)), (screenHeigth * 0.094) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), screenWidth * 0.0105, screenHeigth * 0.0203, QColor(0, 0, 0, 255));
			painter.drawText((screenWidth * 0.015) + (570 * (i % 3)), (screenHeigth * 0.107) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), QString().setNum(j + 1));
		}
	}
}

//===

InsermLibrary::DrawPlotsVisu::drawBars::drawBars(PROV *p_prov) : baseCanvas(p_prov)
{
	currentProv = p_prov;
}

InsermLibrary::DrawPlotsVisu::drawBars::~drawBars()
{

}

void InsermLibrary::DrawPlotsVisu::drawBars::drawDataOnTemplate(ELAN *p_elan, LOCAANALYSISOPTION *p_anaopt, 
																int currentFreqBand, double *** bigdata, 
																vector<int> p_eventsUsed, vector<PVALUECOORD> p_significantValue)
{
	string a_rt;
	vector<int> v_id, v_origid, v_f;
	int compteurElec = 0, s_pos = 0, s_figure = 0, compteur = 0;
	stringstream path, expTask, tifNameStream;
	QString tifName;
	QPainter *painter = nullptr;
	QPixmap *pixmap = nullptr;
	bool goIn = false;
	int windowSample[2]{ (int) round(64 * currentProv->visuBlocs[0].dispBloc.epochWindow[0] / 1000), 
						 (int) round(64 * currentProv->visuBlocs[0].dispBloc.epochWindow[1] / 1000) };

	double *v_erp = allocate1DArray<double>(currentProv->nbRow());
	double *v_lim = allocate1DArray<double>(currentProv->nbRow());
	double *m_erpP = allocate1DArray<double>(currentProv->nbRow());
	double *m_erpM = allocate1DArray<double>(currentProv->nbRow());
	double *m_erpPMax = allocate1DArray<double>(currentProv->nbRow());
	double *m_erpMMax = allocate1DArray<double>(currentProv->nbRow());
	bool *sizeSigcheck = allocate1DArray<bool>(p_significantValue.size());

	double **data = allocate2DArray<double>(p_eventsUsed.size(), (windowSample[1] - windowSample[0]) + 1);

	for (int i = 0; i < p_elan->ss_elec->a_rt.size(); i++)	
	{
		a_rt = p_elan->ss_elec->a_rt[i];
		v_id = p_elan->ss_elec->v_id[i];
		v_origid = p_elan->ss_elec->v_origid[i];

		if (i > 0)
		{
			compteurElec += p_elan->ss_elec->v_id[i - 1].size() - 1;
		}

		s_pos = i % nbElecPerFigure;
	
		if (s_pos == 0) //On a finis une série de 3, on enregistre l'image
		{
			if (s_figure > 0)
			{
				pixmap->save(tifName, "JPG");
			}
			s_figure = s_figure + 1;

			deleteAndNullify1D(painter);
			deleteAndNullify1D(pixmap);
			pixmap = new QPixmap(pixmapTemplate);
			painter = new QPainter(pixmap);
			painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
			painter->setFont(QFont("Arial", 12, 1, false));

			stringstream().swap(path);
			path << p_anaopt->expTask << "_f" << p_anaopt->frequencys[currentFreqBand][0] << "f"
				 << p_anaopt->frequencys[currentFreqBand][p_anaopt->frequencys[currentFreqBand].size() - 1] << "_ds"
				 << (p_elan->trc->samplingFrequency / 64) << "_sm0"; 
				
			stringstream().swap(expTask);
			expTask << p_anaopt->patientFolder << "/" << p_anaopt->expTask;

			stringstream().swap(tifNameStream);
			tifNameStream << expTask.str() << "/" << path.str() << "_bar_f" << s_figure << ".jpg";
			tifName = &tifNameStream.str()[0];
		}

		//écris le nom de l'éléc
		painter->drawText((screenWidth * 0.028) + 8 + (570 * (i % 3)), screenHeigth * 0.090, a_rt.c_str());

		for (int j = 0; j < nbSite; j++)
		{
			v_f = findIndexes(&v_id[0], v_id.size(), j + 1);
			goIn = (v_f.empty() == false) && (v_f[0] + 1 < v_id.size());

			if (goIn == true)
			{
				//then we can extract the data
				for (int k = 0; k < p_eventsUsed.size(); k++)
				{
					for (int l = 0; l < (windowSample[1] - windowSample[0]) + 1; l++)
					{
						data[k][l] = bigdata[k][compteur][l];
					}
				}
				compteur++;

				//then we loop across conditions
				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					v_f = findIndexes(&p_eventsUsed[0], p_eventsUsed.size(), 
									   currentProv->visuBlocs[k].mainEventBloc.eventCode[0]);

					if (v_f.empty() == false)
					{
						double *v_data_e = allocate1DArray<double>(v_f.size());
						for (int l = 0; l < v_f.size(); l++)
						{
							double tempMean = 0;
							for (int m = 0; m < (windowSample[1] - windowSample[0]) + 1; m++)
							{
								tempMean += data[v_f[l]][m];
							}
							v_data_e[l] = tempMean / ((windowSample[1] - windowSample[0]) + 1);
							v_data_e[l] = (v_data_e[l] - 1000) / 10;
						}

						double s_mean = mean1DArray(v_data_e, v_f.size());
						double s_std = stdDeviation(v_data_e, v_f.size(), s_mean);
						double s_sem = s_std / sqrt(v_f.size());
						double s_lim = 1.96 * s_sem;
						v_erp[k] = s_mean;
						v_lim[k] = s_lim;
						deAllocate1DArray(v_data_e);
					}
					else
					{
						cout << "ATTENTION, PLEASE : NO EVENT WITH TYPE = " 
						     << currentProv->visuBlocs[k].mainEventBloc.eventCode[0] << endl;
					}
				}

				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					m_erpP[k] = v_erp[k] + v_lim[k];
					m_erpM[k] = v_erp[k] - v_lim[k];
				}

				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					m_erpPMax[k] = *(max_element(m_erpP, m_erpP + currentProv->nbRow()));
					m_erpMMax[k] = abs(*(min_element(m_erpM, m_erpM + currentProv->nbRow())));
				}

				double tempMax1 = *(max_element(m_erpPMax, m_erpPMax + currentProv->nbRow()));
				double tempMax2 = *(max_element(m_erpMMax, m_erpMMax + currentProv->nbRow()));

				double maxCurveLegend = max(tempMax1, tempMax2);

				painter->setPen(QColor(255, 0, 255, 255)); //pink petant
				painter->drawText((screenWidth * 0.04) + (570 * (i % 3)), (screenHeigth * 0.107) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), QString().setNum(round(maxCurveLegend)));
				painter->setPen(QColor(0, 0, 0, 255)); //noir

				int coeffEsp = ceil(450 / currentProv->nbRow());
				double x = (screenWidth * 0.0586) + (570 * (i % 3));
				double y = (screenHeigth * 0.107) + (screenHeigth * 0.0173) + (screenHeigth * 0.0406 * (j));
				double x_Krus = 0;
				double y_Krus = 0;

				double ma = *(max_element(&m_erpPMax[0], &m_erpPMax[0] + currentProv->nbRow()));
				double mi = abs(*(min_element(&m_erpMMax[0], &m_erpMMax[0] + currentProv->nbRow())));

				ma = ma / maxCurveLegend;
				mi = mi / maxCurveLegend;

				double scaleFactor = 20 / max(ma, mi);

				bool firstE = false;
				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					v_erp[k] = v_erp[k] / maxCurveLegend;
					v_lim[k] = v_lim[k] / maxCurveLegend;

					switch (k % 3)
					{
					case 0:
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] - v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] + v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * v_erp[k], Qt::GlobalColor::blue); //blue //carré de 20 par 20 max
						break;
					case 1:
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] - v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] + v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * v_erp[k], Qt::GlobalColor::red); //red //carré de 20 par 20 max
						break;
					case 2:
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] - v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * (v_erp[k] + v_lim[k]), Qt::GlobalColor::gray);
						painter->fillRect(x + (coeffEsp * k), y, 20, -scaleFactor * v_erp[k], Qt::GlobalColor::green); //green //carré de 20 par 20 max
						break;
					default:
						break;
					}

					for (int z = 0; z < p_significantValue.size(); z++)
					{
						if (((p_significantValue[z].elec == compteurElec + j) && (p_significantValue[z].window == k)))
						{
							if ((sizeSigcheck[z] == false))
							{
								if (firstE == false)
								{
									painter->setBrush(Qt::white);
									painter->drawEllipse(x - 55, y - 7, 13, 13);
									firstE = true;
								}

								x_Krus = 112 + (570 * (i % 3)) + (coeffEsp * k);
								y_Krus = y + (-scaleFactor * (v_erp[k] + v_lim[k])) - 4;
								painter->setPen(QColor(255, 0, 255, 255)); //pink petant
								painter->drawLine(x_Krus, y_Krus, x_Krus + 19, y_Krus);
								painter->setPen(QColor(0, 0, 0, 255)); //noir

								sizeSigcheck[z] = true;
							}
						}
					}

				}
				painter->drawLine(x, y, x + 450, y);
				//on remet en noir	
				painter->setPen(QColor(0, 0, 0, 255));
			}
		}
	}

	pixmap->save(tifName, "JPG");

	deAllocate1DArray(v_erp);
	deAllocate1DArray(v_lim);
	deAllocate1DArray(m_erpP);
	deAllocate1DArray(m_erpM);
	deAllocate1DArray(m_erpPMax);
	deAllocate1DArray(m_erpMMax);
	deAllocate1DArray(sizeSigcheck);
	deAllocate2DArray(data, p_eventsUsed.size());
}

//===

InsermLibrary::DrawPlotsVisu::drawPlots::drawPlots(PROV *p_prov) : baseCanvas(p_prov)
{
	currentProv = p_prov;
}

InsermLibrary::DrawPlotsVisu::drawPlots::~drawPlots()
{

}

//0 for mono, 1 for bipo , 2 for env2plot
void InsermLibrary::DrawPlotsVisu::drawPlots::drawDataOnTemplate(ELAN *p_elan, LOCAANALYSISOPTION *p_anaopt, 
																 double *** bigdata, vector<int> p_eventsUsed, 
																 int currentFreqBand, int cards2Draw)
{
	string a_rt;
	vector<int> v_id, v_origid, v_f;
	int compteurElec = 0, s_pos = 0, s_figure = 0, compteur = 0, samplingFreq = 0;
	stringstream  path, expTask, tifNameStream;
	QString tifName;
	QPainter *painter = nullptr;
	QPixmap *pixmap = nullptr;
	bool goIn = false;
	
	if (cards2Draw == 2)
	{
		samplingFreq = p_elan->elanFreqBand[currentFreqBand]->eeg.sampling_freq;
	}
	else
	{
		samplingFreq = p_elan->trc->samplingFrequency;
	}

	int windowSample[2]{ (int) round(samplingFreq * currentProv->visuBlocs[0].dispBloc.epochWindow[0] / 1000),
						 (int) round(samplingFreq * currentProv->visuBlocs[0].dispBloc.epochWindow[1] / 1000) };

	double *v_erp = allocate1DArray<double>((windowSample[1] - windowSample[0]) + 1);
	double *v_std = allocate1DArray<double>((windowSample[1] - windowSample[0]) + 1);
	double *v_sem = allocate1DArray<double>((windowSample[1] - windowSample[0]) + 1);
	double *v_lim = allocate1DArray<double>((windowSample[1] - windowSample[0]) + 1);
	double *m_erpPMax = allocate1DArray<double>(currentProv->nbRow());
	double *m_erpMMax = allocate1DArray<double>(currentProv->nbRow());
	double **data = allocate2DArray<double>(p_eventsUsed.size(), (windowSample[1] - windowSample[0]) + 1);
	double **m_erp = allocate2DArray<double>(currentProv->nbRow(), (windowSample[1] - windowSample[0]) + 1);
	double **m_lim = allocate2DArray<double>(currentProv->nbRow(), (windowSample[1] - windowSample[0]) + 1);
	double **m_erpP = allocate2DArray<double>(currentProv->nbRow(), (windowSample[1] - windowSample[0]) + 1);
	double **m_erpM = allocate2DArray<double>(currentProv->nbRow(), (windowSample[1] - windowSample[0]) + 1);

	for (int i = 0; i < p_elan->ss_elec->a_rt.size(); i++)
	{
		a_rt = p_elan->ss_elec->a_rt[i];
		v_id = p_elan->ss_elec->v_id[i];
		v_origid = p_elan->ss_elec->v_origid[i];

		if (i > 0)
		{
			compteurElec += p_elan->ss_elec->v_id[i - 1].size() - 1;
		}

		s_pos = i % nbElecPerFigure;

		if (s_pos == 0) //On a finis une série de 3, on enregistre l'image
		{
			if (s_figure > 0)
			{
				pixmap->save(tifName, "JPG");
			}
			s_figure = s_figure + 1;

			deleteAndNullify1D(painter);
			deleteAndNullify1D(pixmap);
			pixmap = new QPixmap(pixmapTemplate);
			painter = new QPainter(pixmap);
			painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
			painter->setFont(QFont("Arial", 12, 1, false));

			stringstream().swap(expTask);
			expTask << p_anaopt->patientFolder << "/" << p_anaopt->expTask;

			stringstream().swap(tifNameStream);
			switch (cards2Draw)
			{
			case 0:
				tifNameStream << expTask.str() << "/" << p_anaopt->expTask << "_erp_mono_f" << s_figure << ".jpg";
				break;
			case 1:
				tifNameStream << expTask.str() << "/" << p_anaopt->expTask << "_erp_bipo_f" << s_figure << ".jpg";
				break;
			case 2:
				stringstream().swap(path);
				path << p_anaopt->expTask << "_f" << p_anaopt->frequencys[currentFreqBand][0] << "f"
					 << p_anaopt->frequencys[currentFreqBand][p_anaopt->frequencys[currentFreqBand].size() - 1] << "_ds"
					 << (p_elan->trc->samplingFrequency / 64) << "_sm0";
				tifNameStream << expTask.str() << "/" << path.str() << "_plot_f" << s_figure << ".jpg";
				break;
			default:
				cout << "Attention Erreur" << endl;
				break;
			}
			tifName = &tifNameStream.str()[0];
		}

		//écris le nom de l'éléc
		painter->drawText((screenWidth * 0.028) + 8 + (570 * (i % 3)), screenHeigth * 0.090, a_rt.c_str());

		for (int j = 0; j < nbSite; j++)
		{
			v_f = findIndexes(&v_id[0], v_id.size(), j + 1);
			switch (cards2Draw)
			{
			case 0:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size());
				break;
			case 1:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size() - 1);
				break;
			case 2:
				goIn = (v_f.empty() == false) && (v_f[0] + 1 <= v_id.size() - 1);
				break;
			}

			if (goIn == true)
			{
				//then we can extract the data
				for (int k = 0; k < p_eventsUsed.size(); k++)
				{
					for (int l = 0; l < (windowSample[1] - windowSample[0]) + 1; l++)
					{
						data[k][l] = bigdata[k][compteur][l];
					}
				}
				compteur++;

				//then we loop across conditions
				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					v_f = findIndexes(&p_eventsUsed[0], p_eventsUsed.size(),
						currentProv->visuBlocs[k].mainEventBloc.eventCode[0]);


					if (v_f.empty() == false)
					{
						double **m_data_se = allocate2DArray<double>(v_f.size(), (windowSample[1] - windowSample[0]) + 1);

						for (int l = 0; l < v_f.size(); l++)
						{
							for (int m = 0; m < (windowSample[1] - windowSample[0]) + 1; m++)
							{
								m_data_se[l][m] = data[v_f[l]][m];
								m_data_se[l][m] = (m_data_se[l][m] - 1000) / 10;
							}
						}

						for (int l = 0; l < (windowSample[1] - windowSample[0]) + 1; l++)
						{
							v_erp[l] = mean2DArray(m_data_se, v_f.size(), l);

							double tempStd = 0;
							for (int m = 0; m < v_f.size(); m++)
							{
								tempStd += (m_data_se[m][l] - v_erp[l]) * (m_data_se[m][l] - v_erp[l]);
							}

							if (v_f.size() - 1 == 0)
							{
								v_std[l] = 0;
							}
							else
							{
								v_std[l] = sqrt(tempStd / (v_f.size() - 1));
							}

							v_sem[l] = v_std[l] / sqrt(v_f.size());
							v_lim[l] = 1.96*v_sem[l];
							m_erp[k][l] = v_erp[l];
							m_lim[k][l] = v_lim[l];
						}

						deAllocate2DArray(m_data_se, v_f.size());
					}
					else
					{
						cout << "ATTENTION, PLEASE : NO EVENT WITH TYPE = " 
						     << currentProv->visuBlocs[k].mainEventBloc.eventCode[0] << endl;
					}
				}

				//m_erp = m_erp - mean(m_erp(1, :)); % we remove the same value to all the erp's, 
				//so that it is more or less centered on zero at the first sample
				double meanErp = mean2DArray(m_erp, currentProv->nbRow(), 0);
				for (int k = 0; k < (windowSample[1] - windowSample[0]) + 1; k++)
				{
					for (int l = 0; l < currentProv->nbRow(); l++)
					{
						m_erp[l][k] -= meanErp;
					}
				}

				//m_erp + lim et m_erp - lim
				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					for (int l = 0; l < (windowSample[1] - windowSample[0]) + 1; l++)
					{
						m_erpP[k][l] = m_erp[k][l] + m_lim[k][l];
						m_erpM[k][l] = m_erp[k][l] - m_lim[k][l];
					}
				}

				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					m_erpPMax[k] = *(max_element(m_erpP[k], m_erpP[k] + (windowSample[1] - windowSample[0]) + 1));
					m_erpMMax[k] = *(max_element(m_erpM[k], m_erpM[k] + (windowSample[1] - windowSample[0]) + 1));
				}

				double tempMax1 = *(max_element(m_erpPMax, m_erpPMax + currentProv->nbRow()));
				double tempMax2 = *(max_element(m_erpMMax, m_erpMMax + currentProv->nbRow()));
				double maxCurveLegend = max(tempMax1, tempMax2);

				painter->setPen(QColor(255, 0, 255, 255)); //pink petant
				painter->drawText((screenWidth * 0.04) + (570 * (i % 3)), (screenHeigth * 0.107) + (screenHeigth * 0.0203) + (screenHeigth * 0.0406 * (j)), QString().setNum(round(maxCurveLegend)));
				painter->setPen(QColor(0, 0, 0, 255)); //noir

				for (int k = 0; k < currentProv->nbRow(); k++)
				{
					QPainterPath qpath, qpathP, qpathM;

					double mAAx = *(max_element(m_erp[k], m_erp[k] + (windowSample[1] - windowSample[0]) + 1));
					double mIIn = *(min_element(m_erp[k], m_erp[k] + (windowSample[1] - windowSample[0]) + 1));
					for (int l = 0; l < (windowSample[1] - windowSample[0]) + 1; l++)
					{
						v_erp[l] = m_erp[k][l] / maxCurveLegend;
						v_lim[l] = m_lim[k][l] / maxCurveLegend;

						/*x = position de départ   +  prochaine éléctrode   +  data (prochaine) colonne*/
						double x = (screenWidth * 0.0586) + (570 * (i % 3)) + ((480 / nbColLegend) * (k / 3)) + ((l *(480 / nbColLegend)) / ((windowSample[1] - windowSample[0]) + 1));

						/*y = position de départ - data + plot éléctrode*/
						double y = (130 - ((25) * v_erp[l])) + (screenHeigth * 0.0406 * (j));
						double yP = (130 - ((25)  * (v_erp[l] + v_lim[l])) + (screenHeigth * 0.0406 * (j)));
						double yM = (130 - ((25)  * (v_erp[l] - v_lim[l])) + (screenHeigth * 0.0406 * (j)));

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
							painter->drawPath(qpath);
							painter->drawPath(qpathP);	
							painter->drawPath(qpathM);	
							break;
						case 1:		
							painter->setPen(QColor(255, 0, 0, 255)); //red		
							painter->drawPath(qpath);
							painter->drawPath(qpathP);	
							painter->drawPath(qpathM);
							break;	
						case 2:	
							painter->setPen(QColor(0, 255, 0, 255)); //green
							painter->drawPath(qpath);	
							painter->drawPath(qpathP);
							painter->drawPath(qpathM);
							break;		
						default:	
							painter->setPen(QColor(0, 0, 0, 255)); //black
							break;				
						}	
					}
				}

				painter->setPen(QColor(0, 0, 0, 255));
			}//end if go in
		}
	}

	pixmap->save(tifName, "JPG");

	deAllocate1DArray(v_erp);
	deAllocate1DArray(v_std);
	deAllocate1DArray(v_sem);
	deAllocate1DArray(v_lim);
	deAllocate1DArray(m_erpPMax);
	deAllocate1DArray(m_erpMMax);
	deAllocate2DArray(data, p_eventsUsed.size());
	deAllocate2DArray(m_erp, currentProv->nbRow());
	deAllocate2DArray(m_lim, currentProv->nbRow());
	deAllocate2DArray(m_erpP, currentProv->nbRow());
	deAllocate2DArray(m_erpM, currentProv->nbRow());
}