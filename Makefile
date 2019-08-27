all: ut_tamarin

test: ut_tamarin lemmas
	./ut_tamarin ./test/test_protocol.spthy --lemma_file=./test/lemmas.ut --timeout=1 -c

lemmas: ./test/test_protocol.spthy
	./ut_tamarin ./test/test_protocol.spthy ./test/lemmas.ut

ut_tamarin: ut_tamarin.cc
	g++ ut_tamarin.cc -o ut_tamarin

clean:
	rm ./ut_tamarin
	rm ./test/lemmas.ut
