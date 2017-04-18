#include "mapsGenerator.h"

using namespace InsermLibrary;
using namespace InsermLibrary::DrawCard;

mapsGenerator::mapsGenerator(int width, int height)
{
	Heigth = height;
	Width = width;
}

mapsGenerator::~mapsGenerator()
{
	delete ColorMapJet;
}

void mapsGenerator::trialmatTemplate(vector<int> trialsPerRow, int windowMS[2], InsermLibrary::PROV *p_prov)
{
	double matrixWidth, matrixHeight;
	QPoint MatrixtopLeft;
	
	double colorBarWidth, colorBarHeight;
	QPoint colorBartopLeft;

	fullMap = QRect(0, 0, Width, Heigth);

	ColorMapJet = new QColor[512];
	jetColorMap512(ColorMapJet);

	//== Matrix measurement
	matrixWidth = ceil(0.6840 * Width);
	matrixHeight = ceil(0.8148 * Heigth);
	MatrixtopLeft.setX(ceil(0.1302 * Width));
	MatrixtopLeft.setY(ceil(0.07638 * Heigth));

	MatrixRect.setTopLeft(MatrixtopLeft);
	MatrixRect.setWidth(matrixWidth);
	MatrixRect.setHeight(matrixHeight);
	//==

	//== ColorBar measurement
	colorBarWidth = ceil(0.0364 * Width);
	colorBarHeight = matrixHeight;
	colorBartopLeft.setX(ceil(0.8420 * Width));
	colorBartopLeft.setY(ceil(0.07638 * Heigth));

	colorBarRect.setTopLeft(colorBartopLeft);
	colorBarRect.setWidth(colorBarWidth);
	colorBarRect.setHeight(colorBarHeight);
	//==
	
	pixmapTemplate = QPixmap(Width, Heigth);
	QPainter painterTemplate(&pixmapTemplate);
	pixmapTemplate.fill(QColor(Qt::white));

	painterTemplate.drawRect(MatrixRect);
	defineLineSeparation(&painterTemplate, trialsPerRow, 1);
	createColorBar(&painterTemplate);
	createTimeLegend(&painterTemplate, windowMS);
	createTrialsLegend(&painterTemplate, p_prov);
}

void mapsGenerator::graduateColorBar(QPainter *painter, int maxValue)
{
	int numberTick = 3;

	//== put zero on colorbar
	QPoint LbegLine(colorBarRect.x(), colorBarRect.y() + (colorBarRect.height() / 2));
	QPoint LendLine(colorBarRect.x() + (colorBarRect.width() / 5), colorBarRect.y() + (colorBarRect.height() / 2));
	painter->drawLine(LbegLine, LendLine);

	float size = LendLine.x() - LbegLine.x();

	QPoint RbegLine(colorBarRect.x() + colorBarRect.width() - size, colorBarRect.y() + (colorBarRect.height() / 2));
	QPoint RendLine(colorBarRect.x() + colorBarRect.width(), colorBarRect.y() + (colorBarRect.height() / 2));
	painter->drawLine(RbegLine, RendLine);

	QFont fff;
	fff.setPixelSize(0.0254629 * fullMap.height());
	painter->setFont(fff);

	QFontMetrics fm(fff);
	int pixelsWide = fm.width("0");

	QRect zeroRect(0.8836805 * fullMap.width(), 0.4953703 * fullMap.height(), pixelsWide/*0.012152777 * fullMap.width()*/, 0.0254629 * fullMap.height());
	painter->drawText(zeroRect.x(), zeroRect.y(), QString("0"));

	double verticalStep = (double)colorBarRect.height() / (2 * numberTick);
	double numberStep = (double)maxValue / numberTick;

	for (int i = 0; i < numberTick; i++)
	{
		int numberLegend = (int)round(numberStep * (i + 1));
		pixelsWide = fm.width(QString::number(-numberLegend));

		//=== Below Zero
		QPoint Lbeg(LbegLine.x(), LbegLine.y() + (verticalStep * i));
		QPoint Lend(LendLine.x(), LendLine.y() + (verticalStep * i));

		QPoint Rbeg(RbegLine.x(), RbegLine.y() + (verticalStep * i));
		QPoint Rend(RendLine.x(), RendLine.y() + (verticalStep * i));

		int yRect = RendLine.y() + (verticalStep * (i + 1)) - ((0.0254629 * fullMap.height()) / 2);
		QRect rectLeg(zeroRect.x(), yRect, pixelsWide, 0.0254629 * fullMap.height());

		painter->drawLine(Lbeg, Lend);
		painter->drawLine(Rbeg, Rend);

		//=== Above Zero
		QPoint OLbeg(LbegLine.x(), LbegLine.y() - (verticalStep * i));
		QPoint OLend(LendLine.x(), LendLine.y() - (verticalStep * i));

		QPoint ORbeg(RbegLine.x(), RbegLine.y() - (verticalStep * i));
		QPoint ORend(RendLine.x(), RendLine.y() - (verticalStep * i));

		int OyRect = RendLine.y() - (verticalStep * (i + 1)) - ((0.0254629 * fullMap.height()) / 2);
		QRect OrectLeg(zeroRect.x(), OyRect, pixelsWide, 0.0254629 * fullMap.height());

		painter->drawLine(OLbeg, OLend);
		painter->drawLine(ORbeg, ORend);

		//=== Draw Numeric Scale
		painter->drawText(rectLeg, QString::number(-numberLegend));
		painter->drawText(OrectLeg, QString::number(numberLegend));
	}
}

