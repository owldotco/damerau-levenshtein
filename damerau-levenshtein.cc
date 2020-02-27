#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define NAPI_VERSION 5
#include <node_api.h>

using namespace std;

// #define USE_UTF16 true
#ifdef USE_UTF16
typedef char16_t CharType;
#else
typedef char CharType;
#endif
typedef basic_string<CharType> String;

struct Options
{
    double insertion_cost = 1;
    double deletion_cost = 1;
    double substitution_cost = 1;
    double transposition_cost = 1;
    bool search = true;
    bool restricted = false;
    bool damerau = true;
};

struct DistanceEntry
{
    double cost = 0;
};

struct Coordinates
{
    size_t row;
    size_t column;
};

struct CoordinateMatrixEntry
{
    double cost = 0;
    shared_ptr<Coordinates> coordinates = shared_ptr<Coordinates>(0);
    shared_ptr<Coordinates> parentCell = shared_ptr<Coordinates>(0);
};

struct MinCostSubstringStruct
{
    String substring;
    double distance;
};

int _getMatchStart(
    const vector<vector<CoordinateMatrixEntry>> &distanceMatrix,
    const size_t matchEnd,
    const size_t sourceLength)
{
    size_t row = sourceLength;
    size_t column = matchEnd;
    size_t tmpRow;
    size_t tmpColumn;

    // match will be empty string
    if (matchEnd == 0)
    {
        return 0;
    }
    while (row > 1 && column > 1)
    {
        tmpRow = row;
        tmpColumn = column;
        row = distanceMatrix[tmpRow][tmpColumn].parentCell->row;
        column = distanceMatrix[tmpRow][tmpColumn].parentCell->column;
    }

    return column - 1;
}

struct MatrixEntryComparator
{
    bool operator()(const CoordinateMatrixEntry &a, const CoordinateMatrixEntry &b) const
    {
        return a.cost < b.cost;
    }
} matrixEntryComparator;

MinCostSubstringStruct getMinCostSubstring(
    const vector<vector<CoordinateMatrixEntry>> &distanceMatrix,
    const String &source,
    const String &target)
{
    size_t sourceLength = source.length();
    size_t targetLength = target.length();
    double minDistance = sourceLength + targetLength;
    int matchEnd = targetLength;
    for (size_t column = 0; column <= targetLength; column++)
    {
        if (minDistance > distanceMatrix[sourceLength][column].cost)
        {
            minDistance = distanceMatrix[sourceLength][column].cost;
            matchEnd = column;
        }
    }

    int matchStart = _getMatchStart(distanceMatrix, matchEnd, sourceLength);
    return {target.substr(matchStart, matchEnd), minDistance};
}

inline shared_ptr<Coordinates> nullCoords()
{
    return shared_ptr<Coordinates>(0);
}

inline shared_ptr<Coordinates> coordPtr(size_t row, size_t column)
{
    return shared_ptr<Coordinates>(new Coordinates({row, column}));
}

inline shared_ptr<CoordinateMatrixEntry> nullEntry()
{
    return shared_ptr<CoordinateMatrixEntry>(0);
}

