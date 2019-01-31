#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include<GL\glew.h>
#include<GLFW\glfw3.h>

#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "MyGLWindow.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "Model.h"

#include <assimp/Importer.hpp>

const GLint WIDTH = 800, HEIGHT = 600;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
MyGLWindow mainWindow;

Camera camera;

Texture goldTexture;
Texture dirtTexture;

Light mainLight;

Material shinyMaterial;
Material dullMaterial;
Material cookTorranceMaterial;
Material toonMaterial;

Model copter;
Model snowTerrain;

//To control the speed of cam movement
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat animationFactor, flipAnimationFactor = 0.0f;
bool animToggle = false;


//Vertex Shader
static const char * vertexShader = "Shaders/shader.vs";

//Fragment Shader
static const char * fragmentShader = "Shaders/shader.fs";

void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);// find the cross product of any 2 lines joining the triangles
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vertexShader, fragmentShader);
	shaderList.push_back(*shader1);
}

void CreateObjects() {

	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	}; 

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	// a floor was created using coordinates - not used for assignment
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	/*unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
	};*/

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

//This function is to apply a scale factor for perspective projection x axis when viewed in split-view
GLfloat orthoXFactor() {
	if (mainWindow.ortho) {
		return 2.0;
	}
	else {
		return 1.0;
	}
}

