#include "Tray.h"
#include "MinerLauncher.h"

const char* ICO_PATH = ".\\MinerLauncher.ico";

NOTIFYICONDATA nid;
HMENU hMenu;

BOOL updateMenuItem(UINT id, const char* text)
{
  MENUITEMINFO item;

  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = id;
  item.dwTypeData = (LPSTR)text;

  return SetMenuItemInfoA(hMenu, id - ID_TRAY_FIRST, TRUE, &item);
}

LRESULT CALLBACK trayProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
      DWORD cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0, hWnd, NULL);
      switch (cmd)
      {
      case ID_TRAY_CLOSE:
        SendMessageA(hWnd, WM_CLOSE, 0, 0);
        break;
      case ID_TRAY_MINE:
        if (mining())
        {
          CloseMiner();
          updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_START);
        }
        else
        {
          StartMiner();
          updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_STOP);
        }
        break;
      }
      return 0;
    }
    break;
  }

  return DefWindowProcA(hWnd, msg, wParam, lParam);
}

BOOL initMenu()
{
  hMenu = CreatePopupMenu();
  if (!hMenu)
    return FALSE;

  MENUITEMINFO item;

  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = ID_TRAY_CLOSE;
  item.dwTypeData = (LPSTR)"Close";

  if (!InsertMenuItemA(hMenu, ID_TRAY_CLOSE - ID_TRAY_FIRST, TRUE, &item))
    return FALSE;

  ZeroMemory(&item, sizeof(item));
  item.cbSize = sizeof(item);
  item.fMask = MIIM_ID | MIIM_STRING;
  item.wID = ID_TRAY_MINE;
  if (mining())
    item.dwTypeData = (LPSTR)TRAY_TEXT_STOP;
  else
    item.dwTypeData = (LPSTR)TRAY_TEXT_START;

  if (!InsertMenuItemA(hMenu, ID_TRAY_MINE - ID_TRAY_FIRST, TRUE, &item))
    return FALSE;

  return TRUE;
}

BOOL InitTray() {
  WNDCLASSEXA wndCls;

  ZeroMemory(&wndCls, sizeof(wndCls));
  wndCls.cbSize = sizeof(wndCls);
  wndCls.lpfnWndProc = trayProc;
  wndCls.hInstance = GetModuleHandleA(NULL);
  wndCls.lpszClassName = WC_TRAY_CLASS_NAME;
  if (!RegisterClassExA(&wndCls))
    return FALSE;

  if (!initMenu())
    return FALSE;

  HWND hWnd = CreateWindowExA(0, WC_TRAY_CLASS_NAME, NULL, 0, 0, 0, 0, 0, NULL, hMenu, NULL, NULL);
  if (hWnd == NULL)
    return FALSE;

  HICON icon;
  if (!ExtractIconExA(ICO_PATH, 0, NULL, &icon, 1))
    return FALSE;

  nid.cbSize = sizeof(nid);
  nid.hWnd = hWnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
  nid.hIcon = icon;
  memcpy(nid.szTip, TRAY_TOOLTIP, sizeof(TRAY_TOOLTIP));

  Shell_NotifyIconA(NIM_ADD, &nid);
  UpdateWindow(hWnd);

  return TRUE;
}

void CloseTray()
{
  Shell_NotifyIconA(NIM_DELETE, &nid);
  if (nid.hIcon != 0)
    DestroyIcon(nid.hIcon);

  if (hMenu != 0)
    DestroyMenu(hMenu);

  UnregisterClass(WC_TRAY_CLASS_NAME, GetModuleHandleA(NULL));
}