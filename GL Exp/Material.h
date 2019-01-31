#pragma once

#include <GL\glew.h>
#include <glm\glm.hpp>

class Material
{
public:
	Material();

	Material(GLfloat sIntensity, GLfloat shine, GLuint type);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation, GLuint illuminationTypeLocation);
	void UpdateMaterial(GLfloat sIntensity, GLfloat shine, GLuint type);
	void UseCookTorranceMaterial(GLuint albedoLocation, GLuint roughnessLocation, GLuint metallicLocation);
	void SetCookTorranceParams(glm::vec3 albedo, GLfloat metallic, GLfloat roughness);

	~Material();
private:
	GLfloat specularIntensity;
	GLfloat shininess;
	glm::vec3 albedo;
	GLfloat metallic;
	GLfloat roughness;

	GLuint illuminationType;
};

