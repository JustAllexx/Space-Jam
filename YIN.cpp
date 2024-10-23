#include "YIN.h"

//Constants needed for calculating the fourier transforms
const double pi = acos(0.0f) * 2;
const std::complex<double> negi = std::complex<double>(0.f, -2.f * pi); //Used for calculating forward fourier transform
const std::complex<double> posi = std::complex<double>(0.f, 2.f * pi); //Used for calculating the inverse fourier transform

//The maximum frequency I want to calculate for and the lowest (determines the tau values I calculate for)
int frequencyMax = 1000.f;
int frequencyMin = 70.f;

//Calculates the cumulative sum, so [1, 2, 3, 4] -> [1, 3, 6, 10]. Keeps a running total over each index, needed in the differenceFunction
std::valarray<std::complex<double>> YIN::cumulativeSum(std::valarray<std::complex<double>> P) {
	std::complex<double> runningTotal = std::complex<double>(0.f, 0.f);
	int Psize = P.size();
	std::valarray<std::complex<double>> cumulativeSum;
	cumulativeSum.resize(Psize);
	for (int i = 0; i < Psize; i++) {
		runningTotal += P[i];
		cumulativeSum[i] = runningTotal;
	}
	return cumulativeSum;
}

//An implementation of the range function from python
std::valarray<std::complex<double>> YIN::range(int N) {
	std::valarray < std::complex < double>> r;
	r.resize(N);
	for (int i = 0; i < N; i++) { r[i] = i; }
	return r;
}

//Cooley-Tukey optimised forward Fourier transform. Uses recursion to optimise 
std::valarray<std::complex<double>> YIN::fourierTransform(std::valarray<std::complex<double>> P) {
	int n = P.size();

	if (n == 1) { //Base case, a Fourier transform of an element size 1, is itself
		return P;
	}

	std::complex<double> omega = exp((negi) / (std::complex<double>)n);
	std::complex<double> calc;
	//Splits up the P into its even Indices and it's odd indices so [1,2,3,4] -> [1,3] and [2,4]
	std::valarray<std::complex<double>> Pe, Po, Y;

	Pe = P[std::slice(0, n / 2, 2)];
	Po = P[std::slice(1, n / 2, 2)];
	//Recursive call of the main function
	Pe = fourierTransform(Pe);
	Po = fourierTransform(Po);

	Y.resize(n);

	int Ysize = floor(n / 2);
	std::complex<double> running = std::complex<double>(1.f, 0.f);
	for (int i = 0; i < Ysize; i++) {
		//Calculates the twiddle factor (as define dy Cooley-Tukey) another method of maintaining performance
		calc = running * Po[i];
		Y[i] = Pe[i] + calc;
		Y[i + Ysize] = Pe[i] - calc;
		running *= omega;
	}

	return Y;
}

//Actually the same as the previous function, but the exponent of the omega value is made positive and not negative, this acts as inversing a Fourier Transform
//Comes from a property of complex conjugates that negates the fourier transform
std::valarray<std::complex<double>> YIN::inverseFourierTransform(std::valarray<std::complex<double>> P) {
	int n = P.size();

	if (n == 1) {
		return P;
	}

	std::complex<double> omega = exp((posi) / (std::complex<double>)n);
	//std::complex<double> omega = omegaMapInv[n];
	std::complex<double> calc;

	std::valarray<std::complex<double>> Pe, Po, Y;

	Pe = P[std::slice(0, n / 2, 2)];
	Po = P[std::slice(1, n / 2, 2)];

	Pe = inverseFourierTransform(Pe);
	Po = inverseFourierTransform(Po);

	//Y.reserve(n);
	Y.resize(n);

	int Ysize = floor(n / 2);
	std::complex<double> running = std::complex<double>(1.f, 0.f);
	for (int i = 0; i < Ysize; i++) {
		calc = running * Po[i];
		Y[i] = Pe[i] + calc;
		Y[i + Ysize] = Pe[i] - calc;
		running *= omega;
	}

	return Y;
}