void mapsGenerator::drawVerticalZeroLine(QPainter *painter, int windowMs[2])
{
	float sizePxLine = 0.005208333 * MatrixRect.width();
	int zeroBorder = ceil((double)MatrixRect.width() / (windowMs[1] - windowMs[0]) * abs(windowMs[0]));

	painter->setPen(QColor(255, 255, 255, 255)); //white
	for (int i = 0; i < (sizePxLine / 2); i++)
	{
		painter->drawLine(MatrixRect.x() - i + zeroBorder, MatrixRect.y() + MatrixRect.height() - ((0.005208333 * MatrixRect.width()) - 2), MatrixRect.x() - i + zeroBorder, MatrixRect.y() + 1);
		painter->drawLine(MatrixRect.x() + i + zeroBorder, MatrixRect.y() + MatrixRect.height() - ((0.005208333 * MatrixRect.width()) - 2), MatrixRect.x() + i + zeroBorder, MatrixRect.y() + 1);
	}
	painter->drawLine(MatrixRect.x() + zeroBorder, MatrixRect.y() + MatrixRect.height() - ((0.005208333 * MatrixRect.width()) - 2), MatrixRect.x() + zeroBorder, MatrixRect.y() + 1);
}

void mapsGenerator::displayStatsOnMap(QPainter *painter, vector<vector<int>> indexCurrentMap, vector<PVALUECOORD> significantValue, int windowMS[2], int nbRow)
{
	for (int y = nbRow - 1; y >= 0; y--)
	{
		if (indexCurrentMap[y].size() > 0)
		{
			for (int z = 0; z < indexCurrentMap[y].size(); z++)
			{
				int xTimeMs = (significantValue[indexCurrentMap[y][z]].window * 100);
				double MsByPx = separationLines[y].width() / (double)(windowMS[1] - windowMS[0]);
				double xBegWin = separationLines[y].x() + (xTimeMs * MsByPx);
				double xEndWin = xBegWin + (200 * MsByPx);

				painter->setPen(QColor(255, 0, 255, 255)); //pink petant
				for (int zz = 0; zz < separationLines[y].height(); zz++)
				{
					painter->drawLine(xBegWin, separationLines[y].y() + zz, xEndWin, separationLines[y].y() + zz);
				}
				painter->setBrush(QColor(0, 0, 0, 255));
				painter->drawEllipse(QPoint(xBegWin, separationLines[y].y()), 4, 4);
			}
		}
	}
}

void mapsGenerator::drawMapTitle(QPainter *painter, string outputLabel, string elecName)
{
	string labelPicPath = outputLabel;
	labelPicPath.append(elecName.c_str()).append(".jpg");

	painter->setPen(QColor(Qt::GlobalColor::black));
	painter->drawText(QPoint(0.20833 * fullMap.width(), 0.0532407 * fullMap.height()), QString(labelPicPath.c_str()));
}

vector<vector<double>> mapsGenerator::horizontalInterpolation(double ** chanelToInterpol, int interpolationFactor,
									TRIGGINFO *triggCatEla, int nbSubTrials, int nbSampleWindow, int indexBegTrigg)
{
	vector<vector<double>> eegDataInterpolated;
	for (int l = 0; l < nbSubTrials; l++)
	{
		vector<double> eegDataOneTrial;
		for (int m = 0; m < nbSampleWindow; m++)
		{
			double beginValue = chanelToInterpol[triggCatEla->trigg[indexBegTrigg + l].origPos][m];
			double endValue = chanelToInterpol[triggCatEla->trigg[indexBegTrigg + l].origPos][m + 1];

			for (int n = 0; n < interpolationFactor; n++)
			{
				double coeff = (double)n / interpolationFactor;
				eegDataOneTrial.push_back(((1 - coeff) * beginValue) + (coeff * endValue));
			}
		}
		eegDataInterpolated.push_back(eegDataOneTrial);
	}

	return eegDataInterpolated;
}

