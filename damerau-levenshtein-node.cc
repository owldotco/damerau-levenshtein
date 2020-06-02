#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#define NAPI_VERSION 5
#include <node_api.h>

#include "./damerau-levenshtein.cc"

using namespace std;

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

    MinCostSubstringStruct res = levenshteinDistance(source, target, options);

    napi_value resOb;
    napi_value resSubstring;
    napi_value resDistance;
    napi_value resOffset;

    if (_napi_create_string(env, res.substring.c_str(), res.substring.length(), &resSubstring) != napi_ok)
        return NULL;
    if (napi_create_double(env, res.distance, &resDistance) != napi_ok)
        return NULL;
    if (napi_create_int32(env, res.offset, &resOffset) != napi_ok)
        return NULL;

    if (napi_create_object(env, &resOb) != napi_ok)
        return NULL;

    if (napi_set_named_property(env, resOb, "substring", resSubstring) != napi_ok)
        return NULL;
    if (napi_set_named_property(env, resOb, "distance", resDistance) != napi_ok)
        return NULL;
    if (napi_set_named_property(env, resOb, "offset", resOffset) != napi_ok)
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
