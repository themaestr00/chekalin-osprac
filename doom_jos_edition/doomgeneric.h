#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#ifndef DOOMGENERIC_RESX
#define DOOMGENERIC_RESX 320
#endif
#ifndef DOOMGENERIC_RESY
#define DOOMGENERIC_RESY 200
#endif

#include "inc/lib.h"

typedef uint32_t pixel_t;

extern pixel_t* DG_ScreenBuffer;

extern Window_ptr window;
extern Renderer_ptr renderer;
extern Texture_ptr texture;

void doomgeneric_Create(int argc, char** argv);
void doomgeneric_Tick();

// Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char* title);

#endif // DOOM_GENERIC
