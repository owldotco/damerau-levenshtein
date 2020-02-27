# Damerau-Levenshtein distance

A direct, naïve, and not quite complete port of the Levenshtein
algorithm from [natural](https://github.com/NaturalNode/natural)
to an N-API/C++ extension.

In the browser, it will use the implementation copied from
[natural](https://github.com/NaturalNode/natural). In NodeJS,
it uses the N-API implementation, which tests as ~15x faster.