#pragma once

#include "shared_src/Board.h"
#include "Global.h"

int YVal( unsigned long db, int pixeli );
void Display( unsigned long db, BOOL Plot );
void CopytoDispbuf( ULONG scan );
void UpdateTxT( void );
int GetCursorPosition();
void initCamera();
void initMeasurement();
void startMess( void * dummy );
