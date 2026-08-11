#include <fftw3.h>
#include <span>
#include <vector>

const int sampleSize = 1024;

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
    float pitchFromBuffer(std::span<double>);

    //Debug
    double* getACF() const noexcept {return acfOut;}
};