#include "Stats.h"

vector<vector<vector<double>>> InsermLibrary::Stats::pValuesWilcoxon(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***bigdata)
{
	int numberCol = p_prov->nbCol();
	int numberRow = p_prov->nbRow();
	//===
	int v_window_ms[2];
	int mini = 0, Maxi = 0;
	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		mini = min(mini, p_prov->visuBlocs[i].dispBloc.epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i].dispBloc.epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	//===

	vector<vector<vector<double>>> p_value3D;
	vector<vector<vector<int>>> signe3D;
	for (int i = 0; i < p_elan_struct->chan_nb; i++)
	{
		vector<vector<double>> p_valueBig;
		vector<vector<int>> p_signeBig;

		for (int j = 0; j < numberCol; j++)
		{
			for (int k = 0; k < numberRow; k++)
			{
				for (int z = 0; z < p_prov->visuBlocs.size(); z++)
				{
					if (p_prov->visuBlocs[z].dispBloc.col == j + 1)
					{
						if (p_prov->visuBlocs[z].dispBloc.row == k + 1)
						{
							int a = triggCatEla->mainGroupSub[correspEvent[z]];
							int b = triggCatEla->mainGroupSub[correspEvent[z] + 1];
							int numberSubTrial = b - a;

							vector<double> baseLineData, eegData;
							vector<vector<double>> eegDataBig;
							double temp = 0, temp2 = 0;

							for (int l = 0; l < numberSubTrial; l++)
							{
								int baselineDebut = round((64 * (p_prov->visuBlocs[z].dispBloc.baseLineWindow[0] - p_prov->visuBlocs[z].dispBloc.epochWindow[0])) / 1000);
								int baselineFin = round((64 * (p_prov->visuBlocs[z].dispBloc.baseLineWindow[1] - p_prov->visuBlocs[z].dispBloc.epochWindow[0])) / 1000);

								for (int m = 0; m < (baselineFin - baselineDebut); m++)
								{
									temp += bigdata[i][triggCatEla->trigg[a + l].origPos][baselineDebut + m];
								}
								baseLineData.push_back(temp / (baselineFin - baselineDebut));
								temp = 0;
							}

							int numberWin = 0;
							if (p_prov->invertmapsinfo != "")
							{
								numberWin = (p_prov->visuBlocs[z].dispBloc.epochWindow[1] - p_prov->visuBlocs[z].dispBloc.epochWindow[0]) / (200 / 2);  // 200/2 car overlap 50% 
							}
							else
							{
								numberWin = p_prov->visuBlocs[z].dispBloc.epochWindow[1] / (200 / 2);  // 200/2 car overlap 50% 
							}

							for (int n = 0; n < numberWin - 1; n++)
							{

								for (int l = 0; l < numberSubTrial; l++)
								{
									//boucle moyenne des X fenetres que l'on veut (même taille que base line)

									int winDebut = round((64 * (0 + (100 * n) + (p_prov->visuBlocs[z].dispBloc.epochWindow[0] - v_window_ms[0]))) / 1000);
									int winFin = round((64 * (200 + (100 * n) + (p_prov->visuBlocs[z].dispBloc.epochWindow[0] - v_window_ms[0]))) / 1000);

									for (int m = 0; m < (winFin - winDebut); m++)
									{
										temp += bigdata[i][triggCatEla->trigg[a + l].origPos][winDebut + m];
									}
									eegData.push_back(temp / (winFin - winDebut));
									temp = 0;
								}
								eegDataBig.push_back(eegData);
								eegData.clear();
							}

							vector<double> p_value;
							for (int l = 0; l < eegDataBig.size(); l++)
							{
								p_value.push_back(wilcoxon(baseLineData, eegDataBig[l]));
							}
							p_valueBig.push_back(p_value);

							vector<int> valueSigne;
							for (int l = 0; l < eegDataBig.size(); l++)
							{
								double temp = 0;
								int val2go = 0;
								double meanBL = 0, meanWIN = 0;
								for (int m = 0; m < eegDataBig[l].size(); m++)
								{
									meanBL += baseLineData[m];
									meanWIN += eegDataBig[l][m];
								}
								meanBL /= eegDataBig[l].size();
								meanWIN /= eegDataBig[l].size();

								temp = meanWIN - meanBL;
								if (temp < 0)
								{
									val2go -= 1;
								}
								else if (temp == 0)
								{

								}
								else if (temp > 0)
								{
									val2go += 1;
								}
								valueSigne.push_back(val2go);
							}
							p_signeBig.push_back(valueSigne);
						}
					}
				}
			}
		}
		signe3D.push_back(p_signeBig);
		p_value3D.push_back(p_valueBig);
	}
	return p_value3D;
}

