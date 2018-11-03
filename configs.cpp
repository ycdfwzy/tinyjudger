#include "configs.h"

std::string JudgeResult2string(JudgeResult jr) {
	std::string ret;
	switch (jr){
		case Accept:
			ret = "Accept";
			break;
		case WrongAnswer:
			ret = "Wrong Answer";
			break;
		case RuntimeError:
			ret = "Runtime Error";
			break;
		case TimeLimitExceed:
			ret = "Time Limit Exceed";
			break;
		case MemoryLimitExceed:
			ret = "Memory Limit Exceed";
			break;
		case OutputLimitExceed:
			ret = "Output Limit Exceed";
			break;
		case DangerSystemCall:
			ret = "Danger System Call";
			break;
		case JudgementFailed:
			ret = "Judgement Failed";
			break;
		case CompileError:
			ret = "Compile Error";
			break;
		default:
			ret = "unknown";
	}
	return ret;
}
