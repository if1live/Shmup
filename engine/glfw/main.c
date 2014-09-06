#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <strsafe.h>

#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include "../src/menu.h"
#include "../src/io_interface.h"

#define WIN32_WINDOWS_WIDTH (320)
#define WIN32_WINDOWS_HEIGHT (480)

//#define WIN32_WINDOWS_WIDTH (768)
//#define WIN32_WINDOWS_HEIGHT (1024)

int gameOn;


#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define MOUSE_L_BUTTON 0
#define MOUSE_R_BUTTON 1
int buttonState[2];
int lastPosition[2];

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(WIN32_WINDOWS_WIDTH, WIN32_WINDOWS_HEIGHT, "Shmup", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	//glew init
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	// game engine start
	unsigned char engineParameters = 0;
	char cwd[256];
	WORD nBufferLength=256;
	char lpBuffer[256];

	//Create a console so we can see outputs.
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	HWND  consoleHandle = GetConsoleWindow();
	MoveWindow(consoleHandle, 1, 1, 680, 480, 1);
	printf("[main.cpp] Console initialized.\n");

	GetCurrentDirectoryA(nBufferLength, lpBuffer);
	memset(cwd, 0, 256);
	strcat(cwd, "RD=");
	strcat(cwd, lpBuffer);
	_putenv(cwd);

	memset(cwd, 0, 256);
	strcat(cwd, "WD=");
	strcat(cwd, lpBuffer);
	_putenv(cwd);

	engineParameters |= GL_11_RENDERER;
	//engineParameters |= GL_20_RENDERER;

	renderer.statsEnabled = 0;
	//renderer.materialQuality = MATERIAL_QUALITY_LOW;
	renderer.materialQuality = MATERIAL_QUALITY_HIGH;


	renderer.glBuffersDimensions[WIDTH] = WIN32_WINDOWS_WIDTH;
	renderer.glBuffersDimensions[HEIGHT] = WIN32_WINDOWS_HEIGHT;


	gameOn = 1;


	dEngine_Init();
	renderer.statsEnabled = 0;
	engine.licenseType = LICENSE_FULL;
	//This is only for windows build. Uses the viewport
	IO_Init();

	dEngine_InitDisplaySystem(engineParameters);

	renderer.props |= PROP_FOG;

	memset(buttonState, 0, sizeof(buttonState));

	/*
	The only complicated thing here is the time to sleep. timediff returned by the engine is telling us how long the frame should last.
	Since a PC can render a frame in 1-2ms we need to substract frame hosting duration to timediff (either 16 or 17ms) and sleep for this amount of time.
	*/
	while (gameOn && !glfwWindowShouldClose(window))
	{
		// Check the state of the mouse and its position, may generate a touch_t if the 
		// left button is pressed.
		unsigned long startFrame = timeGetTime();


		//TODO
		//WIN_ReadInputs();
		dEngine_HostFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();

		unsigned long endFrame = timeGetTime();

		unsigned long timeForFrame = endFrame - startFrame;

		int timeToSleep = timediff - timeForFrame;

		//Log_Printf("timeToSleep=%d\n",timeToSleep);
		// Game is clocked at 60Hz (timediff will be either 16 or 17, this value
		// comes from timer.c).
		if (timeToSleep > 0) {
			Sleep(timeToSleep);
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