vector<vector<vector<int>>> InsermLibrary::Stats::signWilcoxon(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***bigdata)
{
	int numberCol = p_prov->nbCol();
	int numberRow = p_prov->nbRow();
	//===
	int v_window_ms[2];
	int mini = 0, Maxi = 0;
	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		mini = min(mini, p_prov->visuBlocs[i].dispBloc.epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i].dispBloc.epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	//===

	vector<vector<vector<int>>> signe3D;
	for (int i = 0; i < p_elan_struct->chan_nb; i++)
	{
		vector<vector<int>> p_signeBig;

		for (int j = 0; j < numberCol; j++)
		{
			for (int k = 0; k < numberRow; k++)
			{
				for (int z = 0; z < p_prov->visuBlocs.size(); z++)
				{
					if (p_prov->visuBlocs[z].dispBloc.col == j + 1)
					{
						if (p_prov->visuBlocs[z].dispBloc.row == k + 1)
						{
							int a = triggCatEla->mainGroupSub[correspEvent[z]];
							int b = triggCatEla->mainGroupSub[correspEvent[z] + 1];
							int numberSubTrial = b - a;

							vector<double> baseLineData, eegData;
							vector<vector<double>> eegDataBig;
							double temp = 0, temp2 = 0;

							for (int l = 0; l < numberSubTrial; l++)
							{
								int baselineDebut = round((64 * (p_prov->visuBlocs[z].dispBloc.baseLineWindow[0] - p_prov->visuBlocs[z].dispBloc.epochWindow[0])) / 1000);
								int baselineFin = round((64 * (p_prov->visuBlocs[z].dispBloc.baseLineWindow[1] - p_prov->visuBlocs[z].dispBloc.epochWindow[0])) / 1000);

								for (int m = 0; m < (baselineFin - baselineDebut); m++)
								{
									temp += bigdata[i][triggCatEla->trigg[a + l].origPos][baselineDebut + m];
								}
								baseLineData.push_back(temp / (baselineFin - baselineDebut));
								temp = 0;
							}

							int numberWin = 0;
							if (p_prov->invertmapsinfo != "")
							{
								numberWin = (p_prov->visuBlocs[z].dispBloc.epochWindow[1] - p_prov->visuBlocs[z].dispBloc.epochWindow[0]) / (200 / 2);  // 200/2 car overlap 50% 
							}
							else
							{
								numberWin = p_prov->visuBlocs[z].dispBloc.epochWindow[1] / (200 / 2);  // 200/2 car overlap 50% 
							}

							for (int n = 0; n < numberWin - 1; n++)
							{

								for (int l = 0; l < numberSubTrial; l++)
								{
									//boucle moyenne des X fenetres que l'on veut (même taille que base line)

									int winDebut = round((64 * (0 + (100 * n) + (p_prov->visuBlocs[z].dispBloc.epochWindow[0] - v_window_ms[0]))) / 1000);
									int winFin = round((64 * (200 + (100 * n) + (p_prov->visuBlocs[z].dispBloc.epochWindow[0] - v_window_ms[0]))) / 1000);

									for (int m = 0; m < (winFin - winDebut); m++)
									{
										temp += bigdata[i][triggCatEla->trigg[a + l].origPos][winDebut + m];
									}
									eegData.push_back(temp / (winFin - winDebut));
									temp = 0;
								}
								eegDataBig.push_back(eegData);
								eegData.clear();
							}

							vector<int> valueSigne;
							for (int l = 0; l < eegDataBig.size(); l++)
							{
								double temp = 0;
								int val2go = 0;
								double meanBL = 0, meanWIN = 0;
								for (int m = 0; m < eegDataBig[l].size(); m++)
								{
									meanBL += baseLineData[m];
									meanWIN += eegDataBig[l][m];
								}
								meanBL /= eegDataBig[l].size();
								meanWIN /= eegDataBig[l].size();

								temp = meanWIN - meanBL;

								if (temp < 0)
								{
									val2go -= 1;
								}
								else if (temp == 0)
								{

								}
								else if (temp > 0)
								{
									val2go += 1;
								}
								valueSigne.push_back(val2go);
							}
							p_signeBig.push_back(valueSigne);
						}
					}
				}
			}
		}
		signe3D.push_back(p_signeBig);
	}

	return signe3D;
}

