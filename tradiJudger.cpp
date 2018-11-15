#include "tradiJudger.h"
#include "parseArgs.h"
#include <fstream>

using namespace std;
const string checker_dir = "/home/ycdfwzy/myworkspace/tinyjudger/checkers";
const string resSuf = ".res";
const string errSuf = ".err";
const string rptSuf = ".rpt";

JudgerResult run_C_CPP(const JudgerConfig& judgerConfig){
	JudgerResult judgerResult("Accept", 0, 0, "OK");

	CompileResult cr(Accept);
	if (judgerConfig.Lang == "C++")
		cr = runCompiler(judgerConfig.sourceDir.c_str(),
						"/usr/bin/g++", 
						"-O2", "-lm", "-x", "c++",
						Pathjoin(judgerConfig.sourceDir, judgerConfig.source+string(".code")).c_str(),
						"-o",
						Pathjoin(judgerConfig.sourceDir, judgerConfig.source).c_str(),
						NULL);
	else
		cr = runCompiler(judgerConfig.sourceDir.c_str(),
						"/usr/bin/gcc", 
						"-O2", "-lm", "-x", "c",
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
					oss.clear();
					oss.str("");
					oss << "TESTCASE #" << i << ": ";
					judgerResult.info = oss.str()+cr.info;
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
	return judgerResult;
}

string PythonComplie(const JudgerConfig& judgerConfig){
	string source = Pathjoin(judgerConfig.sourceDir, judgerConfig.source+string(".code"));
	string target = Pathjoin(judgerConfig.sourceDir, judgerConfig.source);
	string code = 
		"\"import sys\nimport py_compile\ntry:\n    py_compile.compile('"
		+ source + "', '" + target +
		"', doraise=True)\nexcept Exception as e:\n    print(e)\n    sys.exit(1)\nsys.exit(0)\n\"";
	return code;
}

JudgerResult run_Python(const JudgerConfig& judgerConfig){
	JudgerResult judgerResult("Accept", 0, 0, "OK");

	// CompileResult cr = runCompiler(judgerConfig.sourceDir.c_str(),
	// 								"/usr/bin/python3.6", 
	// 								"-E", "-O", "-B", "-c",
	// 								PythonComplie(judgerConfig).c_str(),
	// 								NULL);
	// cout << cr.success << endl;
	// cout << cr.info << endl;
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
								"/usr/bin/python3.6",
								Pathjoin(judgerConfig.sourceDir, judgerConfig.source+string(".code")).c_str(),
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
				oss.clear();
				oss.str("");
				oss << "TESTCASE #" << i << ": ";
				judgerResult.info = oss.str()+cr.info;
				break;
			}
		} else
		{
			judgerResult.result = JudgeResult2string(rr.jr);
			judgerResult.info = string("Failed at test case ")+oss.str();
			break;
		}
	}

	return judgerResult;
}

// JudgerResult run_Nodejs(const JudgerConfig& judgerConfig){
// 	JudgerResult judgerResult("Accept", 0, 0, "OK");

// 	for (int i = 0; i < judgerConfig.ntests; ++i){
// 		ostringstream oss;
// 		oss << i << '.';
// 		string inputfile = judgerConfig.inputPre+oss.str()+judgerConfig.inputSuf;
// 		string outputfile = judgerConfig.outputPre+string(".")+judgerConfig.outputSuf;
// 		string ansfile = judgerConfig.outputPre+oss.str()+judgerConfig.outputSuf;
// 		string reportfile = judgerConfig.outputPre+rptSuf;
// 		RunResult rr = runProgram(judgerConfig.dataDir.c_str(),
// 								(judgerConfig.outputPre+resSuf).c_str(),
// 								inputfile.c_str(),
// 								outputfile.c_str(),
// 								(judgerConfig.outputPre+errSuf).c_str(),
// 								judgerConfig.Lang.c_str(),
// 								"/usr/bin/nodejs",
// 								Pathjoin(judgerConfig.sourceDir, judgerConfig.source+string(".code")).c_str(),
// 								NULL);
// 	}

// 	return judgerResult;
// }

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

	
	if (judgerConfig.Lang == "C++" || judgerConfig.Lang == "C"){
		return run_C_CPP(judgerConfig);
	} else
	if (judgerConfig.Lang == "Python"){
		return run_Python(judgerConfig);
	}
	// else
	// if (judgerConfig.Lang == "Nodejs"){
	// 	return run_Nodejs(judgerConfig);
	// }
}

int main(int argc, char** argv){
	JudgerConfig judgerConfig;
	tradi_judger_parse_args(argc, argv, judgerConfig);
	JudgerResult judgerResult = main_test(judgerConfig);
	ofstream fout("result.json");
	fout << "{" << endl;
	fout << "    \"Result\" : \"" << judgerResult.result << "\"," << endl;
	fout << "    \"time\" : " << judgerResult.time << "," << endl;
	fout << "    \"memory\" : " << judgerResult.memory << "," << endl;
	fout << "    \"Info\" : \"" << judgerResult.info << "\"" << endl;
	fout << "}" << endl;
	fout.close();
	return 0;
}