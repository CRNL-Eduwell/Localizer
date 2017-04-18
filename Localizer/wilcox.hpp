#ifndef _WILCOX_HPP
#define _WILCOX_HPP

#include <iostream>	
#include <vector>

using namespace std;

namespace InsermLibrary
{
	template <typename InputIterator, typename ElementType, typename Predicate>
	const ElementType accumulate_if(InputIterator first, const InputIterator last, ElementType init, const Predicate predicate)
	{
		for (; first != last; ++first)
			if (predicate(*first))
				init += *first;
		return init;
	}

	template <typename T>
	const vector<Sign> calcSigns(const vector<T>& vec1, const vector<T>& vec2)
	{
		std::vector<Sign> signs = {};
		if (vec1.size() == vec2.size()) {
			signs.resize(vec1.size(), none);
			std::transform(vec1.begin(), vec1.end(), vec2.begin(), signs.begin(), [](T x, T y) { return (x == y ? none : (x < y ? minus : plus)); });
		}
		else
			std::cout << "The two vectors must have equal sizes for a paired test (getSigns error)" << std::endl;
		return signs;
	}

	template <typename T>
	const vector<T> calcDifference(const vector<T>& vec1, const vector<T>& vec2)
	{
		std::vector<T> copy = {};
		if (vec1.size() == vec2.size()) {
			copy.resize(vec1.size());
			std::transform(vec1.begin(), vec1.end(), vec2.begin(), copy.begin(), std::minus<T>());
		}
		else
			std::cout << "The two vectors must have equal sizes for a paired test (getSigns error)" << std::endl;
		return copy;
	}

	template <typename T>
	void makeAbs(vector<T>& vec)
	{
		std::transform(vec.begin(), vec.end(), vec.begin(), [](double x) { return (x >= 0 ? x : x = -x); });
	}

	template <typename T>
	const vector<T> getAbs(const std::vector<T>& vec)
	{
		std::vector<T> copy(vec);
		makeAbs(copy);
		return copy;
	}

	template <typename T>
	const T sumAbs(const std::vector<T>& vec)
	{
		std::vector<T> copy = getAbs(vec);
		T sumOfAbs = std::accumulate(copy.begin(), copy.end(), 0);
		return sumOfAbs;
	}
}
#endif
