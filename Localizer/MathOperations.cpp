#include "MathOperations.h"

std::vector<float> MathOperations::VectorConvolution(std::vector<float> & vecA, std::vector<float> & vecB)
{
	if (vecA.size() < vecB.size())
		throw std::invalid_argument("vecA should be bigger or equal to vecB size");

	std::vector<float> outConv = std::vector<float>(vecA.size() + vecB.size() - 1);
	for (int i = 0; i < vecA.size(); i++)
	{
		for (int j = 0; j < vecB.size(); j++)
		{
			outConv[i + j] += vecA[i] * vecB[j];
		}
	}
	return outConv;
}