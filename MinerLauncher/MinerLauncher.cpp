#include "framework.h"

#define MAX_ITEMS 256

const char* CONFIG = ".\\MinerLauncher.ini";
int GAME_COUNT;
int MINER_COUNT;
char GAMING_PROFILE[1024];
char MINING_PROFILE[1024];
char* GAMES[MAX_ITEMS];
char* MINERS[MAX_ITEMS];

HANDLE hGame;
HANDLE *hMiners;

BOOL istartswith(const char* str, const char* prefix)
{
  size_t strLen = strlen(str);
  size_t prefixLen = strlen(prefix);

  if (prefixLen > strLen)
    return FALSE;

  return _strnicmp(str, prefix, prefixLen) == 0;
}

void CloseMiner()
{
  // close miners
  for (int i = 0; i < MINER_COUNT; i++)
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
  if (CreateProcessA(NULL, (LPSTR)GAMING_PROFILE, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
}

void StartMiner()
{
  // switch profile to mining
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  if (CreateProcessA(NULL, (LPSTR)MINING_PROFILE, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }

  // start miners
  for (int i = 0; i < MINER_COUNT; i++)
  {
    if (hMiners[i])
      continue;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    if (CreateProcessA(NULL, (LPSTR)MINERS[i], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
      CloseHandle(pi.hThread);
      hMiners[i] = pi.hProcess;
    }
  }  
}

int load(const char* section, const char* key, char** dst)
{
  char name[32];
  int count = 0;
 
  for (count; count < MAX_ITEMS; count++)
  {
    sprintf_s(name, "%s%d", key, count);
    char* value = (char*)malloc(1024);
    if (!value)
      return 0;

    if (GetPrivateProfileStringA(section, name, NULL, value, 1024, CONFIG))
      dst[count] = value;
    else
    {
      free(value);
      break;
    }
  }

  return count;
}

BOOL LoadConfig()
{  
  if (!GetPrivateProfileStringA("GAME", "profile", NULL, GAMING_PROFILE, sizeof(GAMING_PROFILE), CONFIG))
    return FALSE;
  
  if (!GetPrivateProfileStringA("MINE", "profile", NULL, MINING_PROFILE, sizeof(MINING_PROFILE), CONFIG))
    return FALSE;

  MINER_COUNT = load("MINE", "miner", MINERS);
  if (MINER_COUNT == 0)
    return FALSE;

  GAME_COUNT = load("GAME", "game", GAMES);
  if (GAME_COUNT == 0)
    return FALSE;

  hMiners = (HANDLE*)malloc(MINER_COUNT * sizeof(HANDLE));
  if (!hMiners)
    return FALSE;

  return TRUE;
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
    for (int i=0; i<GAME_COUNT; i++)
      if (istartswith(path, GAMES[i]))
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
  WaitForSingleObject(hGame, INFINITE);
  CloseHandle(hGame);
  hGame = NULL; 

  StartMiner();
} 

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  if (!LoadConfig())
    return 0;

  StartMiner();
  
  HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, HandleForegroundWindowChange, 0, 0, WINEVENT_SKIPOWNPROCESS | WINEVENT_OUTOFCONTEXT);
  if (!hook)
    return 1;

  // Main message loop:
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0));
  
  UnhookWinEvent(hook);
  return 0;
}



