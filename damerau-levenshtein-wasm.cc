#include <emscripten.h>

#include "./damerau-levenshtein.cc"

EMSCRIPTEN_KEEPALIVE
extern "C"
bool useUtf16() {
    return USE_UTF16;
}

EMSCRIPTEN_KEEPALIVE
extern "C"
void call_levenshteinDistance(
    const _char_type *source,
    const _char_type *target,
    const double insertion_cost,
    const double deletion_cost,
    const double substitution_cost,
    const double transposition_cost,
    const bool search,
    const bool restricted,
    const bool damerau,
    MinCostSubstringStruct *out)
{
    _string sourceStr(source), targetStr(target);

    Options options;
    options.insertion_cost = insertion_cost;
    options.deletion_cost = deletion_cost;
    options.substitution_cost = substitution_cost;
    options.transposition_cost = transposition_cost;
    options.search = search;
    options.restricted = restricted;
    options.damerau = damerau;

    MinCostSubstringStruct result = levenshteinDistance(sourceStr, targetStr, options);
    memcpy(out, &result, sizeof(MinCostSubstringStruct));
}

EMSCRIPTEN_KEEPALIVE
extern "C"
double get_MinCostSubstringStruct_distance(MinCostSubstringStruct *res) {
    return res->distance;
}

EMSCRIPTEN_KEEPALIVE
extern "C"
int32_t get_MinCostSubstringStruct_offset(MinCostSubstringStruct *res) {
    return res->offset;
}

EMSCRIPTEN_KEEPALIVE
extern "C"
const _char_type *get_MinCostSubstringStruct_substring(MinCostSubstringStruct *res) {
    return res->substring.c_str();
}

EMSCRIPTEN_KEEPALIVE
extern "C"
size_t sizeof_MinCostSubstringStruct() {
    return sizeof(MinCostSubstringStruct);
}
