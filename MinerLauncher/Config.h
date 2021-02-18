#pragma once

#include "framework.h"

#define MAX_ITEMS 256
#define MAX_COMMAND 1024

class Config
{
  const char* path = NULL;

public:
  int game_count = 0;
  int miner_count = 0;
  char gaming_profile[MAX_COMMAND];
  char mining_profile[MAX_COMMAND];
  char* games[MAX_ITEMS];
  char* miners[MAX_ITEMS];

  Config(const char* path);
  ~Config() {};

private:
  int load(const char* section, const char key[8], char** dst);
};