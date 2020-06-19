#pragma once

#include "shared_src/Board.h"
#include "Global.h"

extern BYTE Dispcnt;
extern int yVal;
extern volatile int testcnt;
extern UINT choosen_board;
extern BOOL both_boards;
extern BOOL cont_mode;

void Resort_to_DBs( UINT drvno, void * p1dim, void * p2dim, BYTE db1, BYTE db2 );
int YVal( unsigned long db, int pixeli );
void Display( unsigned long db, BOOL Plot );
void CopytoDispbuf( ULONG scan );
void UpdateTxT( void );
int GetCursorPosition();
void initCamera();
void initMeasurement();
void startMess( void * dummy );
