all: ut_tamarin

debug: src/ut_tamarin.cc
	g++ src/ut_tamarin.cc -I . -g -std=c++17 -pthread -o ut_tamarin

ut_tamarin: src/ut_tamarin.cc
	g++ src/ut_tamarin.cc -I . -std=c++17 -pthread -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
