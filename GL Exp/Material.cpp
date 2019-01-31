#include "Material.h"



Material::Material()
{
	specularIntensity = 0.0f;
	shininess = 0.0f;
}

Material::Material(GLfloat sIntensity, GLfloat shine, GLuint type) {
	specularIntensity = sIntensity;
	shininess = shine;
	illuminationType = type;
}

void Material::UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation, GLuint illuminationTypeLocation)
{
	glUniform1f(specularIntensityLocation, specularIntensity);
	glUniform1f(shininessLocation, shininess);
	glUniform1i(illuminationTypeLocation, illuminationType);
}

void Material::UpdateMaterial(GLfloat sIntensity, GLfloat shine, GLuint type)
{
	specularIntensity += sIntensity;
	//shininess *= shine;
	//illuminationType += type;
}

void Material::UseCookTorranceMaterial(GLuint albedoLocation, GLuint roughnessLocation, GLuint metallicLocation)
{
	glUniform3f(albedoLocation, albedo.x, albedo.y, albedo.z);
	glUniform1f(roughnessLocation, roughness);
	glUniform1f(metallicLocation, metallic);
}

void Material::SetCookTorranceParams(glm::vec3 albedo_, GLfloat metallic, GLfloat roughness)
{
	albedo = albedo_;
	metallic = metallic;
	roughness = roughness;
}

Material::~Material()
{
}
