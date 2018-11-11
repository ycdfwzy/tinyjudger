#include "tradiJudger.h"
#include "parseArgs.h"

using namespace std;
const string checker_dir = "/home/ycdfwzy/myworkspace/tinyjudger/checkers";
const string resSuf = ".res";
const string errSuf = ".err";
const string rptSuf = ".rpt";

JudgerResult main_test(const JudgerConfig& judgerConfig){
	cout << "time=" << judgerConfig.time << endl;
	cout << "memory=" << judgerConfig.memory << endl;
	cout << "output=" << judgerConfig.output << endl;
	cout << "Language=" << judgerConfig.Lang << endl;
	cout << "inputpre=" << judgerConfig.inputPre << endl;
	cout << "inputsuf=" << judgerConfig.inputSuf << endl;
	cout << "outputpre=" << judgerConfig.outputPre << endl;
	cout << "outputsuf=" << judgerConfig.outputSuf << endl;
	cout << "datadir=" << judgerConfig.dataDir << endl;
	cout << "checker=" << judgerConfig.checker << endl;
	cout << "ntests=" << judgerConfig.ntests << endl;
	cout << "sourcename=" << judgerConfig.source << endl;
	cout << "sourcedir=" << judgerConfig.sourceDir << endl;

	JudgerResult judgerResult("Accept", 0, 0, "OK");
	if (judgerConfig.Lang == "C++"){
		CompileResult cr = runCompiler(judgerConfig.sourceDir.c_str(),
										"/usr/bin/g++", 
										"-x", "c++",
										Pathjoin(judgerConfig.sourceDir, judgerConfig.source+string(".code")).c_str(),
										"-o",
										Pathjoin(judgerConfig.sourceDir, judgerConfig.source).c_str(),
										NULL);
		if (cr.success){
			for (int i = 0; i < judgerConfig.ntests; ++i) {
				ostringstream oss;
				oss << i << '.';
				string inputfile = judgerConfig.inputPre+oss.str()+judgerConfig.inputSuf;
				string outputfile = judgerConfig.outputPre+string(".")+judgerConfig.outputSuf;
				string ansfile = judgerConfig.outputPre+oss.str()+judgerConfig.outputSuf;
				string reportfile = judgerConfig.outputPre+rptSuf;
				RunResult rr = runProgram(judgerConfig.dataDir.c_str(),
										(judgerConfig.outputPre+resSuf).c_str(),
										inputfile.c_str(),
										outputfile.c_str(),
										(judgerConfig.outputPre+errSuf).c_str(),
										judgerConfig.Lang.c_str(),
										Pathjoin(judgerConfig.sourceDir, judgerConfig.source).c_str(),
										NULL);

				judgerResult.time = max(judgerResult.time, rr.time);
				judgerResult.memory = max(judgerResult.memory, rr.memory);

				if (rr.jr == Accept && rr.ec == NoError){
					CheckerResult cr = runChecker(judgerConfig.dataDir.c_str(),
													Pathjoin(checker_dir, judgerConfig.checker).c_str(),
													(judgerConfig.outputPre+resSuf).c_str(),
													(judgerConfig.outputPre+errSuf).c_str(),
													Pathjoin(judgerConfig.dataDir, inputfile).c_str(),
													Pathjoin(judgerConfig.dataDir, outputfile).c_str(),
													Pathjoin(judgerConfig.dataDir, ansfile).c_str(),
													Pathjoin(judgerConfig.dataDir, reportfile).c_str());

					if (!cr.success){
						judgerResult.result = "Wrong Answer";
						judgerResult.info = cr.info;
						break;
					}

				} else
				{
					judgerResult.result = JudgeResult2string(rr.jr);
					judgerResult.info = string("Failed at test case ")+oss.str();
					break;
				}
			}
		} else
		{
			judgerResult.result = "Compile Error";
			judgerResult.time = cr.time;
			judgerResult.memory = cr.memory;
			judgerResult.info = cr.info;
		}
	}
	return judgerResult;
}

int main(int argc, char** argv){
	JudgerConfig judgerConfig;
	judger_parse_args(argc, argv, judgerConfig);
	JudgerResult judgerResult = main_test(judgerConfig);
	cout << "Result:" << endl;
	cout << judgerResult.result << endl;
	cout << judgerResult.info << endl;
	cout << "time=" << judgerResult.time << "ms" << endl;
	cout << "memory=" << judgerResult.memory << "kb" << endl;
	return 0;
}