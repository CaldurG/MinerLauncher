#include "Config.h"

Config::Config(const char* path)
{
  Config::path = path;

  if (!GetPrivateProfileStringA("GAME", "profile", NULL, gaming_profile, sizeof(gaming_profile), path))
    throw "[GAME] profile not found";

  if (!GetPrivateProfileStringA("MINE", "profile", NULL, mining_profile, sizeof(mining_profile), path))
    throw "[MINE] profile not found";

  miner_count = load("MINE", "miner", miners);
  if (miner_count == 0)
    throw "[MINE] miner0 not found";

  game_count = load("GAME", "game", games);
  if (game_count == 0)
    throw "[GAME] game0 not found"; 
}

int Config::load(const char* section, const char key[8], char** dst)
{
  char name[32];
  int count = 0;

  for (count; count < MAX_ITEMS; count++)
  {
    sprintf_s(name, "%s%d", key, count);
    char* value = (char*)malloc(MAX_COMMAND);
    if (!value)
      return 0;

    if (GetPrivateProfileStringA(section, name, NULL, value, MAX_COMMAND, path))
      dst[count] = value;
    else
    {
      free(value);
      break;
    }
  }

  return count;
}


