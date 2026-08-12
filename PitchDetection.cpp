#include "PitchDetection.h"
#include <optional>
#include <span>
#include <vector>

void PitchDetection::calculateACF(std::span<double> buffer) {
    fftw_execute_dft_r2c(forwardPlan, buffer.data(), forwardFFT);

    size_t halfSize = sampleSize / 2 + 1;
    for (size_t i = 0; i < halfSize; i++) {
        const auto [re, im] = forwardFFT[i];
        forwardFFT[i][0] = re * re + im * im;
        forwardFFT[i][1] = 0.;
    }
    fftw_execute(inversePlan);

    for (size_t i = 0; i < sampleSize; i++) {
        acfOut[i] /= static_cast<double>(sampleSize);
    }
}

std::vector<double> PitchDetection::calculateDifferenceFunction(std::span<double> buffer) {
    //Calculate cumulative sum
    const size_t N = buffer.size();

    double total{0.};
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

std::vector<double> PitchDetection::calculateCMNDF(std::span<double> buffer) {
    std::vector<double> CMNDF(sampleSize);

    std::vector<double> DF = calculateDifferenceFunction(buffer);
    CMNDF[0] = 1.;
    double runningTotal{0.};
    for (size_t tau = 1; tau < sampleSize; tau++) {
        runningTotal += DF[tau];
        CMNDF[tau] = static_cast<double>(tau) * DF[tau];
        CMNDF[tau] /= runningTotal; 
    }

    return CMNDF;
}

std::optional<double> PitchDetection::pitchFromBuffer(std::span<double> buffer) {
    std::vector<double> CMNDF = calculateCMNDF(buffer);
    for (size_t i = tauMin; i <= tauMax; i++) {
        if (CMNDF[i] < harmonyThreshold &&
            CMNDF[i] < CMNDF[i - 1] &&
            CMNDF[i] <= CMNDF[i + 1]
        ) {
            return sampleRate / static_cast<double>(i);
        }
    }
    return std::nullopt;
}