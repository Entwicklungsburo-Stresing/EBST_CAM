#pragma once

#include "shared_src/Board.h"
#include "CCDexamp.h"

int YVal( unsigned long db, int pixeli );
void Display( unsigned long db, BOOL Plot );
void CopytoDispbuf();
void UpdateTxT( void );
void UpdateDisplay();
unsigned int __stdcall UpdateDisplayThread( void *parg );
int GetCursorPosition();
void initCamera();
void initMeasurement();
void startMess( void * dummy );