vector<vector<vector<double>>> InsermLibrary::Stats::pValuesKruskall(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***eegData)
{
	int numberCol = p_prov->nbCol();
	int numberRow = p_prov->nbRow();
	//===
	int v_window_ms[2], v_window_sam[2];
	int mini = 0, Maxi = 0;
	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		mini = min(mini, p_prov->visuBlocs[i].dispBloc.epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i].dispBloc.epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	//===

	v_window_sam[0] = round((64 * v_window_ms[0]) / 1000);
	v_window_sam[1] = round((64 * v_window_ms[1]) / 1000);

	vector<vector<vector<double>>> p_value3D;
	for (int i = 0; i < p_elan_struct->chan_nb; i++)
	{
		vector<double> baseLineData, EEGData;
		vector<vector<double>> eegDataBig;
		double temp = 0, temp2 = 0;
		vector<vector<double>> p_valueBig;

		//On calcule la valeur pour la base line (cas silence)
		int a = triggCatEla->mainGroupSub[correspEvent[correspEvent.size() - 1]];
		int b = triggCatEla->mainGroupSub[correspEvent[correspEvent.size() - 1] + 1];
		int numberSubTrial = b - a;

		for (int l = 0; l < numberSubTrial; l++)
		{
			int baselineDebut = 0;  //=16;
			int baselineFin = v_window_sam[1] - v_window_sam[0];   //=48;

			for (int m = 0; m < (baselineFin - baselineDebut); m++)
			{
				//temp += eegData[triggCatEla->trigg[a + l].origPos][i][baselineDebut + m];
				temp += eegData[a + l][i][baselineDebut + m];
			}
			baseLineData.push_back(temp / (baselineFin - baselineDebut));
			temp = 0;
		}

		//On calcule la valeur pour le premier cas 
		for (int n = 0; n < triggCatEla->mainGroupSub.size() - 2; n++)
		{
			a = triggCatEla->mainGroupSub[correspEvent[n]];
			b = triggCatEla->mainGroupSub[correspEvent[n + 1]];
			numberSubTrial = b - a;

			for (int l = 0; l < numberSubTrial; l++)
			{
				//boucle moyenne des X fenetres que l'on veut (même taille que base line)

				int winDebut = 0;  //=16;
				int winFin = v_window_sam[1] - v_window_sam[0];   //=48;

				for (int m = 0; m < (winFin - winDebut); m++)
				{
					//temp2 += eegData[triggCatEla->trigg[a + l].origPos][i][winDebut + m];
					temp2 += eegData[a + l][i][winDebut + m];
				}
				EEGData.push_back(temp2 / (winFin - winDebut));
				temp2 = 0;
			}
			eegDataBig.push_back(EEGData);
			EEGData.clear();
		}


		//On calcule les p
		double *dataArray[2];
		int nbSamplePerGroup[2];
		for (int n = 0; n < 1; n++)   //(une seule fenêtre qui prend tout en compte )
		{
			double p = 0, H = 0;
			vector<double> P_VALUE;
			dataArray[0] = &baseLineData[0];
			for (int l = 0; l < eegDataBig.size(); l++)
			{
				dataArray[1] = &eegDataBig[l][0];
				nbSamplePerGroup[0] = eegDataBig[l].size();
				nbSamplePerGroup[1] = eegDataBig[l].size();

				kruskal_wallis(dataArray, 2, nbSamplePerGroup, &H, &p, 1);
				P_VALUE.push_back(p);
			}
			p_valueBig.push_back(P_VALUE);
		}
		p_value3D.push_back(p_valueBig);
	}

	return p_value3D;
}

