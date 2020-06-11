all: ut_tamarin

debug: src/ut_tamarin.cc
	g++ src/ut_tamarin.cc -I . -I ./include/ -g -std=c++17 -pthread -o ut_tamarin

ut_tamarin: src/main.cc
	g++ src/main.cc src/app.cc src/terminator.cc -I . -I ./include/ -std=c++17 -pthread -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
