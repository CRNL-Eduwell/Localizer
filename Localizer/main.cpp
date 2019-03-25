#include <QtWidgets/QApplication>
#include "localizer.h"
#include "BiquadFilter.h"

int main(int argc, char *argv[])
{
	//QApplication a(argc, argv);
	//Localizer w;
	//w.show();
	//return a.exec();

	int signalLength = 256;
	int samplingFreq = 256;

	double* sin2Freq = new double[signalLength];
	for (int i = 0; i < signalLength; i++)
		sin2Freq[i] = sin((2 * 3.14 * 15 * i) / samplingFreq) + sin((2 * 3.14 * 50 * i) / samplingFreq);

	Filters::BiquadFilter *fil = new Filters::BiquadFilter(50, samplingFreq, Filters::FilterType::Notch, 0.707);
	float *dbResponse = fil->FilterDecibelResponse();
	OutputArrayToCsv1D(dbResponse, samplingFreq, "C:/Users/micromed/Desktop/file.csv");

	delete[] sin2Freq;
	delete[] dbResponse;
	delete fil;
}