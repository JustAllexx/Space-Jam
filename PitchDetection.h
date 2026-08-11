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

public:
    PitchDetection();
    ~PitchDetection();

    void calculateACF(std::span<double> buffer);
    std::vector<double> calculateDifferenceFunction(std::span<double> buffer);
    std::vector<double> calculateCMNDF(std::span<double> buffer);
    std::optional<double> pitchFromBuffer(std::span<double>);

    //Debug
    double* getACF() const noexcept {return acfOut;}
};