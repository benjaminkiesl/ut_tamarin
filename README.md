# UT Tamarin

UT Tamarin is a simple tool that assists you with the development and maintenance of models for the automated-reasoning tool [Tamarin](https://tamarin-prover.github.io), which is used for verifying security protocols. In case you are working with big Tamarin models that contain lots of lemmas, UT Tamarin might be exactly what you were looking for. The main features are as follows:

* Run Tamarin on a list of lemmas with a specified timeout and print the results.
* Specify dedicated heuristics that are tailored to specific lemmas.
* Run Tamarin on a lemma by trying all predefined heuristics ("hammer" at the lemma).

Lemma lists and dedicated heuristics can be specified in a JSON config file as explained below. The following screenshot shows a typical output of UT Tamarin when run on a couple of lemmas:

![UT Tamarin Output](images/screenshot_utt.png)

## Getting Started

### Prerequisites

UT Tamarin requires an installed version of Tamarin. By default, UT Tamarin assumes that Tamarin is in your PATH, meaning that it can be executed by calling `tamarin-prover`. If this is not the case, then you can specify the path to an executable of Tamarin.

### Installation

To build UT Tamarin, just run the `make` command from within the main directory. This then builds the executable `ut_tamarin`.

### Running UT Tamarin

To run UT Tamarin, just execute the following command from the shell: 

`./ut_tamarin INPUT_TAMARIN_FILE --config_file=CONFIG_FILE`

* `INPUT_TAMARIN_FILE` is the path to a Tamarin theory file (i.e., a .spthy file)
* `CONFIG_FILE` is the path to a JSON file that contains configuration options for UT Tamarin such as the list of lemmas that should be proved or dedicated custom heuristics. See below for details.

For example, if your Tamarin theory file is the file test_protocol.spthy (located in the directory from which you call UT Tamarin) and your JSON config file is the file utt_config.json, then you would call UT Tamarin as follows:

`./ut_tamarin test_protocol.spthy --config_file=utt_config.json`

Further arguments, such as a dedicated timeout for Tamarin (default is ten minutes) can be passed to UT Tamarin. For details call `./ut_tamarin --help`.

Finally, make sure you have write access to the directory from which you call ut_tamarin because ut_tamarin produces temporary files in that directory.

### Specifying Configuration Options of UT Tamarin

UT Tamarin allows you to specify configuration options via a JSON file that you then pass to UT Tamarin is explained above. Such a JSON file can contain:

	* A white list of lemmas: If you specify a white list, then only those lemmas from the Tamarin theory file are proved that are also in the white list. 
	* A black list of lemmas: If you specify a blacklist, then all lemmas from the blacklist are ignored when running UT Tamarin.
	* Global fact annotations: These annotations list fact symbols within your Tamarin theory file that should have a higher or lower priority in the heuristics. Higher priority is achieved by adding the prefix `F_` to a fact symbol, lower priority is achieved by adding `L_`.
	* Local fact annotations: They work like global fact annotations with the only difference that they can be applied to specific lemmas (instead of all lemmas in the theory file). Local fact annotations overrule global fact annotations. Moreover, local facts can be declared as "neutral" (in case they have a high or low priority in the global fact annotations).

The following is a sample JSON config file for UT Tamarin that should be self-explanatory:

```
{
	"lemma_blacklist": [ 
		"black_listed_lemma"	
	],
	"lemma_whitelist": [],
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
