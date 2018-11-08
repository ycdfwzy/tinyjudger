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
	tradiJudger \
	executor \
	$(CHECKER_EXE)

all: $(EXE)

% : %.cpp checkers/testlib.h
	g++ $< -o $@

tradiJudger: tradiJudger.cpp configs.o tradiJudger.h
	g++ tradiJudger.cpp configs.o -o tradiJudger
executor: executor.cpp parseArgs.o configs.o
	g++ executor.cpp parseArgs.o configs.o -o executor
parseArgs.o: parseArgs.cpp parseArgs.h configs.h
	g++ -c parseArgs.cpp -o parseArgs.o
configs.o: configs.cpp configs.h
	g++ -c configs.cpp -o configs.o
clean:
	rm *.o $(EXE)