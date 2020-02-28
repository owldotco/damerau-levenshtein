#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#define NAPI_VERSION 5
#include <node_api.h>

using namespace std;

#define USE_UTF16 true
#define USE_STATIC_BUFFER false

#if USE_UTF16
typedef char16_t _char_type;
typedef u16string _string;
#define _napi_get_value_string napi_get_value_string_utf16
#define _napi_create_string napi_create_string_utf16
#else
typedef char _char_type;
typedef string _string;
#define _napi_get_value_string napi_get_value_string_utf8
#define _napi_create_string napi_create_string_utf8
#endif

#if USE_STATIC_BUFFER
#define MAX_STRING_BUF_LEN (1 * 1024 * 1024 / sizeof(_char_type))
#else
#define MAX_STRING_BUF_LEN (8 * 1024 * 1024 / sizeof(_char_type))
#endif

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

struct Coordinates
{
    int row = 0;
    int column = 0;
};

struct CoordinateMatrixEntry
{
    double cost = 0;
    Coordinates coordinates = Coordinates({0, 0});
    Coordinates parentCell = Coordinates({0, 0});
};

struct MinCostSubstringStruct
{
    double distance;
    _string substring;
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
        row = distanceMatrix[tmpRow][tmpColumn].parentCell.row;
        column = distanceMatrix[tmpRow][tmpColumn].parentCell.column;
    }

    return column - 1;
}

struct MatrixEntryComparator
{
    inline bool operator()(const CoordinateMatrixEntry &a, const CoordinateMatrixEntry &b) const
    {
        return a.cost < b.cost;
    }
} matrixEntryComparator;

MinCostSubstringStruct getMinCostSubstring(
    const vector<vector<CoordinateMatrixEntry>> &distanceMatrix,
    const _string &source,
    const _string &target)
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
    return {minDistance, target.substr(matchStart, matchEnd - matchStart)};
}

MinCostSubstringStruct levenshteinDistance(
    const _string &source,
    const _string &target,
    const Options &options)
{
    bool isUnrestrictedDamerau = options.damerau && !options.restricted;
    bool isRestrictedDamerau = options.damerau && options.restricted;
    unordered_map<_char_type, size_t> lastRowMap(512);

    auto sourceLength = source.length();
    auto targetLength = target.length();

    vector<vector<CoordinateMatrixEntry>> distanceMatrix(
        sourceLength + 1,
        vector<CoordinateMatrixEntry>(targetLength + 1));

    for (size_t row = 1; row <= sourceLength; row++)
    {
        distanceMatrix[row][0] = CoordinateMatrixEntry({distanceMatrix[row - 1][0].cost + options.deletion_cost, Coordinates(), {row - 1, 0}});
    }

    for (size_t column = 1; column <= targetLength; column++)
    {
        if (options.search)
        {
            distanceMatrix[0][column] = CoordinateMatrixEntry({0, Coordinates(), Coordinates()});
        }
        else
        {
            distanceMatrix[0][column] = {
                distanceMatrix[0][column - 1].cost + options.insertion_cost,
                Coordinates(),
                {0, column - 1}};
        }
    }

    for (size_t row = 1; row <= sourceLength; row++)
    {
        int lastColMatch = -1;

        for (size_t column = 1; column <= targetLength; column++)
        {
            double costToInsert = distanceMatrix[row][column - 1].cost + options.insertion_cost;
            double costToDelete = distanceMatrix[row - 1][column].cost + options.deletion_cost;

            // TODO Unicode?
            _char_type sourceElement = source[row - 1];
            _char_type targetElement = target[column - 1];
            double costToSubstitute = distanceMatrix[row - 1][column - 1].cost;
            if (sourceElement != targetElement)
            {
                costToSubstitute = costToSubstitute + options.substitution_cost;
            }

            vector<CoordinateMatrixEntry> possibleParents({CoordinateMatrixEntry({costToInsert, {row, column - 1}}),
                                                           CoordinateMatrixEntry({costToDelete, {row - 1, column}}),
                                                           CoordinateMatrixEntry({costToSubstitute, {row - 1, column - 1}})});

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
                double costBeforeTransposition =
                    distanceMatrix[lastRowMatch - 1][lastColMatch - 1].cost;
                double costToTranspose = costBeforeTransposition + ((row - lastRowMatch - 1) * options.deletion_cost) + ((column - lastColMatch - 1) * options.insertion_cost) + options.transposition_cost;
                possibleParents.push_back({costToTranspose,
                                           {lastRowMatch - 1, lastColMatch - 1}});
            }
            // Source and target chars are 1-indexed in the distanceMatrix so previous
            // source/target element is (col/row - 2)
            bool canDoRestrictedDamerau = isRestrictedDamerau && row > 1 && column > 1 && sourceElement == target[column - 2] && source[row - 2] == targetElement;

            if (canDoRestrictedDamerau)
            {
                double costBeforeTransposition = distanceMatrix[row - 2][column - 2].cost;
                possibleParents.push_back({costBeforeTransposition + options.transposition_cost,
                                           {row - 2, column - 2},
                                           Coordinates()});
            }

            auto minCostParent = min_element(
                possibleParents.begin(), possibleParents.end(),
                matrixEntryComparator);

            distanceMatrix[row][column] = CoordinateMatrixEntry({minCostParent->cost, Coordinates(), minCostParent->coordinates});

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
        return MinCostSubstringStruct({distanceMatrix[sourceLength][targetLength].cost, source});

    return getMinCostSubstring(distanceMatrix, source, target);
}

napi_status extractString(napi_env env, napi_value arg, _string& str)
{
    size_t buf_size;

#if USE_STATIC_BUFFER
    _char_type buf[MAX_STRING_BUF_LEN];
    if (_napi_get_value_string(env, arg, buf, MAX_STRING_BUF_LEN, &buf_size) != napi_ok)
        return napi_invalid_arg;
    str += _string(buf);
#else
    if (_napi_get_value_string(env, arg, NULL, MAX_STRING_BUF_LEN, &buf_size) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "String is too large");
        return napi_invalid_arg;
    }
    shared_ptr<_char_type> buf(new _char_type[buf_size + 1], default_delete<_char_type[]>());
    if (_napi_get_value_string(env, arg, buf.get(), buf_size + 1, NULL) != napi_ok)
    {
        napi_throw_error(env, "EINVAL", "Expected string");
        return napi_invalid_arg;
    }
    str += _string(buf.get());
#endif

    return napi_ok;
}

