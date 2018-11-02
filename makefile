main: main.cpp parseArgs.o configs.h
	g++ main.cpp parseArgs.o -o main
parseArgs.o: parseArgs.cpp parseArgs.h configs.h
	g++ -c parseArgs.cpp -o parseArgs.o
clean:
	rm *.o main