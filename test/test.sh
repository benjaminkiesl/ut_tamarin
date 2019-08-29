#!/usr/bin/env bash
# Run this file from its parent directory
./ut_tamarin ./test/test_protocol.spthy --timeout=1 -c
./ut_tamarin ./test/test_protocol.spthy -g --output_file=./test/lemmas.ut
