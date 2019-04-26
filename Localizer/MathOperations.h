#ifndef _MATHOPERATIONS_H
#define _MATHOPERATIONS_H

#include <iostream>
#include <vector>
#include <stdexcept>      // std::invalid_argument

class MathOperations
{
public:
	static std::vector<float> VectorConvolution(std::vector<float> & vecA, std::vector<float> & vecB);
};

#endif