#include "bloom.h"
#include "Utilities/ShaderLoader.h"
#include <valarray>
#include <string>

const size_t kernelSize = 5;

void updateGaussianKernel(float standardDeviation, Program& bloomProgram) {
	//The program needs to be loaded by OpenGL to update the values
	bloomProgram.use();

	//Where the gaussian values are stored
	std::valarray<float> kernelValues;
	kernelValues.resize(kernelSize);
	kernelValues[0] = 1.f;
	float sumValue = 1.f;
	//Because (when x = 0) is always 1 for out version of the distribution we can avoid this calculation
	for (size_t x = 1; x < kernelSize; x++) {
		float gaussianValue = gaussianDistribution(static_cast<float>(x), standardDeviation);
		kernelValues[x] = gaussianValue;
		//Multiply by two here because the gaussian kernel is symmetrical
		sumValue += 2 * gaussianValue;
	}

	//Normalise the kernel values so the sum of all the values (when expanded) is 1
	kernelValues /= sumValue;

	//Iterate through the kernel updating each value in the gaussian fragment shader's weight array
	//The weight array are what are used by the shader to calculate the gaussian blur
	for (size_t j = 0; j < kernelSize; j++) {
		//We need to access each weight value individually. So we create the weight location for the weight value we want
		//So weight[0] is the first value of our kernel
		std::string weightLocation = "weight[" + std::to_string(j) + "]";
		bloomProgram.setFloat(weightLocation, kernelValues[j]);
	}
}