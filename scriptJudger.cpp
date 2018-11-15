#include "utils/parseArgs.h"
#include "utils/scriptJudger.h"

using namespace std;

int main(int argc, char** argv){
	RunConfig runConfig;
	script_judger_parse_args(argc, argv, runConfig);
	runConfig.resultFileName = "result.txt";
	RunResult runResult = runExecutor(runConfig);
	cout << runResult.jr << " " << runResult.time << " " <<
			runResult.memory << " " << runResult.ec << endl;
	return 0;
}