# Damerau-Levenshtein distance

A direct port of the Levenshtein/Damerau-Levenshtein
algorithm from [natural](https://github.com/NaturalNode/natural)
to an N-API extension and wasm binary. Implemented in C++.

In NodeJS, it uses the N-API implementation, and in the browser,
it uses the wasm implementation. In tests on production
data on our workload, it is ~15x faster than the original JS; on
smaller strings as used in the automated tests, the speedup is ~27x.

The implementation from [natural](https://github.com/NaturalNode/natural)
is included for reference.

Correctness is based on the assumption that `natural` is correct. :-)
Compliance is currently verified by running distance checks on several
thousand random sample strings and verifying that the N-API generates
identical results.