vector<vector<double>> mapsGenerator::verticalInterpolation(vector<vector<double>> chanelToInterpol, int interpolationFactor)
{
	int vertSize = chanelToInterpol.size();
	int horizSize = chanelToInterpol[0].size();

	vector<vector<double>> eegDataInterpolated;
	for (int l = 0; l < horizSize; l++)
	{
		vector<double> eegDataOneSample;
		for (int m = 0; m < vertSize - 1; m++)
		{
			double beginValue = chanelToInterpol[m][l];
			double endValue = chanelToInterpol[m + 1][l];

			for (int n = 0; n < interpolationFactor; n++)
			{
				double coeff = (double)n / interpolationFactor;
				eegDataOneSample.push_back(((1 - coeff) * beginValue) + (coeff * endValue));
			}
		}
		eegDataInterpolated.push_back(eegDataOneSample);
	}

	return eegDataInterpolated;
}

void mapsGenerator::eegData2ColorMap(vector<int> colorX[512], vector<int> colorY[512], vector<vector<double>> interpolatedData, 
							int nbSubTrials, int nbSampleWindow, int horizInterpFactor, int vertInterpFactor, double maxValue)
{
	double minValue = -maxValue;

	for (int l = 0; l < vertInterpFactor * (nbSubTrials - 1); l++)
	{
		for (int m = 0; m < horizInterpFactor * nbSampleWindow; m++)
		{
			double r = (interpolatedData[m][l] - minValue) / (maxValue - minValue); 

			int col = 0 + (511 * r);
			if (col < 0)
			{
				col = 0;
			}
			else if (col > 511)
			{
				col = 511;
			}

			/*2Dim vector, since changing he color of the qt pen takes time , we fill every pixel
			of one color, then the next, then the next , etc ...*/
			colorX[col].push_back(m);
			colorY[col].push_back(l);
		}
	}
}

void mapsGenerator::eegData2ColorMap(vector<int> colorX[512], vector<int> colorY[512], double **eegData, TRIGGINFO *triggCatEla, 
							int nbSubTrials, int nbSampleWindow, int indexBegTrigg, double maxValue)
{
	double minValue = -maxValue;

	for (int l = 0; l < nbSubTrials; l++)
	{
		for (int m = 0; m < nbSampleWindow; m++)
		{
			double r = (eegData[triggCatEla->trigg[indexBegTrigg + l].origPos][m] - minValue) / (maxValue - minValue);

			int col = 0 + (511 * r);
			if (col < 0)
			{
				col = 0;
			}
			else if (col > 511)
			{
				col = 511;
			}

			/*2Dim vector, since changing he color of the qt pen takes time , we fill every pixel
			of one color, then the next, then the next , etc ...*/
			colorX[col].push_back(m);
			colorY[col].push_back(l);
		}
	}
}

vector<int> mapsGenerator::checkIfNeedDisplayStat(vector<PVALUECOORD> significantValue, int indexCurrentElec)
{
	vector<int> allIndexCurrentMap;
	for (int z = 0; z < significantValue.size(); z++)
	{
		if (significantValue[z].elec == indexCurrentElec)
		{
			allIndexCurrentMap.push_back(z);
		}
	}

	return allIndexCurrentMap;
}

vector<vector<int>> mapsGenerator::checkIfConditionStat(vector<PVALUECOORD> significantValue, vector<int> allIndexMap, int numberRow)
{
	vector<vector<int>> indexCurrentMap;
	for (int y = numberRow - 1; y >= 0; y--)
	{
		vector<int> indexCondition;
		for (int z = 0; z < allIndexMap.size(); z++)
		{
			if (significantValue[allIndexMap[z]].condit == y)
			{
				indexCondition.push_back(allIndexMap[z]);
			}
		}
		indexCurrentMap.push_back(indexCondition);
	}

	return indexCurrentMap;
}

//=== Private 

