#include "MinerLauncher.h"
#include "Tray.h"
#include "Config.h"

const char* CONFIG = "MinerLauncher.ini";

Config* config;
Tray* tray;
HANDLE hGame;
HANDLE* hMiners;

BOOL istartswith(const char* str, const char* prefix)
{
  size_t strLen = strlen(str);
  size_t prefixLen = strlen(prefix);

  if (prefixLen > strLen)
    return FALSE;

  return _strnicmp(str, prefix, prefixLen) == 0;
}

BOOL mining()
{
  if (!hMiners)
    return FALSE;

  for (int i = 0; i < config->miner_count; i++)
    if (hMiners[i])
      return TRUE;

  return FALSE;
}

void CloseMiner()
{
  // close miners
  for (int i = 0; i < config->miner_count; i++)
  {
    if (!hMiners[i])
      continue;

    TerminateProcess(hMiners[i], 0);
    CloseHandle(hMiners[i]);
    hMiners[i] = NULL;
  }

  // switch profile to gaming
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  if (CreateProcessA(NULL, (LPSTR)config->gaming_profile, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }

  tray->updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_START);
}

void StartMiner()
{
  // switch profile to mining
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  if (CreateProcessA(NULL, (LPSTR)config->mining_profile, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }

  // start miners
  for (int i = 0; i < config->game_count; i++)
  {
    if (hMiners[i])
      continue;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    if (CreateProcessA(NULL, (LPSTR)config->miners[i], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
      CloseHandle(pi.hThread);
      hMiners[i] = pi.hProcess;
    }
  }  

  tray->updateMenuItem(ID_TRAY_MINE, TRAY_TEXT_STOP);
}

HANDLE isGame(HWND hWnd)
{
  if (!hWnd)
    return NULL;

  DWORD procId = 0;
  GetWindowThreadProcessId(hWnd, &procId);
  if (!procId)
    return NULL;

  HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, procId);
  if (!hProc)
    return NULL;

  char path[MAX_PATH];
  if (GetModuleFileNameExA(hProc, NULL, path, MAX_PATH))
  {
    for (int i=0; i<config->game_count; i++)
      if (istartswith(path, config->games[i]))
        return hProc;
  }

  CloseHandle(hProc);
  return NULL;
}

void CALLBACK HandleForegroundWindowChange(HWINEVENTHOOK hHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
  if (hGame)
    return; 
  
   hGame = isGame(hWnd);
   if (!hGame)
     return; 
  
  CloseMiner();
} 

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  try
  {
    config = new Config(CONFIG);
    hMiners = (HANDLE*)calloc(config->miner_count, sizeof(HANDLE));
    if (!hMiners)
      return 1;

    tray = new Tray(LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1)));
    StartMiner();

    HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, HandleForegroundWindowChange, 0, 0, WINEVENT_SKIPOWNPROCESS | WINEVENT_OUTOFCONTEXT);
    if (!hook)
      return 1;    

    UINT_PTR timer = SetTimer(NULL, NULL, 500, NULL);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      if (msg.message == WM_QUIT)
        break;

      TranslateMessage(&msg);
      DispatchMessageA(&msg);

      if (hGame)
      {
        if (WaitForSingleObject(hGame, 0) != WAIT_TIMEOUT)
        {
          CloseHandle(hGame);
          hGame = NULL;
          StartMiner();
        }
      }
    }
      
    KillTimer(NULL, timer);
    UnhookWinEvent(hook);
    delete config;
    delete tray;
    return 0;
  }
  catch (const char* msg)
  {
    MessageBoxA(NULL, msg, "Error", MB_ICONEXCLAMATION);
    return 1;
  }
}



