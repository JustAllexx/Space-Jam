#define _CRT_SECURE_NO_DEPRECATE
#define STB_IMAGE_IMPLEMENTATION
#include "ObjectLoader.h"

//This class takes in a wavefront file path as an argument
//The program decodes the wavefront file and outputs 3 vectors.
//Each vector has a size a multiple of 3, every 3 is a face that needs to be rendered and each vector contains details about where that face should be, how to texture it and which way it should be facing for lighting calculations
bool ObjectLoader::loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals)
{
	std::vector<glm::vec3> vertexArray;
	std::vector<glm::vec3> normalArray;
	std::vector<glm::vec2> uvArray;

	std::fstream in(path);
	std::string nextLine;
	while (getline(in, nextLine)) {
		std::stringstream data(nextLine);
		std::vector<std::string> splitData;
		std::string instruction;
		while (std::getline(data, instruction, ' ')) {
			splitData.push_back(instruction);
		}
		//If the first word of the line is a v we know it is describing a vertex
		//Isolates the vertex information and adds it to the vertex array
		if (splitData[0] == "v") {
			float xComponent = std::stof(splitData[1]);
			float yComponent = std::stof(splitData[2]);
			float zComponent = std::stof(splitData[3]);
			glm::vec3 vert = glm::vec3(xComponent, yComponent, zComponent);
			vertexArray.push_back(vert);
		}
		//If the first word is vt, then it is a UV coordinate and is added to the uvArray
		else if (splitData[0] == "vt") {
			float xComponent = std::stof(splitData[1]);
			float yComponent = std::stof(splitData[2]);
			glm::vec2 uv = glm::vec2(xComponent, yComponent);
			uvArray.push_back(uv);
		}
		//If the first word is vn, then is a normal direction and is added to the normal array
		else if (splitData[0] == "vn") {
			float xComponent = std::stof(splitData[1]);
			float yComponent = std::stof(splitData[2]);
			float zComponent = std::stof(splitData[3]);
			glm::vec3 normal = glm::vec3(xComponent, yComponent, zComponent);
			normalArray.push_back(normal);
		}
		//If the first word is f, it is describing a face
		//Face instructions use the indexes of previous data, specifically vertex (v), UV (u) and normal(n) data
		//this comes in the order v/u/n v/u/n v/u/n, we append each instruction to the out arrays for rendering
		else if (splitData[0] == "f") {
			int faceIndices[9];
			std::stringstream combinedData(splitData[1] + "/" + splitData[2] + "/" + splitData[3]);
			std::string index;
			int count = 0;
			//Get the indices in the form of integers
			while (std::getline(combinedData, index, '/')) {
				faceIndices[count] = std::stoi(index) - 1;
				count += 1;
			}
			out_vertices.push_back(vertexArray[faceIndices[0]]);
			out_uvs.push_back(uvArray[faceIndices[1]]);
			out_normals.push_back(normalArray[faceIndices[2]]);

			out_vertices.push_back(vertexArray[faceIndices[3]]);
			out_uvs.push_back(uvArray[faceIndices[4]]);
			out_normals.push_back(normalArray[faceIndices[5]]);

			out_vertices.push_back(vertexArray[faceIndices[6]]);
			out_uvs.push_back(uvArray[faceIndices[7]]);
			out_normals.push_back(normalArray[faceIndices[8]]);

		}
	}
	return false;
}

//Loads a texture file, buffers it into openGL and then returns the texture ID
//This function uses stbi to load png files.
GLuint ObjectLoader::loadTexture(const char* path)
{
	GLuint retTexture;
	stbi_set_flip_vertically_on_load(true);
	int imgWidth, imgHeight, numChannels;
	//Opens the images file in raw byte form, also returns information about image height, width and the number of colour channels in the image
	unsigned char* bytes = stbi_load(path, &imgWidth, &imgHeight, &numChannels, 0);

	//Generate the texture and then buffer in the loaded image
	glGenTextures(1, &retTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glGenerateMipmap(GL_TEXTURE_2D);
	//Explicitly free the memory because it isn't needed anymore and can take up a lot of memory if not explicitly cleared
	stbi_image_free(bytes);
	return retTexture;
}
