all: ut_tamarin

debug: ut_tamarin.cc
	g++ ut_tamarin.cc -g -std=c++17 -pthread -o ut_tamarin

ut_tamarin: ut_tamarin.cc
	g++ ut_tamarin.cc -std=c++17 -pthread -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
