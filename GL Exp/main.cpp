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

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <windows.h>

//the below function gets the working directory
std::string getexepath()
{
	char result[256];
	return std::string(result, GetModuleFileNameA(NULL, result, 256));
}

// Use Nvidia graphics card on Windows
#ifdef _WIN32
extern "C" {
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif

const GLint WIDTH = 800, HEIGHT = 600;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
unsigned int loadCubemap(std::vector<std::string> faces);
void RenderFloor(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType);
void GetUniformLocations(GLuint &uniformModel, GLuint &uniformProjection, GLuint &uniformView, GLuint &uniformAmbientIntensity, GLuint &uniformAmbientColour, GLuint &uniformDirection, GLuint &uniformDiffuseIntensity, GLuint &uniformEyePosition, GLuint &uniformSpecularIntensity, GLuint &uniformShininess, GLuint &uniformIlluminationType, GLuint &uniformAlbedo, GLuint &uniformRoughness, GLuint &uniformMetallic);
void SetMaterialProps();
void SetAnimationParams();
void RenderHelicopterToon(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType);
void RenderHelicopterCookTorrance(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType);
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

Model skyBoxModelImported;
Model lowPolyBird;

//To control the speed of cam movement
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat animationFactor, flipAnimationFactor = 0.0f;
bool animToggle = false;


//Vertex Shader
static const char * vertexShader = "Shaders/shader.vs";
static const char * skyboxVertexShader = "Shaders/skybox.vs";

//Fragment Shader
static const char * fragmentShader = "Shaders/shader.fs";
static const char * skyboxFragmentShader = "Shaders/skybox.fs";

GLuint cubemapTexture;

//Credits - Learnopengl.com
//Faces for skybox texture
std::vector<std::string> skyBoxFaces
{
	fs::current_path().string() + "\\Textures\\skybox4\\right.jpg",
	fs::current_path().string() + "\\Textures\\skybox4\\left.jpg",
	fs::current_path().string() + "\\Textures\\skybox4\\top.jpg",
	fs::current_path().string() + "\\Textures\\skybox4\\bottom.jpg",
	fs::current_path().string() + "\\Textures\\skybox4\\front.jpg",
	fs::current_path().string() + "\\Textures\\skybox4\\back.jpg"
};

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

	Shader *shader2 = new Shader();
	shader2->CreateFromFiles(skyboxVertexShader, skyboxFragmentShader);
	shaderList.push_back(*shader2);
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

void CreateSkyBox() {

	//vertices
	GLfloat skyboxVertices[] = {
		          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	Mesh *skyBoxObject = new Mesh();
	skyBoxObject->CreateSkyBoxMesh(skyboxVertices);
	meshList.push_back(skyBoxObject);

	cubemapTexture = loadCubemap(skyBoxFaces);
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

void RenderFloor(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType)
{
	//Floor
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	model = glm::scale(model, glm::vec3(orthoXFactor() * 1.0f, 1.0f, 1.0f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
	meshList[2]->RenderMesh();
}

int main() {

	mainWindow = MyGLWindow(1366, 768);
	mainWindow.Initialise();

	mainWindow.ortho = false; //To decide between split view and full screen
	mainWindow.animationMode = false; // animate the pyramids inside the house

	//mainWindow1 = MyGLWindow(1366, 768);
	//mainWindow1.Initialise();
	
	CreateObjects();
	CreateSkyBox();
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

	skyBoxModelImported = Model();
	skyBoxModelImported.LoadModel("Models/cube.obj");

	lowPolyBird = Model();
	//lowPolyBird.LoadModel("Models/teapot.obj");
	//lowPolyBird.LoadModel("Models/male_head.obj");
	//lowPolyBird.LoadModel("Models/LowPolyBird.obj");
	lowPolyBird.LoadModel("Models/copter.obj");


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

	GLuint uniformSkyBox = 0;


	
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
		glClearColor(0.1f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		GetUniformLocations(uniformModel, uniformProjection, uniformView, uniformAmbientIntensity, uniformAmbientColour, uniformDirection, uniformDiffuseIntensity, uniformEyePosition, uniformSpecularIntensity, uniformShininess, uniformIlluminationType, uniformAlbedo, uniformRoughness, uniformMetallic);

		mainLight.UseLight(uniformAmbientIntensity, uniformAmbientColour, uniformDiffuseIntensity, uniformDirection);
		
		SetMaterialProps();

		SetAnimationParams();
		
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

		
		//shaderList[1].UseShader();
		//meshList[0]->RenderMesh();
		RenderHelicopterCookTorrance(model, uniformModel, uniformSpecularIntensity, uniformShininess, uniformIlluminationType);

		//Second transformation matrix for the second triangle
		//model = glm::mat4(1.0f);
		
		glm::mat4 rootModel = model;


		//RenderFloor(model, uniformModel, uniformSpecularIntensity, uniformShininess, uniformIlluminationType);

		//RenderHelicopterCookTorrance(model, uniformModel, uniformSpecularIntensity, uniformShininess, uniformIlluminationType);


		//RenderHelicopterToon(model, uniformModel, uniformSpecularIntensity, uniformShininess, uniformIlluminationType);

		//SkyBox Render
		//change depth function so depth test passes when values are equal to depth buffer's content
		glDepthFunc(GL_LEQUAL);
		//use skybox shader
		shaderList[1].UseShader();
		//convert the existing mat4 view matrix to mat3 and then back to mat4 to remove the translation values
		glm::mat4 skyBoxViewMat = glm::mat4(glm::mat3(camera.calculateViewMatrix()));
		
		//SkyBox uniform location
		uniformSkyBox = shaderList[1].GetSkyBoxLocation();
		uniformView = shaderList[1].GetViewLocation();
		uniformProjection = shaderList[1].GetProjectionLocation();
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(skyBoxViewMat));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(uniformSkyBox, 0);

		skyBoxModelImported.RenderModel();
		//meshList[3]->RenderSkyBox(cubemapTexture);
		
		
		glUseProgram(0);

		mainWindow.swapBuffers();

	}

	return 0;

}

void GetUniformLocations(GLuint &uniformModel, GLuint &uniformProjection, GLuint &uniformView, GLuint &uniformAmbientIntensity, GLuint &uniformAmbientColour, GLuint &uniformDirection, GLuint &uniformDiffuseIntensity, GLuint &uniformEyePosition, GLuint &uniformSpecularIntensity, GLuint &uniformShininess, GLuint &uniformIlluminationType, GLuint &uniformAlbedo, GLuint &uniformRoughness, GLuint &uniformMetallic)
{
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
}

void SetMaterialProps()
{
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
			shinyMaterial.UpdateMaterial(-0.2f, 1 / 2, 0);

		}
		mainWindow.updateMaterials = false;
	}
}

void SetAnimationParams()
{
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
}

void RenderHelicopterToon(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType)
{
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
}

void RenderHelicopterCookTorrance(glm::mat4 &model, const GLuint &uniformModel, const GLuint &uniformSpecularIntensity, const GLuint &uniformShininess, const GLuint &uniformIlluminationType)
{
	//Helicopter
	model = glm::mat4(1.0f);

	model = glm::scale(model, glm::vec3(orthoXFactor() * 0.10f, 0.10f, 0.10f));
	model = glm::rotate(model, animationFactor, glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::translate(model, glm::vec3(-15.0f, 3.0f, 0.0f));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	//shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	cookTorranceMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess, uniformIlluminationType);
	//cookTorranceMaterial.UseCookTorranceMaterial(uniformAlbedo, uniformRoughness, uniformMetallic);
	//meshList[2]->RenderMesh();
	lowPolyBird.RenderModel();
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------

//Credits - Learnopengl.com
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}