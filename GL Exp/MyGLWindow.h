#pragma once

#include "stdio.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>
class MyGLWindow
{
public:
	GLFWwindow* mainWindow;
	MyGLWindow();
	MyGLWindow(GLint windowWidth, GLint windowHeight);

	int Initialise();

	GLint getBufferWidth() { return bufferWidth; }
	GLint getBufferHeight() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

	bool* getsKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();
	bool animationMode;
	bool updateMaterials;
	bool boostMaterials;
	bool ortho;
	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	bool flyThrough = true;

	bool getShowCursor();
	void setCursor(bool showCursor, GLFWwindow* window);
	void toggleShowCursor();
	void toggleFlyThrough();
	~MyGLWindow();

private:
	

	bool keys[1024];

	GLfloat lastX; //everytime the mouse moved, what's the last coordinate
	GLfloat lastY;
	GLfloat xChange; // how much has it changed since last movement?
	GLfloat yChange;
	bool mouseFirstMoved;
	bool showCursor = true;
	

	GLint width, height;
	GLint bufferWidth, bufferHeight;
	void createCallbacks();

	static void keyPressed(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);

};

