#pragma once

#include <GL\glew.h>

#include "stb_image.h"

class Texture
{
public:
	Texture();
	Texture(const char* fileLoc);

	bool LoadTexture();
	bool LoadTextureNormal();
	bool LoadTextureA();
	void UseTexture();
	void ClearTexture();
	void SetFileLoc(const char* fileLoc);

	~Texture();

private:
	GLuint textureID[2];
	GLuint textureNormalID;
	int width, height, bitDepth;

	const char* fileLocation;
};

