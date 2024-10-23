#include <valarray>
#include <complex>
#include <iostream>

#pragma once

//The YIN Algorithm is the algorithm that determines the fundamental frequency of the capture buffer
//The capture buffer is converted into an array of a size of a power of 2 (of type complex<double>)
class YIN
{
private:
	//Each functions behaviour and purpose is defined in the design document
	static std::valarray<std::complex<double>> cumulativeSum(std::valarray<std::complex<double>> P);
	static std::valarray<std::complex<double>> range(int N);
	static std::valarray<std::complex<double>> fourierTransform(std::valarray<std::complex<double>> P);
	static std::valarray<std::complex<double>> inverseFourierTransform(std::valarray<std::complex<double>> P);
	static std::valarray<std::complex<double>> differenceFunction(std::valarray<std::complex<double>> signal, int chunkSize, int tauMax);
	static std::valarray<std::complex<double>> cumulativeMeanNormalizedDifferenceFunction(std::valarray<std::complex<double>> df, int tauMax);
	static int calculatePitch(std::valarray<std::complex<double>> cmndf, int tauMin, int tauMax);
public:
	//Main call for the pitch detection algorithm
	static float YINalgorithm(std::valarray<std::complex<double>> signal);
};

