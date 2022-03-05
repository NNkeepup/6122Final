/*
Author: <Nuo Wang>
Class: ECE6122
Last Date Modified: <12/07/2021>
Description:
Source file with main function. 
This file sets up the GLFWwindow, runs ECE_UAV class, load obj files and renders textures.
Reference:http://www.opengl-tutorial.org/cn/intermediate-tutorials/
*/


// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <random>
#include "ECE_UAV.h"


void setAttributeBuffer(GLuint vertexbuffer, GLuint uvbuffer, GLuint normalbuffer,GLuint elementbuffer,
	std::vector<unsigned short> indices)
{
	/*
		@return void
		set 3 attrbute buffers, bind array buffer to element buffer
		and draw triangles.
	*/

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
}

int main(void)
{

	clock_t start, end;
	double time;
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Final Project", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.2f, 0.8f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	GLuint programID_ff = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	GLuint programID_transp = LoadShaders("StandardShading.vertexshader", "StandardTransparentShading.fragmentshader");//Sphere uses transparent shader

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	GLuint Texture_ff = loadBMP_custom("ff.bmp");
	GLuint Texture_sph = loadDDS("white.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read obj files
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<glm::vec3> vertices_ff;
	std::vector<glm::vec2> uvs_ff;
	std::vector<glm::vec3> normals_ff;
	bool res_ff = loadOBJ("footballfield.obj", vertices_ff, uvs_ff, normals_ff);

	
	std::vector<glm::vec3> vertices_sph;
	std::vector<glm::vec2> uvs_sph;
	std::vector<glm::vec3> normals_sph;
	bool res_sphere = loadOBJ("sphere.obj", vertices_sph, uvs_sph, normals_sph);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	std::vector<unsigned short> indices_ff;
	std::vector<glm::vec3> indexed_vertices_ff;
	std::vector<glm::vec2> indexed_uvs_ff;
	std::vector<glm::vec3> indexed_normals_ff;
	indexVBO(vertices_ff, uvs_ff, normals_ff, indices_ff, indexed_vertices_ff, indexed_uvs_ff, indexed_normals_ff);

	std::vector<unsigned short> indices_sph;
	std::vector<glm::vec3> indexed_vertices_sph;
	std::vector<glm::vec2> indexed_uvs_sph;
	std::vector<glm::vec3> indexed_normals_sph;
	indexVBO(vertices_sph, uvs_sph, normals_sph, indices_sph, indexed_vertices_sph, indexed_uvs_sph, indexed_normals_sph);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	//////////////////////////
	GLuint vertexbuffer_ff;
	glGenBuffers(1, &vertexbuffer_ff);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_ff);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices_ff.size() * sizeof(glm::vec3), &indexed_vertices_ff[0], GL_STATIC_DRAW);

	GLuint uvbuffer_ff;
	glGenBuffers(1, &uvbuffer_ff);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_ff);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs_ff.size() * sizeof(glm::vec2), &indexed_uvs_ff[0], GL_STATIC_DRAW);

	GLuint normalbuffer_ff;
	glGenBuffers(1, &normalbuffer_ff);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_ff);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals_ff.size() * sizeof(glm::vec3), &indexed_normals_ff[0], GL_STATIC_DRAW);

	////////////////////////////////////////
	GLuint vertexbuffer_sph;
	glGenBuffers(1, &vertexbuffer_sph);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_sph);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices_sph.size() * sizeof(glm::vec3), &indexed_vertices_sph[0], GL_STATIC_DRAW);

	GLuint uvbuffer_sph;
	glGenBuffers(1, &uvbuffer_sph);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_sph);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs_sph.size() * sizeof(glm::vec2), &indexed_uvs_sph[0], GL_STATIC_DRAW);

	GLuint normalbuffer_sph;
	glGenBuffers(1, &normalbuffer_sph);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_sph);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals_sph.size() * sizeof(glm::vec3), &indexed_normals_sph[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	GLuint elementbuffer_ff;
	glGenBuffers(1, &elementbuffer_ff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_ff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_ff.size() * sizeof(unsigned short), &indices_ff[0], GL_STATIC_DRAW);

	GLuint elementbuffer_sph;
	glGenBuffers(1, &elementbuffer_sph);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_sph);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_sph.size() * sizeof(unsigned short), &indices_sph[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	double lastFrameTime = lastTime;
	int nbFrames = 0;
	
	///initialize uav object
	computeMatricesFromInputs();
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	glUseProgram(programID);
	glm::mat4 ModelMatrix1 = glm::mat4(1.0);
	glm::mat4 modelOrigin = glm::mat4(1.0);
	ModelMatrix1 = scale(ModelMatrix1, vec3(0.2, 0.2, 0.2));
	modelOrigin = ModelMatrix1;
	glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(TextureID, 0);

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);
	setAttributeBuffer(vertexbuffer, uvbuffer, normalbuffer, elementbuffer, indices);
	std::vector<ECE_UAV*> uavs{15};
	
	 //Init the positions of 15 uavs, set thread ID  and start all threads
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			double posx = -50 + 25 * j;
			double posy = 25 - 25 * i;
			uavs[i * 5 + j] = new ECE_UAV;
			uavs[i * 5 + j]->setPosition(posx, posy, 0);
			uavs[i * 5 + j]->setThreadID(i * 5 + j);
			uavs[i * 5 + j]->startThread();
		
		}
	}
	//record the simulation time
	start = std::clock();
	
	do {

		// Measure speed
		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();


		////// Start of the rendering of the sphere object //////
		glUseProgram(programID_transp);
		glm::vec3 lightPos_sp = glm::vec3(1, 1, 50);
		glUniform3f(LightID, lightPos_sp.x, lightPos_sp.y, lightPos_sp.z);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture_sph);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		///Enable Blending to see the transparnt ball
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		glm::mat4 ModelMatrix_sphere = mat4(1.0);
		ModelMatrix_sphere = scale(ModelMatrix_sphere, vec3(0.7, 0.7, 0.7));
		ModelMatrix_sphere = translate(ModelMatrix_sphere, vec3(0, 0, 14.3));
		glm::mat4 MVP_sphere = ProjectionMatrix * ViewMatrix * ModelMatrix_sphere;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_sphere[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix_sphere[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
		setAttributeBuffer(vertexbuffer_sph, uvbuffer_sph, normalbuffer_sph, elementbuffer_sph, indices_sph);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		////// End of the rendering of the sphere  //////
	

		////// Start of the rendering of 15 uavs //////
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				glUseProgram(programID);

				//Change uavs' color
				double timeValue = glfwGetTime();
				double redValue = sin(timeValue) / 2.0f + 0.7f;
				// to change the color variable in shader file using light position
				glm::vec3 lightPos = glm::vec3(1, 1, 0);
				GLuint timeID = glGetUniformLocation(programID, "time");
				glUniform1f(timeID, timeValue);
				glUniform3f(LightID, redValue, lightPos.y, lightPos.z);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects

				//detect elastic collision 
				uavs[i * 5 + j]->elasticCollision(uavs, i * 5 + j);
				
				//update the world position of all uavs
				glm::mat4 modelMatrix_uav = translate(modelOrigin, 
					vec3(uavs[i * 5 + j]->getXPosition(),
						uavs[i * 5 + j]->getYPosition(),
						uavs[i * 5 + j]->getZPosition()));
				glm::mat4 MVP_uav = ProjectionMatrix * ViewMatrix * modelMatrix_uav;
				glBindTexture(GL_TEXTURE_2D, Texture);
				glUniform1i(TextureID, 0);
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_uav[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrix_uav[0][0]);
				setAttributeBuffer(vertexbuffer, uvbuffer, normalbuffer, elementbuffer, indices);
			}
		}
		////// End of the rendering of uavs  //////

		////// Start of the rendering of the footbal field  //////
		
		glUseProgram(programID_ff);
		glm::vec3 lightPos_ff = glm::vec3(1, 1, 50);
		glUniform3f(LightID, lightPos_ff.x, lightPos_ff.y, lightPos_ff.z);

		glm::mat4 ModelMatrix_ff = glm::mat4(1.0);
		ModelMatrix_ff = scale(ModelMatrix_ff, vec3(0.074, 0.074,0.074));
		ModelMatrix_ff = translate(ModelMatrix_ff, vec3(5, 0, 0));
		glm::mat4 MVP_ff = ProjectionMatrix * ViewMatrix * ModelMatrix_ff;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_ff[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix_ff[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture_ff);

		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		setAttributeBuffer(vertexbuffer_ff, uvbuffer_ff, normalbuffer_ff, elementbuffer_ff,
			indices_ff);
		////// End of rendering of the football field object //////

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		end = clock();
		time = (double)(end - start) / CLOCKS_PER_SEC;

		//render the scene every other 30 msec
		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		if (time > 70)
		{
			//if time > 70, make uav stop moving
			for (int n = 0; n < 15; ++n)
				uavs[n]->stopThread();
		}

	} // Check if the ESC key was pressed or the window was closed or the simulation time is over 80 secs
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 && time<80.0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