double levenshteinDistance(
    const String &source,
    const String &target,
    const Options &options)
{
    bool isUnrestrictedDamerau = options.damerau && !options.restricted;
    bool isRestrictedDamerau = options.damerau && options.restricted;
    map<CharType, size_t> lastRowMap;

    auto sourceLength = source.length();
    auto targetLength = target.length();

    vector<vector<CoordinateMatrixEntry>> distanceMatrix(
        sourceLength + 1,
        vector<CoordinateMatrixEntry>(targetLength + 1, CoordinateMatrixEntry({0, nullCoords(), nullCoords()})));

    for (size_t row = 1; row <= sourceLength; row++)
    {
        distanceMatrix[row][0] = CoordinateMatrixEntry({distanceMatrix[row - 1][0].cost + options.deletion_cost, nullCoords(),
                                                        coordPtr(row - 1, 0)});
    }

    for (size_t column = 1; column <= targetLength; column++)
    {
        if (options.search)
        {
            distanceMatrix[0][column] = CoordinateMatrixEntry({0, nullCoords(), nullCoords()});
        }
        else
        {
            distanceMatrix[0][column] = {
                distanceMatrix[0][column - 1].cost + options.insertion_cost,
                nullCoords(),
                coordPtr(0, column - 1)};
        }
    }

    for (size_t row = 1; row <= sourceLength; row++)
    {
        int lastColMatch = -1;

        for (size_t column = 1; column <= targetLength; column++)
        {
            float costToInsert = distanceMatrix[row][column - 1].cost + options.insertion_cost;
            float costToDelete = distanceMatrix[row - 1][column].cost + options.deletion_cost;

            // TODO Unicode?
            CharType sourceElement = source[row - 1];
            CharType targetElement = target[column - 1];
            float costToSubstitute = distanceMatrix[row - 1][column - 1].cost;
            if (sourceElement != targetElement)
            {
                costToSubstitute = costToSubstitute + options.substitution_cost;
            }

            vector<CoordinateMatrixEntry> possibleParents({CoordinateMatrixEntry({costToInsert, coordPtr(row, column - 1), nullCoords()}),
                                                           CoordinateMatrixEntry({costToDelete, coordPtr(row - 1, column), nullCoords()}),
                                                           CoordinateMatrixEntry({costToSubstitute, coordPtr(row - 1, column - 1), nullCoords()})});

            // We can add damerau to the possibleParents if the current
            // target-letter has been encountered in our lastRowMap,
            // and if there exists a previous column in this row where the
            // row & column letters matched
            bool canDamerau = isUnrestrictedDamerau &&
                              row > 1 &&
                              column > 1 &&
                              lastColMatch > 0 &&
                              lastRowMap.find(targetElement) != lastRowMap.end();

            if (canDamerau)
            {
                int lastRowMatch = lastRowMap.at(targetElement);
                float costBeforeTransposition =
                    distanceMatrix[lastRowMatch - 1][lastColMatch - 1].cost;
                float costToTranspose = costBeforeTransposition + ((row - lastRowMatch - 1) * options.deletion_cost) + ((column - lastColMatch - 1) * options.insertion_cost) + options.transposition_cost;
                possibleParents.push_back({costToTranspose,
                                           coordPtr(lastRowMatch - 1, lastColMatch - 1),
                                           nullCoords()});
            }
            // Source and target chars are 1-indexed in the distanceMatrix so previous
            // source/target element is (col/row - 2)
            bool canDoRestrictedDamerau = isRestrictedDamerau && row > 1 && column > 1 && sourceElement == target[column - 2] && source[row - 2] == targetElement;

            if (canDoRestrictedDamerau)
            {
                float costBeforeTransposition = distanceMatrix[row - 2][column - 2].cost;
                possibleParents.push_back({costBeforeTransposition + options.transposition_cost,
                                           coordPtr(row - 2, column - 2),
                                           nullCoords()});
            }

            auto minCostParent = min_element(
                possibleParents.begin(), possibleParents.end(),
                matrixEntryComparator);

            distanceMatrix[row][column] = CoordinateMatrixEntry({minCostParent->cost, nullCoords(), minCostParent->coordinates});

            if (isUnrestrictedDamerau)
            {
                lastRowMap.emplace(sourceElement, row);
                if (sourceElement == targetElement)
                {
                    lastColMatch = column;
                }
            }
        }
    }

    if (!options.search)
    {
        return distanceMatrix[sourceLength][targetLength].cost;
    }

    return getMinCostSubstring(distanceMatrix, source, target).distance;
}

napi_value wrapper(napi_env env, napi_callback_info info)
{
    napi_value argv[3];
    size_t argc = 3;
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

    if (argc < 2)
    {
        napi_throw_error(env, "EINVAL", "Too few arguments");
        return NULL;
    }

#ifdef USE_UTF16
    char16_t source[1024];
    size_t source_len;
    if (napi_get_value_string_utf16(env, argv[0], (char16_t *)&source, 1024, &source_len) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "Expected string");
        return NULL;
    }

    char16_t target[1024];
    size_t target_len;
    if (napi_get_value_string_utf16(env, argv[1], (char16_t *)&target, 1024, &target_len) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "Expected string");
        return NULL;
    }
#else
    char source[1024];
    size_t source_len;
    if (napi_get_value_string_utf8(env, argv[0], (char *)&source, 1024, &source_len) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "Expected string");
        return NULL;
    }

    char target[1024];
    size_t target_len;
    if (napi_get_value_string_utf8(env, argv[1], (char *)&target, 1024, &target_len) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "Expected string");
        return NULL;
    }
#endif

    Options options;

    napi_value returnValue;
    napi_status status = napi_create_double(env, levenshteinDistance(source, target, options), &returnValue);
    if (status != napi_ok)
        return NULL;
    return returnValue;
}

napi_value init_all(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value dist_fn;
    status = napi_create_function(env, NULL, 0, wrapper, NULL, &dist_fn);
    if (status != napi_ok)
        return NULL;
    status = napi_set_named_property(env, exports, "levenshteinDistance", dist_fn);
    if (status != napi_ok)
        return NULL;
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init_all)