napi_value wrapper(napi_env env, napi_callback_info info)
{
    napi_value argv[10];
    size_t argc = 10;
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

    if (argc != 9)
    {
        napi_throw_error(env, "EINVAL", "Too few arguments");
        return NULL;
    }
    
    _string source;
    _string target;
    Options options;
    
    int idx = 0;
    if (extractString(env, argv[idx++], source) != napi_ok)
        return NULL;
    if (extractString(env, argv[idx++], target) != napi_ok)
        return NULL;
    if (napi_get_value_double(env, argv[idx++], &options.insertion_cost) != napi_ok)
        return NULL;
    if (napi_get_value_double(env, argv[idx++], &options.deletion_cost) != napi_ok)
        return NULL;
    if (napi_get_value_double(env, argv[idx++], &options.substitution_cost) != napi_ok)
        return NULL;
    if (napi_get_value_double(env, argv[idx++], &options.transposition_cost) != napi_ok)
        return NULL;
    if (napi_get_value_bool(env, argv[idx++], &options.search) != napi_ok)
        return NULL;
    if (napi_get_value_bool(env, argv[idx++], &options.restricted) != napi_ok)
        return NULL;
    if (napi_get_value_bool(env, argv[idx++], &options.damerau) != napi_ok)
        return NULL;

    napi_value returnValue;

    MinCostSubstringStruct res = levenshteinDistance(source, target, options);

    napi_value resOb;
    napi_value resSubstring;
    napi_value resDistance;

    if (_napi_create_string(env, res.substring.c_str(), res.substring.length(), &resSubstring) != napi_ok)
        return NULL;

    if (napi_create_double(env, res.distance, &resDistance) != napi_ok)
        return NULL;

    if (napi_create_object(env, &resOb) != napi_ok)
        return NULL;

    if (napi_set_named_property(env, resOb, "substring", resSubstring) != napi_ok)
        return NULL;
    if (napi_set_named_property(env, resOb, "distance", resDistance) != napi_ok)
        return NULL;

    return resOb;
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
