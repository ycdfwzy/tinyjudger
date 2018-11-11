#include "parseArgs.h"
#include <argp.h>

char exec_argp_args_doc[] = "run program arg1, arg2, ...";
char exec_argp_doc[] = "A tool to run clients' program on server safely";
argp_option exec_argp_options[] = {
	{"tl"                 , 'T', "TIME_LIMIT"  , 0, "Set time limit (in second)"                            ,  1},
	{"rtl"                , 'R', "TIME_LIMIT"  , 0, "Set real time limit (in second)"                       ,  2},
	{"ml"                 , 'M', "MEMORY_LIMIT", 0, "Set memory limit (in mb)"                              ,  3},
	{"ol"                 , 'O', "OUTPUT_LIMIT", 0, "Set output limit (in mb)"                              ,  4},
	{"sl"                 , 'S', "STACK_LIMIT" , 0, "Set stack limit (in mb)"                               ,  5},
	{"in"                 , 'i', "IN"          , 0, "Set input file name"                                   ,  6},
	{"out"                , 'o', "OUT"         , 0, "Set output file name"                                  ,  7},
	{"err"                , 'e', "ERR"         , 0, "Set error file name"                                   ,  8},
	{"work-path"          , 'w', "WORK_PATH"   , 0, "Set the work path of the program"                      ,  9},
	{"type"               , 't', "TYPE"        , 0, "Set the program type (for some program such as python)", 10},
	{"res"                , 'r', "RESULT_FILE" , 0, "Set the file name for outputing the result            ", 10},
	{"unsafe"             , 501, 0             , 0, "Don't check dangerous syscalls"                        , 12},
	{0}
};
error_t exec_argp_parse_opt (int key, char *arg, struct argp_state *state) {
	RunConfig *config = (RunConfig*)state->input;
	char rp[PATH_MAX+1];

	switch (key) {
		case 'T':
			config->lim.time = atoi(arg);
			break;
		case 'R':
			config->lim.realTime = atoi(arg);
			break;
		case 'M':
			config->lim.memory = atoi(arg);
			break;
		case 'O':
			config->lim.output = atoi(arg);
			break;
		case 'S':
			config->lim.stack = atoi(arg);
			break;
		case 'i':
			config->inputFileName = arg;
			break;
		case 'o':
			config->outputFileName = arg;
			break;
		case 'e':
			config->errorFileName = arg;
			break;
		case 'w':
			if (realpath(arg, rp) == NULL){
				std::cout << "error in realpath; path = " << arg << std::endl;
				config->path = "";
			} else
				config->path = std::string(rp);
			if (config->path.empty()) {
				argp_usage(state);
			}
			break;
		case 'r':
			config->resultFileName = arg;
			break;
		case 't':
			config->Lang = arg;
			break;
		case 501:
			config->safe = false;
			break;
		case ARGP_KEY_ARG:
			config->argArr.push_back(arg);
			for (int i = state->next; i < state->argc; i++) {
				config->argArr.push_back(state->argv[i]);
			}
			state->next = state->argc;
			break;
		case ARGP_KEY_END:
			if (state->arg_num == 0) {
				argp_usage(state);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

argp exec_margs = {
	exec_argp_options,
	exec_argp_parse_opt,
	exec_argp_args_doc,
	exec_argp_doc
};

std::string getCurDir(){
	char cur[512];
	if (getcwd(cur, 512) == NULL){
		std::cout << "error in getCurDir. errno = " << errno << std::endl;
		return std::string("");
	}
	return std::string(cur);
}

void exec_parse_args(int argc, char **argv, RunConfig &runConfig){
	runConfig.resultFileName = "stdout";
	runConfig.inputFileName = "stdin";
	runConfig.outputFileName = "stdout";
	runConfig.errorFileName = "stderr";
	runConfig.Lang = "C";
	runConfig.path = getCurDir();
	runConfig.lim = defaultLimit;
	runConfig.safe = true;

	// Todo: parse arguments
	argp_parse(&exec_margs, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &runConfig);

	if (runConfig.lim.realTime == -1)
		runConfig.lim.realTime = runConfig.lim.time+2;

	runConfig.lim.stack = std::min(runConfig.lim.stack, runConfig.lim.memory);

	if (runConfig.path.empty()){
		runConfig.path = getCurDir();
		if (runConfig.path.empty()){
			std::cout << "JudgementFailed for no path!" << std::endl;
			exit(JudgementFailed);
		}
	}
}

char judger_argp_args_doc[] = "run program arg1, arg2, ...";
char judger_argp_doc[] = "A tool to run judger";
argp_option judger_argp_options[] = {
	{"tl"                 , 'T', "TIME_LIMIT"  , 0, "Set time limit (in second)"                            ,  1},
	{"ml"                 , 'M', "MEMORY_LIMIT", 0, "Set memory limit (in mb)"                              ,  2},
	{"ol"                 , 'O', "OUTPUT_LIMIT", 0, "Set output limit (in mb)"                              ,  3},
	{"in-pre"             , 'p', "INPRE"       , 0, "Set input file prefix name"                            ,  4},
	{"in-suf"             , 's', "INSUF"       , 0, "Set input file suffix name"                            ,  5},
	{"out-pre"            , 'P', "OUTPRE"      , 0, "Set output file prefix name"                           ,  6},
	{"out-suf"            , 'S', "OUTSUF"      , 0, "Set output file suffix name"                           ,  7},
	{"Lang"               , 'L', "Language"    , 0, "Set the Language"                                      ,  8},
	{"data-dir"           , 'd', "DATA_DIR"    , 0, "Set the data directory"                                ,  9},
	{"checker"            , 'c', "CHECKER"     , 0, "Set the program type (for some program such as python)", 10},
	{"n-tests"            , 'n', "NTESTS"      , 0, "Set the number of tests"                               , 11},
	{"source-name"        , 'f', "SOURCE_FILE" , 0, "Set the source file name"                              , 12},
	{"source-dir"         , 'D', "SOURCE_DIR"  , 0, "Set the source file directory"                         , 13},
	{0}
};

error_t judger_argp_parse_opt (int key, char *arg, struct argp_state *state){
	JudgerConfig *config = (JudgerConfig*)state->input;

	switch (key){
		case 'T':
			config->time = atoi(arg);
			break;
		case 'M':
			config->memory = atoi(arg);
			break;
		case 'O':
			config->output = atoi(arg);
			break;
		case 'p':
			config->inputPre = arg;
			break;
		case 's':
			config->inputSuf = arg;
			break;
		case 'P':
			config->outputPre = arg;
			break;
		case 'S':
			config->outputSuf = arg;
			break;
		case 'L':
			config->Lang = arg;
			break;
		case 'd':
			config->dataDir = arg;
			break;
		case 'c':
			config->checker = arg;
			break;
		case 'n':
			config->ntests = atoi(arg);
			break;
		case 'f':
			config->source = arg;
			break;
		case 'D':
			config->sourceDir = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

argp judger_margs = {
	judger_argp_options,
	judger_argp_parse_opt,
	judger_argp_args_doc,
	judger_argp_doc
};

void judger_parse_args(int argc ,char **argv, JudgerConfig &judgerConfig){
	judgerConfig.time = 1;
	judgerConfig.memory = 128;
	judgerConfig.output = 64;
	judgerConfig.inputPre = "test";
	judgerConfig.inputSuf = "in";
	judgerConfig.outputPre = "test";
	judgerConfig.outputSuf = "ans";
	judgerConfig.checker = "ncmp";
	judgerConfig.Lang = "C";
	judgerConfig.dataDir = "/tmp";
	judgerConfig.ntests = 10;

	argp_parse(&judger_margs, argc, argv, ARGP_NO_ARGS | ARGP_IN_ORDER, 0, &judgerConfig);
}