#include "PitchDetection.h"
#include <span>

void PitchDetection::calculateACF(std::span<double> buffer) {
    fftw_execute_dft_r2c(forwardPlan, buffer.data(), forwardFFT);

    size_t halfSize = sampleSize / 2 + 1;
    for (size_t i = 0; i < halfSize; i++) {
        const auto [re, im] = forwardFFT[i];
        forwardFFT[i][0] = re * re + im * im;
        forwardFFT[i][1] = 0.f;
    }
    fftw_execute(inversePlan);
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