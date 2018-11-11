#ifndef tinyjudger_tradiJudger_H
#define tinyjudger_tradiJudger_H

#include <string>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstdarg>
#include <sstream>
#include <sys/types.h>
#include "configs.h"

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;

int execute(const char* cmd){
	cout << "to Execute : \"" << string(cmd) << "\"" << endl;
	return system(cmd);
}

string Pathjoin(const string& path, const string& file){
	if (path.length() == 0 || (path.length() > 0 && path[path.length()-1] == '/')){
		return path+file;
	} else
	{
		return path+"/"+file;
	}
}

RunResult runExecutor(
	const RunConfig& rc){
	ostringstream oss;
	oss << "./executor" <<
		" --tl=" << rc.lim.time <<
		" --rtl=" << rc.lim.realTime <<
		" --ml=" << rc.lim.memory <<
		" --ol=" << rc.lim.output <<
		" --sl=" << rc.lim.stack <<
		" --in=" << rc.inputFileName <<
		" --out=" << rc.outputFileName <<
		" --err=" << rc.errorFileName <<
		" --res=" << rc.resultFileName <<
		" --work-path=" << rc.path <<
		" --type=" << rc.Lang;
	if (!rc.safe)
		oss << " --unsafe";
	for (int i = 0; i < rc.argArr.size(); ++i){
		oss << " " << rc.argArr[i];
	}
	if (execute(oss.str().c_str()) != 0){
		return RunResult::fail_execute();
	}
	return RunResult::load(rc.resultFileName.c_str());
}

CompileResult runCompiler(
	const char* compilepath,
	...){
	RunConfig compileConfig;
	compileConfig.lim = compileLimit;
	compileConfig.argArr.clear();
	compileConfig.resultFileName = Pathjoin(string(compilepath), string("compile_res.txt"));
	compileConfig.outputFileName = "stderr";
	compileConfig.errorFileName = Pathjoin(string(compilepath), string("compile_err.txt"));
	compileConfig.inputFileName = "stdin";
	compileConfig.Lang = "compiler";
	compileConfig.path = string(compilepath);
	compileConfig.safe = true;

	// cout << "before parse args" << endl;
	va_list ap;
	va_start(ap, compilepath);
	for (const char* args = va_arg(ap, const char *); args; args = va_arg(ap, const char *)){
		// cout << string(args) << endl;
		compileConfig.argArr.push_back(string(args));
	}
	va_end(ap);

	// cout << "after parse args" << endl;
	
	RunResult res = runExecutor(compileConfig);
	CompileResult ret(res.jr, res.time, res.memory,
					(res.jr == Accept) && (res.ec == NoError));
	if (!ret.success){
		if (res.ec != NoError){
			ret.getInfo(compileConfig.errorFileName.c_str());
		} else
		if (res.jr == JudgementFailed){
			ret.info = "No Comment!";
		} else
		{
			ret.info = "Compile Failed: " + JudgeResult2string(res.jr);
		}
	}
	return ret;
}

RunResult runProgram(
	const char* runningpath,
	const char* resfile,
	const char* infile,
	const char* outfile,
	const char* errfile,
	const char* language,
	...){

	RunConfig runningConfig;
	runningConfig.lim = runningLimit;
	runningConfig.argArr.clear();
	runningConfig.resultFileName = Pathjoin(string(runningpath), string(resfile));
	runningConfig.inputFileName = Pathjoin(string(runningpath), string(infile));
	runningConfig.outputFileName = Pathjoin(string(runningpath), string(outfile));
	runningConfig.errorFileName = Pathjoin(string(runningpath), string(errfile));
	runningConfig.Lang = language;
	runningConfig.path = string(runningpath);
	runningConfig.safe = true;

	va_list ap;
	va_start(ap, language);
	for (const char* args = va_arg(ap, const char *); args; args = va_arg(ap, const char *)){
		// cout << string(args) << endl;
		runningConfig.argArr.push_back(string(args));
	}
	va_end(ap);

	return runExecutor(runningConfig);
}

CheckerResult runChecker(
	const char* path,
	const char* checker,
	const char* resfile,
	const char* errfile,
	const char* inputfile,
	const char* outputfile,
	const char* ansfile,
	const char* rptfile){

	RunConfig checkerConfig;
	checkerConfig.lim = checkerLimit;
	checkerConfig.resultFileName = Pathjoin(string(path), string(resfile));
	checkerConfig.inputFileName = "stdin";
	checkerConfig.outputFileName = "stdout";
	checkerConfig.errorFileName = Pathjoin(string(path), string(errfile));
	checkerConfig.Lang = "checker";
	checkerConfig.path = string(path);
	checkerConfig.safe = true;

	checkerConfig.argArr.clear();
	checkerConfig.argArr.push_back(string(checker));
	checkerConfig.argArr.push_back(string(inputfile));
	checkerConfig.argArr.push_back(string(outputfile));
	checkerConfig.argArr.push_back(string(ansfile));
	checkerConfig.argArr.push_back(string(rptfile));

	RunResult res = runExecutor(checkerConfig);

	cout << "checker res = " << JudgeResult2string(res.jr) << endl;

	CheckerResult ret(res.jr, res.time, res.memory,
					(res.jr == Accept) && (res.ec == NoError));
	if (!ret.success){
		if (res.ec != NoError){
			ret.getInfo(rptfile);
		} else
		if (res.jr == JudgementFailed){
			ret.info = "No Comment!";
		} else
		{
			ret.info = "Check Failed: " + JudgeResult2string(res.jr);
		}
	}
	return ret;
}

#endif