vector<vector<vector<int>>> InsermLibrary::Stats::signKruskall(elan_struct_t *p_elan_struct, PROV *p_prov, TRIGGINFO *triggCatEla, vector<int> correspEvent, double ***eegData)
{
	int numberCol = p_prov->nbCol();
	int numberRow = p_prov->nbRow();
	//===
	int v_window_ms[2];
	int mini = 0, Maxi = 0;
	for (int i = 0; i < p_prov->visuBlocs.size(); i++)
	{
		mini = min(mini, p_prov->visuBlocs[i].dispBloc.epochWindow[0]);
		Maxi = max(Maxi, p_prov->visuBlocs[i].dispBloc.epochWindow[1]);
	}

	v_window_ms[0] = mini;
	v_window_ms[1] = Maxi;
	//===

	vector<vector<vector<int>>> p_sign3D;
	for (int i = 0; i < p_elan_struct->chan_nb; i++)
	{
		vector<double> baseLineData, EEGData;
		vector<vector<double>> eegDataBig;
		double temp = 0, temp2 = 0;
		vector<vector<int>> p_signBig;

		//On calcule la valeur pour la base line (cas silence)
		int a = triggCatEla->mainGroupSub[correspEvent[correspEvent.size() - 1]];
		int b = triggCatEla->mainGroupSub[correspEvent[correspEvent.size() - 1] + 1];
		int numberSubTrial = b - a;

		for (int l = 0; l < numberSubTrial; l++)
		{
			int baselineDebut = 16;
			int baselineFin = 48;

			for (int m = 0; m < (baselineFin - baselineDebut); m++)
			{
				//temp += eegData[triggCatEla->trigg[a + l].origPos][i][baselineDebut + m];
				temp += eegData[a + l][i][baselineDebut + m];
			}
			baseLineData.push_back(temp / (baselineFin - baselineDebut));
			temp = 0;
		}

		//On calcule la valeur pour le premier cas 
		for (int n = 0; n < triggCatEla->mainGroupSub.size() - 2; n++)
		{
			a = triggCatEla->mainGroupSub[correspEvent[n]];
			b = triggCatEla->mainGroupSub[correspEvent[n + 1]];
			numberSubTrial = b - a;

			for (int l = 0; l < numberSubTrial; l++)
			{
				//boucle moyenne des X fenetres que l'on veut (même taille que base line)

				int winDebut = 16;
				int winFin = 48;

				for (int m = 0; m < (winFin - winDebut); m++)
				{
					//temp2 += eegData[triggCatEla->trigg[a + l].origPos][i][winDebut + m];
					temp2 += eegData[a + l][i][winDebut + m];
				}
				EEGData.push_back(temp2 / (winFin - winDebut));
				temp2 = 0;
			}
			eegDataBig.push_back(EEGData);
			EEGData.clear();
		}

		vector<vector<int>> signBig;
		vector<int> sign;
		for (int l = 0; l < eegDataBig.size(); l++)
		{

			double tempt = 0, tempb = 0;
			for (int m = 0; m < eegDataBig[l].size(); m++)
			{
				tempt += eegDataBig[l][m];
				tempb += baseLineData[m];
			}
			tempt /= eegDataBig[l].size();
			tempb /= eegDataBig[l].size();

			double diff = tempt - tempb;
			if (diff > 0)
			{
				sign.push_back(1);
			}
			else if (diff < 0)
			{
				sign.push_back(-1);
			}
			else if (diff == 0)
			{
				sign.push_back(0);
			}
		}
		signBig.push_back(sign);
		p_sign3D.push_back(signBig);
	}

	return p_sign3D;
}

vector<PVALUECOORD> InsermLibrary::Stats::FDR(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D, int &copyIndex, float pLimit)
{
	int V = pValues3D.size() * pValues3D[0].size() * pValues3D[0][0].size();
	int compteur = 0;
	double CV = log(V) + 0.5772;
	double slope = pLimit / (V * CV);

	vector<PVALUECOORD> preFDRValues = loadPValues(pValues3D, pSign3D);

	//heapsort if quicksort pas assez rapide   
	std::sort(preFDRValues.begin(), preFDRValues.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	for (int i = 0; i < V; i++)	
	{
		if (preFDRValues[i].pValue > ((double)slope * (i + 1)))
		{
			copyIndex = i;
			break;
		}
	}

	PVALUECOORD temp;
	vector<PVALUECOORD> significantValue;
	for (int i = 0; i < copyIndex; i++)
	{
		temp.elec = preFDRValues[i].elec;
		temp.condit = preFDRValues[i].condit;
		temp.window = preFDRValues[i].window;
		temp.vectorpos = preFDRValues[i].vectorpos;
		temp.pValue = preFDRValues[i].pValue;
		temp.weight = preFDRValues[i].weight;

		significantValue.push_back(temp);
	}

	std::sort(significantValue.begin(), significantValue.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.pValue < secondValue.pValue);
	});

	return significantValue;
}

