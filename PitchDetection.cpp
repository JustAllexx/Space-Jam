#include "PitchDetection.h"
#include <span>
#include <vector>

void PitchDetection::calculateACF(std::span<double> buffer) {
    fftw_execute_dft_r2c(forwardPlan, buffer.data(), forwardFFT);

    size_t halfSize = sampleSize / 2 + 1;
    for (size_t i = 0; i < halfSize; i++) {
        const auto [re, im] = forwardFFT[i];
        forwardFFT[i][0] = re * re + im * im;
        forwardFFT[i][1] = 0.f;
    }
    fftw_execute(inversePlan);

    for (size_t i = 0; i < sampleSize; i++) {
        acfOut[i] /= static_cast<double>(sampleSize);
    }
}

std::vector<double> PitchDetection::calculateDifferenceFunction(std::span<double> buffer) {
    //Calculate cumulative sum
    const size_t N = buffer.size();

    double total{0.f};
    std::vector<double> cumulativeSum(N+1);
    for (size_t i = 0; i <= N; i++) {
        cumulativeSum.at(i) = total;
        if (i == N) {break;}
        total += buffer[i] * buffer[i];
    }

    //Calculate ACF
    calculateACF(buffer);

    std::vector<double> differenceFunction(N);
    for (size_t i = 0; i < N; i++) {
        double firstEnergyTerm = cumulativeSum[N - i];
        double secondEnergyTerm = cumulativeSum[N] - cumulativeSum[i];
        differenceFunction.at(i) = firstEnergyTerm + secondEnergyTerm - (2 * acfOut[i]);
    }

    return differenceFunction;
}

PitchDetection::PitchDetection() : 
    forwardFFT(static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * sampleSize))),
    acfOut(static_cast<double*>(fftw_malloc(sizeof(double) * sampleSize))),
    forwardPlan(fftw_plan_dft_r2c_1d(sampleSize, nullptr, nullptr, FFTW_ESTIMATE)),
    inversePlan(fftw_plan_dft_c2r_1d(sampleSize, forwardFFT, acfOut, FFTW_ESTIMATE))
{}

PitchDetection::~PitchDetection() {
    fftw_free(forwardFFT);
    fftw_free(acfOut);
    fftw_destroy_plan(forwardPlan);
    fftw_destroy_plan(inversePlan);
};