#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <strsafe.h>

#include "main.h"

#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include "../src/menu.h"
#include "../src/io_interface.h"


int gameOn = 1;
GLFWwindow* window = NULL;

#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define MOUSE_L_BUTTON 0
#define MOUSE_R_BUTTON 1
int buttonState[2];
int lastPosition[2];
void WIN_ReadInputs(){
	io_event_s event;

	int buttonIsPressed = KEYDOWN(VK_LBUTTON);
	static int buttonWasPressed = 0;


	//Get the mouse coordinates in screenspace.
	CURSORINFO pci;
	pci.cbSize = sizeof(pci);
	BOOL success = GetCursorInfo(&pci);

	if (!success){
		//WIN_CheckError("GetCursorInfo");
		return;
	}

	//Convert the screenspage to windowspace coordinates.
	success = ScreenToClient(WIN_GetHWND(), &pci.ptScreenPos);
	if (!success){
		//WIN_CheckError("ScreenToClient");
		return;
	}


	if (KEYDOWN(VK_ESCAPE) && engine.requiredSceneId != 0 && engine.sceneId != 0){
		MENU_Set(MENU_HOME);
		engine.requiredSceneId = 0;
	}


	if (buttonIsPressed)
	{


		if (!buttonWasPressed)
		{
			//This is a began event
			event.type = IO_EVENT_BEGAN;
			event.position[X] = pci.ptScreenPos.x;
			event.position[Y] = pci.ptScreenPos.y;



			//Log_Printf("Click: [%d,%d].\n",event.position[X],event.position[Y]);
			IO_PushEvent(&event);

		}
		else
		{
			//This is maybe a moved event.
			if (pci.ptScreenPos.x != lastPosition[X] ||
				pci.ptScreenPos.y != lastPosition[Y]
				)
			{
				event.type = IO_EVENT_MOVED;
				event.position[X] = pci.ptScreenPos.x;
				event.position[Y] = pci.ptScreenPos.y;
				event.previousPosition[X] = lastPosition[X];
				event.previousPosition[Y] = lastPosition[Y];
				IO_PushEvent(&event);
			}
		}

		lastPosition[X] = pci.ptScreenPos.x;
		lastPosition[Y] = pci.ptScreenPos.y;
		buttonWasPressed = 1;
	}
	else
	{
		if (buttonWasPressed)
		{
			//This is an end event
			event.type = IO_EVENT_ENDED;
			IO_PushEvent(&event);
		}
		buttonWasPressed = 0;
	}





	//Log_Printf("wc: %d,%d\n",pci.ptScreenPos.x,pci.ptScreenPos.y);
}

HWND WIN_GetHWND()
{
	HWND hwnd = glfwGetWin32Window(window);
	return hwnd;
}

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void Create_NativeWindow()
{
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

}

void Destroy_NativeWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void GLSwapBuffers()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}


int main(void)
{
	// game engine start
	unsigned char engineParameters = 0;
	char cwd[256];
	WORD nBufferLength=256;
	char lpBuffer[256];

	Create_NativeWindow();

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

	renderer.statsEnabled = 0;
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


		WIN_ReadInputs();
		dEngine_HostFrame();
		GLSwapBuffers();

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

	Destroy_NativeWindow();
	return 0;
}