//The difference function, the first step in the YIN algorithm
std::valarray<std::complex<double>> YIN::differenceFunction(std::valarray<std::complex<double>> signal, int chunkSize, int tauMax) {

	//Calculating the Cumulative sum of the signal squared
	std::valarray<std::complex<double>> signalSquared = signal * signal;
	std::valarray<std::complex<double>> cumSum;
	cumSum.resize(chunkSize + 1);
	//Want the first value of the cumulative sum to be 0
	cumSum[std::slice(1, chunkSize, 1)] = cumulativeSum(signalSquared);

	//Find the minimum size padding of our array, returns the next biggest power of 2 for the size of the window
	int FFTpaddingSize = pow(2, (int)floor(log2(chunkSize)) + 1);

	//Application of the Wiener-Khinchin formula for the efficient computation of an autocorrelation

	std::valarray<std::complex<double>> signalResised = std::valarray<std::complex<double>>(FFTpaddingSize);
	signalResised[std::slice(0, chunkSize, 1)] = signal;
	std::valarray<std::complex<double>> forwardFFT = fourierTransform(signalResised);

	std::valarray<std::complex<double>> forwardFFTconj;
	forwardFFTconj.resize(FFTpaddingSize);
	for (int i = 0; i < FFTpaddingSize; i++) {
		forwardFFTconj[i] = std::complex<double>(forwardFFT[i].real(), forwardFFT[i].imag() * -1);
	}

	//Calculates the convolution and the "Energy Terms". The Third Energy Term is just a convolution that we can calculate using the Wiener-Khinchin optimsation
	//The other 2 Energy Terms can be found by manipulating the cumulativeSum
	//Because the 2 Energy Terms have no lag value, it makes sense we can find these values by squaring the signal
	std::valarray<std::complex<double>> convolutionInput = forwardFFT * forwardFFTconj;
	std::valarray<std::complex<double>> convolution = inverseFourierTransform(convolutionInput)[std::slice(0, tauMax, 1)];
	convolution /= FFTpaddingSize;
	std::complex<double> firstEnergyTerms = cumSum[chunkSize];
	std::valarray<std::complex<double>> secondEnergyTerms = cumSum[std::slice(0, tauMax, 1)];
	secondEnergyTerms = cumSum[chunkSize] - secondEnergyTerms;

	//Calculates the resulting value of the Difference function
	std::valarray<std::complex<double>> result = firstEnergyTerms + secondEnergyTerms - (2 * convolution);

	return result;
}

//This function takes the result of my difference function and averages each value over the sum of all previous values
//This has the effect of minimising the effect of low tau values
std::valarray<std::complex<double>> YIN::cumulativeMeanNormalizedDifferenceFunction(std::valarray<std::complex<double>> df, int tauMax) {
	std::valarray<std::complex<double>> cmndf = (df * range(tauMax)) / cumulativeSum(df);
	cmndf[0] = std::complex<double>(1.f, 0.f);
	return abs(cmndf);
}

//This function cycles through all the possible periods and returns the period that is under the harmony threshold
int YIN::calculatePitch(std::valarray<std::complex<double>> cmndf, int tauMin, int tauMax) {
	float harmonyThreshold = 0.2f;
	int tau = tauMin;
	for (; tau < tauMax; tau++) {
		if (abs(cmndf[tau]) < harmonyThreshold) {
			while (tau + 1 < tauMax && abs(cmndf[tau + 1]) < abs(cmndf[tau])) {
				tau += 1;
			}
			return tau;
		}
	}

	return 0;
}

float YIN::YINalgorithm(std::valarray<std::complex<double>> signal)
{
	//Might be reverse of what you expect, tau means latency (or the period of the wave) so the minimum latency to calculate for would be the period of the maximum frequency and vice versa
	//44100 is the (expected) sampling rate
	int tauMin = floor(44100 / frequencyMax);
	int tauMax = floor(44100 / frequencyMin);
	//Calculates the differenceFunction of the signal
	std::valarray<std::complex<double>> df = differenceFunction(signal, signal.size(), tauMax);
	//Very long name for a relatively simple function, follows eq(6) of the YIN paper but pairwise rather than individually
	std::valarray<std::complex<double>> cmndf = cumulativeMeanNormalizedDifferenceFunction(df, tauMax);
	//Find the tau value of the fundamental period
	int fundamentalPeriod = calculatePitch(cmndf, tauMin, tauMax);


	if (fundamentalPeriod == 0) {
		return 0.f;
	}
	//Convert that period into a frequency if it is not 0
	float f0 = (44100.f / fundamentalPeriod);

	return f0;
}
