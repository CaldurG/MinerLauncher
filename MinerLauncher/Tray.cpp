#include "Tray.h"
#include "MinerLauncher.h"

Tray* tray;

BOOL Tray::updateMenuItem(UINT id, const char* text)
{
  MENUITEMINFO item;

  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = id;
  item.dwTypeData = (LPSTR)text;

  return SetMenuItemInfoA(hMenu, id - ID_TRAY_FIRST, TRUE, &item);
}

LRESULT CALLBACK Tray::trayProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_TRAY_CALLBACK_MESSAGE:
    if ((lParam == WM_LBUTTONUP) || (lParam == WM_RBUTTONUP))
    {
      POINT point;
      GetCursorPos(&point);
      SetForegroundWindow(hWnd);
      DWORD cmd = TrackPopupMenu(tray->hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0, hWnd, NULL);
      switch (cmd)
      {
      case ID_TRAY_CLOSE:
        SendMessageA(hWnd, WM_CLOSE, 0, 0);
        break;
      case ID_TRAY_MINE:
        if (mining())
        {
          CloseMiner();
          tray->updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_START);
        }
        else
        {
          StartMiner();
          tray->updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_STOP);
        }
        break;
      }
      return 0;
    }
    break;
  }

  return DefWindowProcA(hWnd, msg, wParam, lParam);
}

Tray::Tray(const char* icoPath)
{
  tray = this;
  WNDCLASSEXA wndCls;

  ZeroMemory(&wndCls, sizeof(wndCls));
  wndCls.cbSize = sizeof(wndCls);
  wndCls.lpfnWndProc = trayProc;
  wndCls.hInstance = GetModuleHandleA(NULL);
  wndCls.lpszClassName = WC_TRAY_CLASS_NAME;
  if (!RegisterClassExA(&wndCls))
    throw "RegisterClassExA failed";

  hMenu = CreatePopupMenu();
  if (!hMenu)
    throw "CreatePopupMenu failed";

  MENUITEMINFO item;
  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = ID_TRAY_CLOSE;
  item.dwTypeData = (LPSTR)"Close";

  if (!InsertMenuItemA(hMenu, ID_TRAY_CLOSE - ID_TRAY_FIRST, TRUE, &item))
    throw "InsertMenuItemA failed";

  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = ID_TRAY_MINE;
  if (mining())
    item.dwTypeData = (LPSTR)TRAY_TEXT_STOP;
  else
    item.dwTypeData = (LPSTR)TRAY_TEXT_START;

  if (!InsertMenuItemA(hMenu, ID_TRAY_MINE - ID_TRAY_FIRST, TRUE, &item))
    throw "InsertMenuItemA failed";

  HWND hWnd = CreateWindowExA(0, WC_TRAY_CLASS_NAME, NULL, 0, 0, 0, 0, 0, NULL, hMenu, NULL, NULL);
  if (hWnd == NULL)
    throw "CreateWindowExA failed";

  if (!ExtractIconExA(icoPath, 0, NULL, &hIcon, 1))
    throw "ExtractIconExA failed";

  notifyIconData.cbSize = sizeof(notifyIconData);
  notifyIconData.hWnd = hWnd;
  notifyIconData.uID = 0;
  notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  notifyIconData.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
  notifyIconData.hIcon = hIcon;
  memcpy(notifyIconData.szTip, TRAY_TOOLTIP, sizeof(TRAY_TOOLTIP));

  Shell_NotifyIconA(NIM_ADD, &notifyIconData);
  UpdateWindow(hWnd);
}

Tray::~Tray()
{
  tray = NULL;
  Shell_NotifyIconA(NIM_DELETE, &notifyIconData);  
  
  if (hIcon)
    DestroyIcon(hIcon);

  if (hMenu)
    DestroyMenu(hMenu);

  UnregisterClass(WC_TRAY_CLASS_NAME, GetModuleHandleA(NULL));
}


