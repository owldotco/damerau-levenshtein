# Damerau-Levenshtein distance

[![Greenkeeper badge](https://badges.greenkeeper.io/haggholm/damerau-levenshtein.svg)](https://greenkeeper.io/)

A direct port of the Levenshtein/Damerau-Levenshtein
algorithm from [natural](https://github.com/NaturalNode/natural)
to an N-API/C++ extension.

In the browser, it will use the implementation copied from
[natural](https://github.com/NaturalNode/natural), with the
addition of an `offset` parameter in search results.

In NodeJS, it uses the N-API implementation. In tests on production
data on our workload, it is ~15x faster than the original JS; on
smaller strings as used in the automated tests, the speedup is ~27x.

Correctness is based on the assumption that `natural` is correct. :-)
Compliance is currently verified by running distance checks on several
thousand random sample strings and verifying that the N-API generates
identical results.