/*
glm::mat4 applyTranslation(glm::mat4 model, Axis axis) {
	switch (axis)
	{
	case X:
		model = glm::translate(model, glm::vec3(triOffset, 0.0f, 0.0f));
		break;
	case Y:
		model = glm::translate(model, glm::vec3(0.0f, triOffset, 0.0f));
		break;
	case Z:
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, triOffset));
		break;
	case RESET:
		break;
	default:
		break;
	}
	
	return model;
}

glm::mat4 applyRotation(glm::mat4 model, Axis axis) {
	switch (axis)
	{
	case X:
		model = glm::rotate(model, 0.65f, glm::vec3(1.0f, 0.0f, 0.0f));
		break;
	case Y:
		model = glm::rotate(model, 0.65f, glm::vec3(0.0f, 1.0f, 0.0f));
		break;
	case Z:
		model = glm::rotate(model, 0.65f, glm::vec3(0.0f, 0.0f, 1.0f));
		break;
	case RESET:
		break;
	default:
		break;
	}

	return model;
}

glm::mat4 applyScale(glm::mat4 model, Axis axis) {
	switch (axis)
	{
	case X:
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		break;
	case Y:
		model = glm::scale(model, glm::vec3(2.0f, 0.5f, 1.0f));
		break;
	case Z:
		break;
	case RESET:
		break;
	default:
		break;
	}

	return model;
}
*/
int main() {

	mainWindow = MyGLWindow(1366, 768);
	mainWindow.Initialise();

	mainWindow.ortho = false; //To decide between split view and full screen
	mainWindow.animationMode = false; // animate the pyramids inside the house

	//mainWindow1 = MyGLWindow(1366, 768);
	//mainWindow1.Initialise();
	
	CreateObjects();
	CreateShaders();
	
	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 2.0f, 0.2f);

	goldTexture = Texture((char *)"Textures/gold.png");
	goldTexture.LoadTextureA();

	dirtTexture = Texture((char *)"Textures/dirt.png");
	dirtTexture.LoadTextureA();


	shinyMaterial = Material(1.9f, 16, 0);
	dullMaterial = Material(0.3f, 4, 0);
	toonMaterial = Material(0.8f, 32, 1);
	cookTorranceMaterial = Material(1.0f, 32, 2);
	cookTorranceMaterial.SetCookTorranceParams(glm::vec3(1.0f, 1.0f, 1.0f), 0.8, 0.2);

	copter = Model();
	//snowHouse.LoadModel("Models/Snow covered CottageOBJ.obj");

	copter.LoadModel("Models/copter.obj");

	
	snowTerrain = Model();
	//snowTerrain.LoadModel("Models/SnowTerrain.obj");

	mainLight = Light(1.0f,1.0f,1.0f,0.7f, //color and intensity of the light
				2.0f, -1.0f, -2.0f, 0.4f);// direction and intensity of the light
	
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
			uniformAmbientIntensity = 0, uniformAmbientColour = 0,
			uniformDirection = 0, uniformDiffuseIntensity = 0,
			uniformSpecularIntensity = 0, uniformShininess = 0,
			uniformIlluminationType = 0;
	
	GLuint uniformAlbedo = 0, uniformRoughness = 0, uniformMetallic = 0;
	
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);
	glm::mat4 othoprojection = glm::ortho(-2.2f, 2.2f, -2.2f, 2.2f, 0.1f, 100.0f);

	Assimp::Importer importer = Assimp::Importer();
	while (!mainWindow.getShouldClose()) {
		
		//Time synchronization for movement
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		//Handle user input events
		glfwPollEvents();
		
		camera.keyControl(mainWindow.getsKeys(),deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		//Clear window
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformAmbientIntensity = shaderList[0].GetAmbientIntensityLocation();
		uniformAmbientColour = shaderList[0].GetAmbientColourLocation();
		uniformDirection = shaderList[0].GetDirectionLocation();
		uniformDiffuseIntensity = shaderList[0].GetDiffuseIntensityLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();
		uniformIlluminationType = shaderList[0].GetIlluminationTypeLocation();

		//cook torrance params
		uniformAlbedo = shaderList[0].GetAlbedoLocation();
		uniformRoughness = shaderList[0].GetRoughnessLocation();
		uniformMetallic = shaderList[0].GetMetallicLocation();

		mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColour, uniformDiffuseIntensity, uniformDirection);
		
		if (mainWindow.updateMaterials) {
			//call material.update
			if (mainWindow.boostMaterials) {
				//increase values
				shinyMaterial.UpdateMaterial(0.2f, 2, 0);
				//dullMaterial.UpdateMaterial(0.2f, 2, 0);
				//toonMaterial.UpdateMaterial(0.2f, 2, 1);
			}
			else {
				//decrease values
				shinyMaterial.UpdateMaterial(-0.2f, 1/2, 0);
				
			}
			mainWindow.updateMaterials = false;
		}

		if (mainWindow.animationMode) {
			if (animationFactor >= 360.0)
				animationFactor = 0;
			animationFactor += 0.01;

			if (flipAnimationFactor >= 40.0 || flipAnimationFactor < -40.0)
				animToggle = !animToggle;


			if (animToggle)
				flipAnimationFactor += 0.01;
			else
				flipAnimationFactor -= 0.01;
		}

		//Handle ortho mode
		if (mainWindow.ortho) {
			glViewport(0, 0, mainWindow.getBufferWidth() / 2, mainWindow.getBufferHeight());
		}
		else {
			glViewport(0, 0, mainWindow.getBufferWidth(), mainWindow.getBufferHeight());
		}
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(uniformEyePosition, camera.getCameraPostion().x, camera.getCameraPostion().y, camera.getCameraPostion().z);
		
		//--------------Pyramid-------------------
			//create identity matrix and use it to translate based on offset value
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));
			model = glm::rotate(model, animationFactor, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(orthoXFactor() * 0.25f, 0.25f, 0.25f));
			
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			goldTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			meshList[0]->RenderMesh();
			
			//Second transformation matrix for the second triangle
			//model = glm::mat4(1.0f);

			glm::mat4 rootModel = model;
			

			//----------------Top Small Pyramid----------------
			model = glm::translate(rootModel, glm::vec3(0.0f, 1.1f, 0.0f));
			model = glm::rotate(model, animationFactor*3, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(orthoXFactor() * 0.125f, 0.125f, 0.125f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			goldTexture.UseTexture();
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			meshList[1]->RenderMesh();


			

			//Floor
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
			model = glm::scale(model, glm::vec3(orthoXFactor() * 1.0f, 1.0f, 1.0f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			dirtTexture.UseTexture();
			dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			meshList[2]->RenderMesh();

			//Snow terrain
			/*
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
			model = glm::scale(model, glm::vec3(orthoXFactor() * 1.0f, 1.0f, 1.0f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			meshList[2]->RenderMesh();
			//snowTerrain.RenderModel();
			*/
			
			//Helicopter
			model = glm::mat4(1.0f);
			
			model = glm::scale(model, glm::vec3(orthoXFactor() * 0.10f, 0.10f, 0.10f));
			model = glm::rotate(model, animationFactor, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(-15.0f, 3.0f, 0.0f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			cookTorranceMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			//cookTorranceMaterial.UseCookTorranceMaterial(uniformAlbedo, uniformRoughness, uniformMetallic);
			//meshList[2]->RenderMesh();
			copter.RenderModel();


			//Helicopter 2
			model = glm::mat4(1.0f);

			model = glm::scale(model, glm::vec3(orthoXFactor() * 0.05f, 0.05f, 0.05f));
			model = glm::rotate(model, animationFactor, glm::vec3(0.0f, 0.5f, 0.0f));
			model = glm::translate(model, glm::vec3(-15.0f, 0.0f, 0.0f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			toonMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
			//cookTorranceMaterial.UseCookTorranceMaterial(uniformAlbedo, uniformRoughness, uniformMetallic);
			//meshList[2]->RenderMesh();
			copter.RenderModel();

			//--------------------//------------/-----////----
			//--------------------//------------/-----////----
			//--------------------//------------/-----////----
			/*
			if (mainWindow.ortho) {
				glViewport(mainWindow.getBufferWidth() / 2, 0, mainWindow.getBufferWidth() / 2, mainWindow.getBufferHeight());


				//glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(glm::lookAt(
				//	glm::vec3(-10.0, -6, 40.0), //camera position
				//	glm::vec3(0.0, 0.0, 0.0), // target
				//	glm::vec3(0.0, 0.0, -1.0))
				//));

				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(othoprojection));
				glUniform3f(uniformEyePosition, camera.getCameraPostion().x, camera.getCameraPostion().y, camera.getCameraPostion().z);


				//create identity matrix and use it to translate based on offset value
				model = glm::mat4(1.0f);

				model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));
				model = glm::rotate(model, animationFactor, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				goldTexture.UseTexture();
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[0]->RenderMesh();

				//Second transformation matrix for the second triangle
				//model = glm::mat4(1.0f);
				rootModel = model;
				model = glm::translate(rootModel, glm::vec3(-0.7f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				goldTexture.UseTexture();
				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[1]->RenderMesh();
				//-----------------------------------------
				model = glm::translate(rootModel, glm::vec3(0.6f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				goldTexture.UseTexture();
				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[1]->RenderMesh();

				//--------------------------------
				model = glm::translate(rootModel, glm::vec3(0.0f, 1.1f, 0.0f));
				model = glm::rotate(model, animationFactor, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				goldTexture.UseTexture();
				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[1]->RenderMesh();


				//---------------------------------

				//--------------------------------
				model = glm::translate(model, glm::vec3(0.0f, 1.4f, 0.0f));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				goldTexture.UseTexture();
				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				meshList[1]->RenderMesh();


				//---------------------------------

				//Floor
				//model = glm::mat4(1.0f);
				//model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
				////model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

				//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				//dirtTexture.UseTexture();
				//dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				//meshList[2]->RenderMesh();

				//Snow terrain
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
				//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				//meshList[2]->RenderMesh();
				snowTerrain.RenderModel();


				//House
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
				model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				//meshList[2]->RenderMesh();
				snowHouse.RenderModel();
			}
			*/


			glUseProgram(0);

		mainWindow.swapBuffers();

	}

	return 0;

}