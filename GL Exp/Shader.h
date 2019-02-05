#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <GL\glew.h>
class Shader
{
public:
	Shader();
	void CreateFromString(const char* vertexCode, const char* fragmentCode);
	void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);

	std::string ReadFile(const char* fileLocation);

	GLuint GetProjectionLocation();
	GLuint GetModelLocation();
	GLuint GetViewLocation();
	GLuint GetAmbientIntensityLocation();
	GLuint GetAmbientColourLocation();
	GLuint GetDiffuseIntensityLocation();
	GLuint GetDirectionLocation();
	GLuint GetSpecularIntensityLocation();
	GLuint GetShininessLocation();
	GLuint GetEyePositionLocation();
	GLuint GetIlluminationTypeLocation();

	//Cook Torrance
	GLuint GetAlbedoLocation();
	GLuint GetRoughnessLocation();
	GLuint GetMetallicLocation();

	GLuint GetSkyBoxLocation();
	GLuint GetShaderID();
	void UseShader();
	void ClearShader();
	~Shader();

private:
	GLuint shaderID, uniformProjection, uniformModel, uniformView, uniformEyePosition,
		uniformAmbientIntensity, uniformAmbientColour, uniformDiffuseIntensity, uniformDirection,
		uniformSpecularIntensity, uniformShininess, uniformIlluminationType;

	GLuint uniformAlbedo, uniformRoughness, uniformMetallic;

	GLuint uniformSkyBox;
	
	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void setUniforms(GLuint shaderID);
	void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};

