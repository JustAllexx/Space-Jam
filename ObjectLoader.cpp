#define _CRT_SECURE_NO_DEPRECATE
#define STB_IMAGE_IMPLEMENTATION
#include "ObjectLoader.h"
#include <string>
#include <fstream>
#include <sstream>

//This class takes in a wavefront file path as an argument
//The program decodes the wavefront file and outputs 3 vectors.
//Each vector has a size a multiple of 3, every 3 is a face that needs to be rendered and each vector contains details about where that face should be, how to texture it and which way it should be facing for lighting calculations
Mesh::Mesh(const char* filepath)
{
	std::vector<glm::vec3> vertexArray;
	std::vector<glm::vec3> normalArray;
	std::vector<glm::vec2> uvArray;

	std::fstream in(filepath);
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
			size_t faceIndices[9];
			std::stringstream combinedData(splitData[1] + "/" + splitData[2] + "/" + splitData[3]);
			std::string index;
			int count = 0;
			//Get the indices in the form of integers
			while (std::getline(combinedData, index, '/')) {
				faceIndices[count] = static_cast<size_t>(std::stoi(index)) - 1;
				count += 1;
			}
			vertices.push_back(vertexArray[faceIndices[0]]);
			uvs.push_back(uvArray[faceIndices[1]]);
			normals.push_back(normalArray[faceIndices[2]]);

			vertices.push_back(vertexArray[faceIndices[3]]);
			uvs.push_back(uvArray[faceIndices[4]]);
			normals.push_back(normalArray[faceIndices[5]]);

			vertices.push_back(vertexArray[faceIndices[6]]);
			uvs.push_back(uvArray[faceIndices[7]]);
			normals.push_back(normalArray[faceIndices[8]]);

		}
	}

	//Create OpenGL resources
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	return;
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &normalBuffer);
}

Texture::Texture(const char* filepath) {
	STBIImage image(filepath);
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getImageWidth(), image.getImageHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getData());
	glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() {
	glDeleteTextures(1, &textureID);
}
