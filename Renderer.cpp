#include "Renderer.h"
#include "bloom.h"
#include "GameManager.h"

const int screenHeight = 480;
const int screenWidth = 854;
//Gaussian kernel

//Shader Paths
const char* phongVert = "Shaders/PhongLighting.vert";
const char* phongFrag = "Shaders/PhongLighting.frag";
const char* GUIVert = "Shaders/GUIShader.vert";
const char* GUIFrag = "Shaders/GUIShader.frag";
const char* screenVert = "Shaders/screenShader.vert";
const char* screenFrag = "Shaders/screenShader.frag";
const char* gaussianVert = "Shaders/gaussianBlur.vert";
const char* gaussianFrag = "Shaders/gaussianBlur.frag";

Renderer::Renderer() {
	//Creates a quad that can be rendered onto the screen
	//Useful for displaying framebuffers
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    createFrameBuffers();
    createPrograms();
}

void Renderer::createFrameBuffers() {
	//The gaussian blur requires two framebuffers that are switched between a handful of times before rendering
	//The blurring shader is toggled between blurring horizontally and vertically
	gaussianHorizontalBuffer.emplace(1);
	gaussianVerticalBuffer.emplace(1);

	//This is the framebuffer where everything is initially rendered to
	//The ObjectManager renders to this framebuffer, this framebuffer is not displayed to the user.
	renderFramebuffer.emplace(2);

	//This creates the renderbuffer needed to display each framebuffer
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
	
	//Tells OpenGL I'm interested in rendering to two textures (or colour attachments)
	//The colour attachments are how I control what gets rendered to each texture
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
}

void Renderer::createPrograms() {
    //Loads in the gaussian vertex and fragment shaders, this program creates the bloom effect by blurring certain objects on the screen
	//Update the weights of the kernel inside the gaussian blur program
	gaussianProgram.emplace(gaussianVert, gaussianFrag);
	updateGaussianKernel(3.f, gaussianProgram.value());

	//Loads the program that is responsible for displaying the final framebuffer to the user
	screenProgram.emplace(screenVert, screenFrag);
	//Because the screenShader combines the bloomed texture and the rendered texture, it needs access to both textures
	//Here I specify which colour attachment belongs to which texture
	screenProgram->setInt("screenTexture", 0);
	screenProgram->setInt("bloomBlur", 1);

	//Program responsible for displaying GUI Elements like text and images
	//Uses orthogonal projection. (Orthogonal projection makes it so that no matter how far away an object is from the screen, it's the same size)
	guiProgram.emplace(GUIVert, GUIFrag);
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(500.0f), 0.0f, static_cast<float>(500.0f));
	guiProgram->setMat4("textprojection", textProjection);

	//Shader program initialisation
	shaderProgram.emplace(phongVert, phongFrag);
}

void Renderer::displayFrameBuffer() {
	//Unbinds any previously bound vertex array object
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0); //Activates the first texture slot
	//Binds the vertex array and buffer into OpenGL for rendering, we're telling OpenGL we want to render a quad
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	//Buffer in the vertex data of the quad I want to render to the screen.
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	//Finally draw it, there are 6 vertices required to draw a quad, 3 for each face (since we're drawing with exclusively triangles).
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::render(GameManager& gameManager) {
	//Binds the framebuffer that I want the ObjectManager to render every object to
	renderFramebuffer->bind();
	//Tells OpenGL to clear the screen completely and replace it with black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear all information about what colour the image is and clear information about which pixel of the previous frame was closest to the camera
	//If Object 1 is behind Object 2, but Object 1 is rendered after Object 2. Object 1 will appear on top of Object 2
	//This feature is provided by OpenGL so that if a pixel is supposed to be behind another object. OpenGL will ignore it
	//TODO: This can always stay on, so put this in some sort of setup
	glEnable(GL_DEPTH_TEST);

	gameManager.render();
	
	//Gaussian blur
	//The guassian blur fragment shader is called repeatedly to blur the image drawn to Colour Attachment 1, switching between blurring horizontally and vertically
	gaussianProgram->use();
	glBindTexture(GL_TEXTURE_2D, renderFramebuffer->getAttachment1ID());
	for (int i = 0; i < 25; i++) {
		gaussianHorizontalBuffer->bind();
		gaussianProgram->setInt("horizontal", true);
		displayFrameBuffer();

		glBindTexture(GL_TEXTURE_2D, gaussianHorizontalBuffer->getAttachment0ID());
		gaussianVerticalBuffer->bind();
		gaussianProgram->setInt("horizontal", false);
		displayFrameBuffer();

		glBindTexture(GL_TEXTURE_2D, gaussianVerticalBuffer->getAttachment0ID());
	}
	
	//This is the final render to the screen
	//The screen program (vertex shader and fragment shader) combines the Colour Attachment 0 texture with the blurred Colour Attachment 1 texture
	//This gives the completed bloom effect
	screenProgram->use();
	Framebuffer::bindRenderFramebuffer();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderFramebuffer->getAttachment0ID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gaussianVerticalBuffer->getAttachment0ID());
	displayFrameBuffer();
	//Clears the vertex buffer and clears the texture buffer
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	//Swaps the display buffer so the user is finally presented with the image
	glutSwapBuffers();
	//Clear any rendering commands in the GPU that may still be being processed
	glFlush();
}

Renderer::~Renderer() {}