tradiJudger: tradiJudger.cpp configs.o tradiJudger.h
	g++ tradiJudger.cpp configs.o -o tradiJudger
executor: executor.cpp parseArgs.o configs.o
	g++ executor.cpp parseArgs.o configs.o -o executor
parseArgs.o: parseArgs.cpp parseArgs.h configs.h
	g++ -c parseArgs.cpp -o parseArgs.o
configs.o: configs.cpp configs.h
	g++ -c configs.cpp -o configs.o
clean:
	rm *.o executor tradiJudger