CHECKER_EXE = \
	checkers/acmp \
	checkers/caseicmp \
	checkers/casencmp \
	checkers/casewcmp \
	checkers/dcmp \
	checkers/fcmp \
	checkers/hcmp \
	checkers/icmp \
	checkers/lcmp \
	checkers/ncmp \
	checkers/pointscmp \
	checkers/rcmp \
	checkers/rcmp4 \
	checkers/rcmp6 \
	checkers/rcmp9 \
	checkers/rncmp \
	checkers/uncmp \
	checkers/wcmp \
	checkers/yesno

EXE = \
	scriptJudger \
	tradiJudger \
	executor \
	$(CHECKER_EXE)

all: $(EXE)

% : %.cpp checkers/testlib.h
	g++ $< -o $@

scriptJudger: scriptJudger.cpp utils/configs.o utils/parseArgs.o utils/scriptJudger.h
	g++ scriptJudger.cpp utils/configs.o utils/parseArgs.o -o scriptJudger
tradiJudger: tradiJudger.cpp utils/configs.o utils/parseArgs.o utils/tradiJudger.h
	g++ tradiJudger.cpp utils/configs.o utils/parseArgs.o -o tradiJudger
executor: utils/executor.cpp utils/parseArgs.o utils/configs.o
	g++ utils/executor.cpp utils/parseArgs.o utils/configs.o -o executor
utils/parseArgs.o: utils/parseArgs.cpp utils/parseArgs.h utils/configs.h
	g++ -c utils/parseArgs.cpp -o utils/parseArgs.o
utils/configs.o: utils/configs.cpp utils/configs.h
	g++ -c utils/configs.cpp -o utils/configs.o
clean:
	rm *.o scriptJudger tradiJudger executor
clean_all:
	rm *.o $(EXE)