#pragma once

#include <GL/glew.h>
class Mesh
{
public:
	Mesh();

	void CreateMesh(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();
	void CreateSkyBoxMesh(GLfloat * vertices);
	void RenderSkyBox(GLuint cubemapTexture);
	void RenderSkyBox1(GLuint cubemapTexture);
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLuint skyboxVAO, skyboxVBO;
	GLsizei indexCount;

};

