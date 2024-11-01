#include "pch.h"

#include <core/types.h>

#include <fmod/win/fsbank/inc/fsbank.h>

void compile_sound(String path)
{
	//FSBank_Init(FSBANK_FSBVERSION_FSB5, FSBANK_INIT_NORMAL, 16, ".fscache");

	
	FSBANK_SUBSOUND s;
	s.numFiles = 1;


	//FSBank_Build(&s, 1, FSBANK_FORMAT_FADPCM, FSBANK_BUILD_FSB5_DONTWRITENAMES, 0);
}
