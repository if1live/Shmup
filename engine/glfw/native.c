#include "../src/native_services.h"
#include "../src/texture.h"

#include "windows.h"
#include "Mmsystem.h"

#include "lodepng.h"
void loadNativePNG(texture_t* tmpTex)
{
	unsigned int texid;
	char fullpath[256];

	//Shmup ask texture loading via relative path, we need to append the game directory.
	fullpath[0] = '\0';
	strcat(fullpath, FS_Gamedir());
	strcat(fullpath, "/");
	strcat(fullpath, tmpTex->path);

	unsigned error;
	unsigned char* image;
	unsigned width, height;

	error = lodepng_decode32_file(&image, &width, &height, fullpath);
	

	// make opengl texture??
	//ilGenImages(1, &texid); // Generation of one image name 
	//ilBindImage(texid); // Binding of image name 
	//ilLoadImage((const wchar_t*)fullpath);

	tmpTex->bpp = 4;
	tmpTex->width = width;
	tmpTex->height = height;
	tmpTex->numMipmaps = 1;

	tmpTex->data = (ubyte**)calloc(1, sizeof(ubyte*));
	tmpTex->data[0] = (ubyte*)calloc(tmpTex->width*tmpTex->height*tmpTex->bpp, sizeof(ubyte*));
	tmpTex->dataLength = 0;

	if (error) {
		Log_Printf("error %u: %s, '%s'\n", error, lodepng_error_text(error), tmpTex->path);
		return;
	}


	if (tmpTex->bpp == 4)
	{
		//lodepng always make RGBA image
		tmpTex->format = TEXTURE_GL_RGBA;
	}

	memcpy(tmpTex->data[0], (void*)image, tmpTex->width*tmpTex->height * tmpTex->bpp);

	//free(image);

	

	
}



//On windows there is no gamecenter, those method are here just to satisfy the linker.


int  Native_RetrieveListOf(char replayListIn[10][256])
{

	return 0;
}
void Native_UploadFileTo(char path[256]){}
void Action_ShowGameCenter(void* tag){}
void Native_UploadScore(uint score){}
void Native_LoginGameCenter(void){}


void SND_LogMCI_Error(char* commmand,int cmdResult)
{
	wchar_t errorMessage[128];

	mciGetErrorString(cmdResult,errorMessage,sizeof(errorMessage));

	Log_Printf("MCI Error '%ls' reason: '%s'\n",errorMessage,commmand);
}


char* format = "open \"%s\" type mpegvideo alias myFile ";
void SND_InitSoundTrack(char* filename)
{
	char command[256];
	int cmdResult;

	Log_Printf("[SND_InitSoundTrack] start '%s'.\n",filename);

	sprintf(command,format,filename);

	Log_Printf(command);
	Log_Printf("\n");

	cmdResult = mciSendStringA(command, NULL, 0, 0);

	if (cmdResult)
		SND_LogMCI_Error(command,cmdResult);
}

void SND_StartSoundTrack(void)
{
	int cmdResult;
	cmdResult = mciSendStringA("play myFile", NULL, 0, 0);

	if (cmdResult)
		SND_LogMCI_Error("play myFile",cmdResult);
}

void SND_StopSoundTrack(void)
{
	int cmdResult;
	cmdResult = mciSendStringA("stop myFile", NULL, 0, 0);
	if (cmdResult)
		SND_LogMCI_Error("stop myFile",cmdResult);

	cmdResult = mciSendStringA("close myFile", NULL, 0, 0);
	if (cmdResult)
		SND_LogMCI_Error("close myFile",cmdResult);
}