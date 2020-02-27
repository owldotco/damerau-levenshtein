# Damerau-Levenshtein distance

A direct, naïve, and not quite complete port of the Levenshtein
algorithm from [natural](https://github.com/NaturalNode/natural)
to an N-API/C++ extension.

Currently it is a very straightforward translation, not at all
optimized for C++. On average, it tests as being about 2.3x faster
than the JS implementation. Optimization to get rid of all the
unnecessary heap operations should improve this considerably.