// WindowsLayoutsCPP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include <vector>
#include <tuple>
#include <conio.h>
#include <chrono>
#include <thread>
#include <algorithm>
using namespace std;

typedef tuple<HWND, WINDOWPLACEMENT, RECT> winTup;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	vector<winTup> *infoVector = (vector<winTup>*) lParam;
	WINDOWINFO info; //needed to filter out windows that aren't "real windows
	GetWindowInfo(hWnd, &info);
	if (!((info.dwStyle & (WS_VISIBLE + WS_CAPTION)) == (WS_VISIBLE + WS_CAPTION))) return TRUE;

	WINDOWPLACEMENT placement;
	RECT rect; //needed because Microsoft is incompetent and didn't expose an API for Aero Snap.
	GetWindowPlacement(hWnd, &placement);
	GetWindowRect(hWnd, &rect);
	infoVector->push_back(make_tuple(hWnd, placement, rect));

	//this stuff below is for when you want to print stuff out.
	//	char title[100];
	//	GetWindowTextA(hWnd, title, sizeof(title));
	//	char className[100];
	//	RealGetWindowClassA(hWnd, className, sizeof(className));
	//	cout << title <<endl;

	return TRUE;
}



vector<winTup> getAllWindows()
{
	vector<winTup> infoVec;
	EnumWindows(EnumWindowsProc, (LPARAM)&infoVec);
	return infoVec;
}

int moveAllWindows(vector<winTup> infoVec)
{
	cout << "doing shit with windows" << endl;
	vector<winTup> curVec = getAllWindows();
	for (winTup tup : curVec)
	{
		HWND hWnd = std::get<0>(tup);
		vector<winTup>::iterator newPosIter = std::find_if(infoVec.begin(), infoVec.end(), [hWnd](const winTup & tup) {
			return std::get<0>(tup) == hWnd; //checking if hWnd in the infoVec;
		});
		winTup savedTup = *newPosIter;
		if (newPosIter != infoVec.end())//just set the window where it's supposed to go
		{
			RECT tmpRect = std::get<2>(savedTup);

			int showCMD = std::get<1>(savedTup).showCmd;
			if (showCMD == SW_MAXIMIZE || showCMD == SW_SHOWMAXIMIZED) {
				ShowWindowAsync(hWnd, showCMD);				//if it was maximized
				SetWindowPos(hWnd, HWND_TOPMOST, tmpRect.left, tmpRect.top, tmpRect.right - tmpRect.left, tmpRect.bottom - tmpRect.top, SWP_NOZORDER);
			}
			else if (showCMD == SW_MINIMIZE || showCMD == SW_SHOWMINNOACTIVE || showCMD == SW_SHOWMINIMIZED)	ShowWindowAsync(hWnd, SW_SHOWMINIMIZED);	//if it was minimized
			else {
				ShowWindowAsync(hWnd, SW_RESTORE);				//if it was normal
				SetWindowPos(hWnd, HWND_TOPMOST, tmpRect.left, tmpRect.top, tmpRect.right - tmpRect.left, tmpRect.bottom - tmpRect.top, SWP_NOZORDER);
			}


		}
		else //oh no the window isn't on the list! you know what to do bouncer. "minimize" it ;)
		{
			ShowWindowAsync(hWnd, SW_SHOWMINNOACTIVE);
		}
	}
	return 0;

}

int main()
{
	enum { KEY_LEFT = 0, KEY_RIGHT = 1, KEY_UP = 2 }; //for hotkey events

	vector<vector<winTup>> infoVecs; //where we store all the window positions and whether it was minimized

	int curLayoutIdx = 0;

	infoVecs.push_back(getAllWindows()); //make sure we have something stored at least. makes iteration easier.

	cout << "got das windows " << infoVecs.size() << endl;

	RegisterHotKey(NULL, KEY_LEFT, MOD_ALT | MOD_WIN, VK_LEFT);//register the hotkeys for when stuff happens.
	RegisterHotKey(NULL, KEY_RIGHT, MOD_ALT | MOD_WIN, VK_RIGHT);
	RegisterHotKey(NULL, KEY_UP, MOD_ALT | MOD_WIN, VK_UP);
	MSG msg; //for storing messages
	cout << "ready to do stuff" << endl;
	while (GetMessage(&msg, 0, 0, 0))
	{
		cout << "key event: " << endl;

		if (msg.message == WM_HOTKEY)
		{
			//gotta save the current layout regardless of what new one youre going to.
			infoVecs.at(curLayoutIdx) = getAllWindows();

			switch (msg.wParam)
			{
			case KEY_LEFT:
				cout << "left" << endl;
				if (curLayoutIdx < 1) curLayoutIdx = infoVecs.size() - 1;
				else curLayoutIdx -= 1;
				moveAllWindows(infoVecs.at(curLayoutIdx));
				cout << "done" << endl;
				break;

			case KEY_RIGHT:
				cout << "right" << endl;
				//						infoVecs.size() isn't a signed int, so no negatives allowed here.
				if (curLayoutIdx + 1 >= infoVecs.size()) curLayoutIdx = 0;
				else curLayoutIdx += 1;
				moveAllWindows(infoVecs.at(curLayoutIdx));
				cout << "done" << endl;
				break;
			case KEY_UP:
				//we need to insert to the right.
				cout << "up" << endl;
				infoVecs.insert(infoVecs.begin() + curLayoutIdx, getAllWindows());
				curLayoutIdx += 1;
				cout << "done" << endl;
				break;
			}

			cout << "current index: " << curLayoutIdx << endl;
		}
	}
	return 0;
}

