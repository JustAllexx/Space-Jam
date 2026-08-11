#include <cstddef>
#include <fftw3.h>
#include <optional>
#include <span>
#include <vector>

const int sampleSize = 1024;
const double sampleRate = 44100.f;
const double maxFrequency = 1000.f;
const double minFrequency = 70.f;
const size_t tauMax = static_cast<size_t>(sampleRate / minFrequency); 
const size_t tauMin = static_cast<size_t>(sampleRate / maxFrequency);
const double harmonyThreshold = 0.2f;

class PitchDetection {
private:
    fftw_complex* forwardFFT;
    double* acfOut;
    fftw_plan forwardPlan;
    fftw_plan inversePlan;

    void calculateACF(std::span<double> buffer);
    std::vector<double> calculateDifferenceFunction(std::span<double> buffer);
    std::vector<double> calculateCMNDF(std::span<double> buffer);
public:
    std::optional<double> pitchFromBuffer(std::span<double>);

    PitchDetection() :
        forwardFFT(static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * sampleSize))),
        acfOut(static_cast<double*>(fftw_malloc(sizeof(double) * sampleSize))),
        forwardPlan(fftw_plan_dft_r2c_1d(sampleSize, nullptr, nullptr, FFTW_ESTIMATE)),
        inversePlan(fftw_plan_dft_c2r_1d(sampleSize, forwardFFT, acfOut, FFTW_ESTIMATE)) {};
    
    ~PitchDetection() {
        fftw_free(forwardFFT);
        fftw_free(acfOut);
        fftw_destroy_plan(forwardPlan);
        fftw_destroy_plan(inversePlan);
    };
    PitchDetection (const PitchDetection&) = delete;
    PitchDetection& operator= (const PitchDetection&) = delete;
};