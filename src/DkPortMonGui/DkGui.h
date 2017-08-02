#ifndef __DKGUI_H__
#define __DKGUI_H__

#include "DkWind.h"
#include "DkPortClient.h"

void DkInitializeComponents(void);
void DkPortOnDataReceived(void);
void DkWindOnStartMenu(void);
void DkWindOnStopMenu(void);
void DkWindOnClearMenu(void);
void DkWindOnClose(bool &bCanClose);

void DkWindOnSelectPortMenu(void);
INT_PTR CALLBACK SelectBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

DWORD DkEnumPortAndAddToListBox(HWND hPar);

const WCHAR HexWChar[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', 
							L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

#endif  // End of __DKGUI_H__