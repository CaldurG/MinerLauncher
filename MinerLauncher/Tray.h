#pragma once

#include "framework.h"

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)
#define WC_TRAY_CLASS_NAME "MINER_LAUNCHER_TRAY"
#define ID_TRAY_FIRST 1000
#define ID_TRAY_MINE ID_TRAY_FIRST
#define ID_TRAY_CLOSE (ID_TRAY_FIRST + 1)
#define TRAY_TOOLTIP "Miner Launcher"
#define TRAY_TEXT_START "Start Mining"
#define TRAY_TEXT_STOP "Stop Mining"

class Tray
{
  NOTIFYICONDATA notifyIconData;
  HMENU hMenu = NULL;
  HICON hIcon = NULL;

public:
  Tray(const char* icoPath);
  ~Tray();
  BOOL updateMenuItem(UINT id, const char* text);

private:
  static LRESULT CALLBACK trayProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);  
};