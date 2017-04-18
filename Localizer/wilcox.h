#ifndef _WILCOX_H
#define _WILCOX_H

#include <vector>

using namespace std;

namespace InsermLibrary
{
	template <typename InputIterator, typename ElementType, typename Predicate>
	const ElementType accumulate_if(InputIterator first, const InputIterator last, ElementType init, const Predicate predicate);

	enum Sign { minus = -1, none = 0, plus = 1 };

	template <typename T>
	const vector<Sign> calcSigns(const vector<T>& vec1, const vector<T>& vec2);

	template <typename T>
	const vector<T> calcDifference(const vector<T>& vec1, const vector<T>& vec2);

	template <typename T>
	void makeAbs(vector<T>& vec);

	template <typename T>
	const vector<T> getAbs(const vector<T>& vec);

	template <typename T>
	const T sumAbs(const vector<T>& vec);

	float findMinAbove(const vector<float>& vec, const float& above);

	const vector<float> findRanks(const vector<float>& vec);

	const vector<float> getSignedRanks(const vector<Sign>& signs, const vector<float>& ranks);

	float sumPositives(const vector<float>& vec);

	float sumNegatives(const vector<float>& vec);

	int countNonZero(const vector<float>& vec);

	int countZero(const vector<float>& vec);

	int countPlus(const vector<float>& vec);

	int countMinus(const vector<float>& vec);

	// method based on I. C. A. Oyeka et al., Modified Wilcoxon Signed-Rank Test, Open Journal of Statistics 172-176, 2012 
	float wilcoxon(const vector<float>& vec1, const vector<float>& vec2);
}

#endif