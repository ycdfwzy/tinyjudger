#ifndef tinyjudger_parseArgs_H
#define tinyjudger_parseArgs_H
#include <argp.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "configs.h"

void parse_args(int argc, char **argv, RunConfig &runConfig);

#endif