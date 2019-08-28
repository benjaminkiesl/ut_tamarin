all: ut_tamarin

ut_tamarin: ut_tamarin.cc
	g++ ut_tamarin.cc -std=c++14 -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