vector<PVALUECOORD> InsermLibrary::Stats::loadPValues(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D)
{
	int compteur = 0;
	PVALUECOORD tempPValue;
	vector<PVALUECOORD> pValues;

	for (int i = 0; i < pValues3D.size(); i++)
	{
		for (int j = 0; j < pValues3D[i].size(); j++)
		{
			for (int k = 0; k < pValues3D[i][j].size(); k++)
			{
				tempPValue.elec = i;
				tempPValue.condit = j;
				tempPValue.window = k;
				tempPValue.vectorpos = compteur;
				tempPValue.pValue = pValues3D[i][j][k];
				tempPValue.weight = pSign3D[i][j][k];
				pValues.push_back(tempPValue);

				compteur++;
			}
		}
	}

	return pValues;
}

vector<PVALUECOORD> InsermLibrary::Stats::loadPValues(vector<vector<vector<double>>> pValues3D, vector<vector<vector<int>>> pSign3D, float pLimit)
{
	int compteur = 0;
	PVALUECOORD tempPValue;
	vector<PVALUECOORD> pValues;

	for (int i = 0; i < pValues3D.size(); i++)
	{
		for (int j = 0; j < pValues3D[i].size(); j++)
		{
			for (int k = 0; k < pValues3D[i][j].size(); k++)
			{
				if (pValues3D[i][j][k] < pLimit)
				{
					tempPValue.elec = i;
					tempPValue.condit = j;
					tempPValue.window = k;
					tempPValue.vectorpos = compteur;
					tempPValue.pValue = pValues3D[i][j][k];
					tempPValue.weight = pSign3D[i][j][k];

					pValues.push_back(tempPValue);
				}
				compteur++;
			}
		}
	}

	return pValues;
}

void InsermLibrary::Stats::exportStatsData(ELAN *p_elan, PROV *p_prov, vector<PVALUECOORD> pValues, string outputFolder, bool isBar)
{
	stringstream statFile;
	statFile << outputFolder << "/statLoca.csv";

	ofstream fichierSt(statFile.str().c_str(), ios::out);

	fichierSt << " " << ";";
	for (int i = 0; i < p_elan->m_bipole.size(); i++)
	{
		fichierSt << p_elan->trc->nameElectrodePositiv[p_elan->m_bipole[i]] << ";";
	}
	fichierSt << endl;

	for (int j = 0; j < p_prov->visuBlocs.size(); j++)
	{
		fichierSt << p_prov->visuBlocs[j].mainEventBloc.eventLabel << ";";
		for (int i = 0; i < p_elan->m_bipole.size(); i++)
		{
			vector<int> indexes;
			for (int z = 0; z < pValues.size(); z++)
			{
				if (isBar == false)
				{
					if (pValues[z].elec == i && pValues[z].condit == j)
						indexes.push_back(z);
				}
				else
				{
					if (pValues[z].elec == i && pValues[z].window == j)
						indexes.push_back(z);
				}
			}

			if (indexes.size() > 0)
			{
				int val = 0;
				for (int z = 0; z < indexes.size(); z++)
				{
					val += pValues[indexes[z]].weight;
				}
				fichierSt << val << ";";
			}
			else
			{
				fichierSt << "FALSE" << ";";
			}
		}
		fichierSt << endl;
	}

	fichierSt.close();
}

void InsermLibrary::Stats::exportPChanels(string outputFolder, vector<vector<vector<double>>> pValues3D)
{
	ofstream fichierSt(outputFolder.c_str(), ios::out);

	for (int i = 0; i < pValues3D.size(); i++) //elec
	{
		for (int j = pValues3D[i].size() - 1; j >= 0; j--) //cond
		{
			for (int k = 0; k < pValues3D[i][j].size(); k++) //win trial
			{
				fichierSt << pValues3D[i][j][k] << ";";
			}
			fichierSt << endl;
		}
		fichierSt << endl;
		fichierSt << endl;
	}
	fichierSt.close();
}

void InsermLibrary::Stats::exportFDRChanels(string outputFolder, vector<PVALUECOORD> pValuesFDR)
{
	std::sort(pValuesFDR.begin(), pValuesFDR.end(),
		[](PVALUECOORD firstValue, PVALUECOORD secondValue) {
		return (firstValue.elec < secondValue.elec);
	});

	ofstream fichierSt(outputFolder.c_str(), ios::out);

	for (int i = 0; i < pValuesFDR.size(); i++)
	{
		fichierSt << pValuesFDR[i].elec << ";" << pValuesFDR[i].condit << ";" << pValuesFDR[i].window << ";" << pValuesFDR[i].pValue << ";" << endl;
	}
	fichierSt.close();
}