void mapsGenerator::jetColorMap512(QColor *colorMap)
{
	int compteur = 0;
	for (int i = 0; i < 57; i++)
	{
		colorMap[i].setBlue(143.4375 + (i * 1.9649));
	}

	compteur = 57;
	for (int i = 0; i < 130; i++)
	{
		colorMap[compteur].setGreen((double)0.4366 + (i * 1.9649));
		colorMap[compteur].setBlue(255);
		compteur++;
	}

	compteur = 187;
	for (int i = 0; i < 130; i++)
	{
		colorMap[compteur].setRed(0.8733 + (i * 1.9649));
		colorMap[compteur].setGreen(255);
		colorMap[compteur].setBlue(254.1267 - (i * 1.9649));
		compteur++;
	}

	compteur = 317;
	for (int i = 0; i < 130; i++)
	{
		colorMap[compteur].setRed(255);
		colorMap[compteur].setGreen(253.6901 - (i * 1.9649));
		colorMap[compteur].setBlue(0);
		compteur++;
	}

	compteur = 447;
	for (int i = 0; i < 65; i++)
	{
		colorMap[compteur].setRed(253.2534 - (i * 1.9649));
		colorMap[compteur].setGreen(0);
		colorMap[compteur].setBlue(0);
		compteur++;
	}
}

void mapsGenerator::createColorBar(QPainter *painter)
{
	painter->drawRect(colorBarRect);

	for (int i = 0; i < 512; i++)
	{
		QPainterPath qPath;
		float xBeg = colorBarRect.x() + 1;
		float yBeg = colorBarRect.y() + (((double)(colorBarRect.height() - 1) / 512) * i);
		float xEnd = xBeg + colorBarRect.width() - 2;
		float yEnd = colorBarRect.y() + (((double)(colorBarRect.height() - 1) / 512) * (i));

		float yNext = colorBarRect.y() + 1 + (((double)(colorBarRect.height() - 1) / 512) * (i + 1));

		for (int j = 0; j < (yNext - yBeg); j++)
		{
			QPoint beginPoint(xBeg, yBeg + (j + 1));
			QPoint endPoint(xEnd, yEnd + (j + 1));
			qPath.moveTo(beginPoint);
			qPath.lineTo(endPoint);
		}

		painter->setPen(ColorMapJet[511 - i]);
		painter->drawPath(qPath);
	}
}

void mapsGenerator::defineLineSeparation(QPainter *painter, vector<int> nbTrialPerRow, int nbCol)
{
	int nbRow = nbTrialPerRow.size() - 1;

	int nbHorizLine = nbRow;
	int nbVertLine = nbCol - 1;
	float sizePxLine = 0.005208333 * MatrixRect.width();
	int totalNbTrials = nbTrialPerRow[nbRow];
	int heightCumul = 0;

	for (int i = 0; i < nbCol; i++)
	{
		heightCumul = 0;
		for (int j = 0; j < nbRow; j++)
		{
			int width = (MatrixRect.width() - 1 - ((nbCol - 1) * sizePxLine)) / nbCol;

			int realHeight = (MatrixRect.height() - 2 - (nbRow * sizePxLine));
			double ratioTrials = ((double)(nbTrialPerRow[nbRow - j] - nbTrialPerRow[nbRow - j - 1])) / totalNbTrials;
			
			int height = round(realHeight * ratioTrials);

			if (j % 3 == 0)
			{
				height += 1;
			}
			int x = MatrixRect.x() + 1 + ((width + sizePxLine) * i);
			int y = MatrixRect.y() + 1 + heightCumul;

			heightCumul += height + sizePxLine;

			subMatrixes.push_back(QRect(x, y, width, height));
		}
	}

	for (int i = 0; i < nbCol; i++)
	{
		for (int j = 0; j < nbRow; j++)
		{
			int adjust = 0;
			int x = subMatrixes[j].x();
			int y = subMatrixes[j].y() + subMatrixes[j].height();

			separationLines.push_back(QRect(x, y, subMatrixes[j].width(), sizePxLine));

			if (j == nbRow - 1)
			{
				int a = MatrixRect.height() + MatrixRect.y();
				int b = separationLines[j].height() + separationLines[j].y();
				if (b != a)
				{
					separationLines[j].setHeight(separationLines[j].height() + (a - b));
				}
			}
		}
	}

	//for (int i = subMatrixes.size() - 1; i >= 0; i--)
	//{
	//	if (i % 2 == 0)
	//	{
	//		painter->fillRect(subMatrixes[i], QBrush(Qt::blue));
	//		painter->fillRect(separationLines[i], QBrush(Qt::white));
	//	}
	//	else
	//	{
	//		painter->fillRect(subMatrixes[i], QBrush(Qt::red));
	//		painter->fillRect(separationLines[i], QBrush(Qt::white));
	//	}
	//}
}

