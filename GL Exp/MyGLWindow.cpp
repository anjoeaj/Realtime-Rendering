#include "MyGLWindow.h"



MyGLWindow::MyGLWindow()
{
	width = 800;
	height = 600;
	animationMode = false;
	ortho = false;

	for (size_t i = 0; i < 1024; i++) {
		keys[i] = 0; //init
	}
}

MyGLWindow::MyGLWindow(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	for (size_t i = 0; i < 1024; i++) {
		keys[i] = 0; //init
	}
}


int MyGLWindow::Initialise() {
	//Init GLFW
	if (!glfwInit()) {
		printf("GLFW init failed");
		glfwTerminate();
		return 1;
	}

	//Setup OpenGL window props
	//Version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//Core profile means no backward compatibility. It helps when using deprecated functions.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//But allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//create the main window
	mainWindow = glfwCreateWindow(width, height, "Real-time Rendering", NULL, NULL);

	if (!mainWindow) {
		printf("GLFW window creation failed");
		glfwTerminate();
		return 1;
	}

	//Get buffer size
	
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	//Set context for GLFW to use
	glfwMakeContextCurrent(mainWindow);

	//Allow modern extension features
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		printf("GLEW init failed.");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	//wireframe mode 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Setup viewport size
	glViewport(0, 0, bufferWidth, bufferHeight);

	//enable keyboard and mouse callbacks
	createCallbacks();

	//This is for the cursor to disappear.
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//this is for the user input to identify the calling class
	glfwSetWindowUserPointer(mainWindow, this); 
}

void MyGLWindow::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, keyPressed);
	glfwSetCursorPosCallback(mainWindow, handleMouse);
}

GLfloat MyGLWindow::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat MyGLWindow::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}



void MyGLWindow::keyPressed(GLFWwindow* window, int key, int scancode, int action, int mods) {

	//Get the context
	MyGLWindow* theWindow = static_cast<MyGLWindow*>(glfwGetWindowUserPointer(window));


	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); //press escape to exit
	}
	else if (key == GLFW_KEY_R) {

		theWindow->animationMode = true;

	}
	else if (key == GLFW_KEY_T) {

		theWindow->animationMode = false;

	}
	else if (key == GLFW_KEY_O) {

		theWindow->ortho = true;

	}
	else if (key == GLFW_KEY_P) {

		theWindow->ortho = false;

	}
	else if (key == GLFW_KEY_Z) {

		theWindow->updateMaterials = true;
		theWindow->boostMaterials = false;
	}
	else if (key == GLFW_KEY_X) {

		theWindow->updateMaterials = true;
		theWindow->boostMaterials = true;
	}


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}

}

void MyGLWindow::handleMouse(GLFWwindow* window, double xPos, double yPos) {
	//Get the context
	MyGLWindow* theWindow = static_cast<MyGLWindow*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX; //change in x since the last position
	theWindow->yChange = theWindow->lastY - yPos; //change in y since the last position

	//Set last coordinate as current coordinate - This is for the next move
	theWindow->lastX = xPos;
	theWindow->lastY = yPos;

}


MyGLWindow::~MyGLWindow()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
