all: uttamarin

debug: src/uttamarin.cc
	g++ src/main.cc src/utility.cc src/app.cc src/terminator.cc src/tamarin_config.cc -I . -I ./include/ -g -std=c++17 -pthread -o uttamarin

uttamarin: src/main.cc src/terminator.cc src/tamarin_config.cc
	g++ src/main.cc src/utility.cc src/app.cc src/terminator.cc src/tamarin_config.cc -I . -I ./include/ -std=c++17 -pthread -o uttamarin

clean:
	rm ./uttamarin
	rm ./test/lemmas.ut