void mapsGenerator::createTimeLegend(QPainter *painter, int windowMS[2])
{
	int stepTimeLegend = 200;

	while ((windowMS[1] / stepTimeLegend) > 5 || (abs(windowMS[0]) / stepTimeLegend) > 5)
	{
		stepTimeLegend += 200;
	}

	int zeroBorder = ceil((double)MatrixRect.width() / (windowMS[1] - windowMS[0]) * abs(windowMS[0]));
	int numberTickLeft = abs(windowMS[0]) / stepTimeLegend;
	int numberTickRight = windowMS[1] / stepTimeLegend;

	painter->setPen(Qt::black);

	QFont fff;
	fff.setPixelSize(0.0254629 * fullMap.height());
	
	painter->setFont(fff);

	QFontMetrics fm(fff);
	int z = fm.width("0");
	int pixelsWide = fm.width("0");

	int xBeg = MatrixRect.x() + zeroBorder - (0.5 * pixelsWide);
	int yBeg = 0.9027777 * fullMap.height();
	int width = pixelsWide;
	int heigth = 0.0254629 * fullMap.height();

	QRect legendValueRect(xBeg, yBeg, width, heigth);
	painter->drawText(legendValueRect, QString(QString().number(0)));

	for (int i = 1; i < numberTickRight + 1; i++)
	{
		//if (((i * stepTimeLegend) / 1000) >= 1)
		//{
		pixelsWide = fm.width("XXXX");
		//}
		//else
		//{
		//	pixelsWide = fm.width("XXX");
		//}
		width = pixelsWide;

		int ohterBorder = ceil((double)MatrixRect.width() / (windowMS[1] - windowMS[0]) * (i * stepTimeLegend));
		int xBeg2 = xBeg + ohterBorder - (0.5 * pixelsWide);
		legendValueRect.setX(xBeg2);
		legendValueRect.setWidth(width);
		painter->drawText(legendValueRect, QString(QString().number(0 + (i * stepTimeLegend))));
	}

	for (int i = 1; i < numberTickLeft + 1; i++)
	{
		pixelsWide = fm.width("XXXXX");
		width = pixelsWide;

		int ohterBorder = ceil((double)MatrixRect.width() / (windowMS[1] - windowMS[0]) * (i * stepTimeLegend));
		int xBeg2 = xBeg - ohterBorder;
		legendValueRect.setX(xBeg2);
		legendValueRect.setWidth(width);
		painter->drawText(legendValueRect, QString(QString().number(0 + (-i * stepTimeLegend))));
	}
}

void mapsGenerator::createTrialsLegend(QPainter *painter, PROV *p_prov)
{
	for (int k = 0; k < (p_prov->visuBlocs.size()); k++)
	{
		int index2Draw = (p_prov->visuBlocs.size() - 1) - (p_prov->visuBlocs[k].dispBloc.row - 1);
		if (QFileInfo(p_prov->visuBlocs[k].dispBloc.path.c_str()).exists())
		{
			QRect myLegendRect(0.013020833 * fullMap.width(), subMatrixes[index2Draw].y(), 0.1119618055 * fullMap.width(), subMatrixes[index2Draw].height());
			QPixmap image(p_prov->visuBlocs[k].dispBloc.path.c_str());

			if (p_prov->visuBlocs.size() < 5)
			{
				image = image.scaledToWidth(myLegendRect.width(), Qt::TransformationMode::FastTransformation);
				int offset = (subMatrixes[index2Draw].height() - image.height()) / 2;
				painter->drawPixmap(myLegendRect.x(), myLegendRect.y() + offset, image.width(), image.height(), image);
			}
			else
			{
				image.scaled(myLegendRect.size(), Qt::KeepAspectRatio);
				painter->drawPixmap(myLegendRect, image);
			}
		}
		else
		{
			painter->setPen(QColor(Qt::darkRed));
			int offset = subMatrixes[index2Draw].height() / 2;
			QRect myLegendRect(0.013020833 * fullMap.width(), subMatrixes[index2Draw].y() + offset, 0.1119618055 * fullMap.width(), subMatrixes[0].height());
			painter->drawText(myLegendRect, QString::fromStdString(p_prov->visuBlocs[k].mainEventBloc.eventLabel));
		}
	}
}