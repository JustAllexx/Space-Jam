#pragma once

#include <valarray>

class Program;

//Gaussian Functions
//This function calculates the gaussian distribution for the gaussian blur fragment shader
constexpr float gaussianDistribution(float x, float standardDeviation) {
	//Because we're normalising the weights in the kernel so they sum to 1, the usual constant the result needs to be multiplied by is not necessary here
	return expf((-0.5f * x * x) / (standardDeviation * standardDeviation));
}

//This function updates the gaussian blur kernel inside the gaussian blur fragment shader
//Higher values of standard deviation give a greater degree of blur while lower values give a much sharper blur
//The program is needed as an input to update the values at the end of the function
void updateGaussianKernel(float standardDeviation, Program& bloomProgram);