#include <iostream>	
#include <functional>
#include <algorithm>
#include <numeric>      // accumulate
#include "wilcox.h"
#include "wilcox.hpp"

float InsermLibrary::findMinAbove(const vector<float>& vec, const float& above)
{
	const float max = numeric_limits<float>::max();
	float lowest = max;
	for (vector<float>::const_iterator i = vec.begin(); i != vec.end(); ++i)
		if (*i > above && *i < lowest)
			lowest = *i;
	return (lowest != max ? lowest : above);
}

const vector<float> InsermLibrary::findRanks(const vector<float>& vec)
{
	const size_t sz = vec.size();
	vector<float> ranks(sz, 0.);
	float min = findMinAbove(vec, 0.), nextMin = 0, count = 1;
	bool isOver = false;
	if (min != 0) {
		float assignedRank = 0;
		int nbMin = 0;
		while (!isOver) {
			assignedRank = 0;
			nbMin = std::count(vec.begin(), vec.end(), min);
			for (int i = 0; i < nbMin; ++i) {
				assignedRank += count;
				count++;
			}
			assignedRank /= nbMin;

			for (size_t i = 0; i < sz; ++i)
				if (vec[i] == min)
					ranks[i] = assignedRank;
			nextMin = findMinAbove(vec, min);
			if (min == nextMin)
				isOver = true;
			else
				min = nextMin;
		}
	}
	return ranks;
}

const vector<float> InsermLibrary::getSignedRanks(const vector<Sign>& signs, const vector<float>& ranks)
{
	vector<float> signedRanks = {};
	if (signs.size() != ranks.size())
		cout << "The two vectors must have equal sizes (getSignedRanks error)" << endl;
	else {
		signedRanks.resize(ranks.size(), none);
		for (size_t i = 0; i < ranks.size(); ++i)
			signedRanks[i] = (signs[i] == minus ? -ranks[i] : ranks[i]);
	}
	return signedRanks;
}

float InsermLibrary::sumPositives(const vector<float>& vec)
{
	return accumulate_if(vec.begin(), vec.end(), 0., bind2nd(greater<float>(), 0.));
}

float InsermLibrary::sumNegatives(const vector<float>& vec)
{
	return accumulate_if(vec.begin(), vec.end(), 0., bind2nd(less<float>(), 0.));
}

int InsermLibrary::countNonZero(const vector<float>& vec)
{
	return count_if(vec.begin(), vec.end(), bind2nd(not_equal_to<float>(), 0.));
}

int InsermLibrary::countZero(const vector<float>& vec)
{
	return count_if(vec.begin(), vec.end(), bind2nd(equal_to<float>(), 0.));
}

int InsermLibrary::countPlus(const vector<float>& vec)
{
	return count_if(vec.begin(), vec.end(), bind2nd(greater<float>(), 0.));
}

int InsermLibrary::countMinus(const vector<float>& vec)
{
	return count_if(vec.begin(), vec.end(), bind2nd(less<float>(), 0.));
}

float InsermLibrary::wilcoxon(const vector<float>& vec1, const vector<float>& vec2)
{
	vector<float> diff = calcDifference(vec1, vec2);
	int nbNonZero = countNonZero(diff);
	const vector<Sign> diffSigns = calcSigns(vec1, vec2);
	makeAbs(diff);
	vector<float> ranks = findRanks(diff);
	const vector<float> sRanks = getSignedRanks(diffSigns, ranks);
	float sum = abs(accumulate(sRanks.begin(), sRanks.end(), 0.0));
	float zScore = (sum - 0.5) / sqrt(nbNonZero*(nbNonZero + 1)*(2 * nbNonZero + 1) / 6);
	float pValue = 1 - (0.5 + 0.5*erf(zScore / sqrt(2)));
	return pValue;
}