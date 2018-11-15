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

void exec_parse_args(int argc, char **argv, RunConfig &runConfig);
void tradi_judger_parse_args(int argc ,char **argv, JudgerConfig &judgerConfig);
void script_judger_parse_args(int argc ,char **argv, RunConfig &runConfig);

#endif