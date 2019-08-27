all: ut_tamarin

ut_tamarin: ut_tamarin.cc
	g++ ut_tamarin.cc -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
