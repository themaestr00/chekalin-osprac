#include "inc/lib.h"
#include "m_argv.h"
#include "doomgeneric.h"

pixel_t* DG_ScreenBuffer = NULL;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	// DG_ScreenBuffer will be set in DG_Init by vga_texture_create
	DG_ScreenBuffer = NULL;

	DG_Init();

	D_DoomMain ();
}

