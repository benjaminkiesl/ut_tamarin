# UT Tamarin

UT Tamarin is a simple tool that assists you with the development and maintenance of models for the automated-reasoning tool [Tamarin](https://tamarin-prover.github.io), which is used for verifying security protocols. In case you are working with big Tamarin models that contain lots of lemmas, UT Tamarin might be exactly what you were looking for. The main features are as follows:

* Run Tamarin on a list of lemmas with a specified timeout and print the results.
* Specify dedicated heuristics that are tailored to specific lemmas.
* Run Tamarin on a lemma by trying all predefined heuristics ("hammer" at the lemma).

Lemma lists and dedicated heuristics can be specified in a JSON configuration file as explained below. The following screenshot shows a typical output of UT Tamarin when run on a couple of lemmas:

![UT Tamarin Output](images/screenshot_utt.png)

## Getting Started

### Prerequisites

To build UT Tamarin, you need [GNU Make](https://www.gnu.org/software/make/) and [CMake](https://cmake.org/).

To run UT Tamarin, you need an installed version of the [GNU M4 macro processor](https://www.gnu.org/software/m4/) and of Tamarin (find installation instructions [here](https://tamarin-prover.github.io/manual/book/002_installation.html)). UT Tamarin assumes that Tamarin is in your PATH, meaning that it can be executed by calling `tamarin-prover`. Since Tamarin doesn't run on Windows, neither does UT Tamarin.

### Installation

The easiest way to build UT Tamarin is to just execute the script 'build.sh' in the main directory. After this, the executable 'uttamarin' will be located at 'build/bin/uttamarin'.

The build.sh script simply creates a directory 'build' within the main directory and then executes 'cmake ..' followed by 'make' from within that build directory. If you don't want to use Make, and are familiar with CMake, you can also build the project for any platform you want using the CMake file CMakeLists.txt.

### Running UT Tamarin

To run UT Tamarin, just execute the following command from the shell (assuming that you are in the directory that contains the executable 'uttamarin'): 

`./uttamarin INPUT_TAMARIN_FILE --config_file=CONFIG_FILE`

* `INPUT_TAMARIN_FILE` is the path to a Tamarin theory file (i.e., a .spthy file)
* `CONFIG_FILE` is the path to a JSON file that contains configuration options for UT Tamarin such as the list of lemmas that should be proved or dedicated custom heuristics. See below for details.

For example, if your Tamarin theory file is the file test_protocol.spthy (located in the directory from which you call UT Tamarin) and your JSON configuration file is the file utt_config.json, then you would call UT Tamarin as follows:

`./uttamarin test_protocol.spthy --config_file=utt_config.json`

Further arguments, such as a dedicated timeout for Tamarin (default is ten minutes) can be passed to UT Tamarin. For details call `./uttamarin --help`.

Finally, make sure you have write access to the directory from which you call uttamarin because uttamarin produces temporary files in that directory.

### Specifying Configuration Options of UT Tamarin

UT Tamarin allows you to specify configuration options via a JSON file that you then pass to UT Tamarin as explained above. Such a JSON file can contain:

* An allow list of lemmas: If you specify an allow list, then only those lemmas from the Tamarin theory file are proved that are also in the allow list. 
* A deny list of lemmas: If you specify a deny list, then all lemmas from the deny list are ignored when running UT Tamarin.
* Global fact annotations: These annotations list fact symbols within your Tamarin theory file that should have a higher or lower priority in the heuristics. UT Tamarin enforces these priority declarations by adding either the prefix `F_` (higher priority) or `L_` (lower priority) to a fact symbol before calling Tamarin (no worries, the original spthy file is not changed).
* Local fact annotations: They work like global fact annotations with the only difference that they can be applied to specific lemmas (instead of all lemmas in the theory file). Local fact annotations overrule global fact annotations. Moreover, local fact annotations can assign a "neutral" priority (this can be useful when you want to remove a global fact annotation for a specific lemma).

The following is a sample JSON configuration for UT Tamarin that should be self-explanatory:

```
{
	"lemma_allow_list": [],
	"lemma_deny_list": [ 
		"first_denied_lemma",
		"second_denied_lemma"
	],
	"global_annotations": {
		"important_facts": ["NewFact", "OtherFact"],
		"unimportant_facts": ["SomeUnimportantFact"],
		"neutral_facts": []
	},
	"lemma_annotations": [
		{	
			"lemma_name": "some_statement",
			"neutral_facts": ["OtherFact"]
		},
		{	
			"lemma_name": "other_statement",
			"unimportant_facts": ["AFact", "BFact"]
		}
	]
}
```

## Built With

* [CLI11](https://github.com/CLIUtils/CLI11) - Command line parser for C++11.
* [JSON for Modern C++](https://github.com/nlohmann/json) - An excellent JSON library for C++.
