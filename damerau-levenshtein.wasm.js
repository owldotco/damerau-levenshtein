/** @typedef {number & {}} Ptr */

/**
 * @typedef {{
 *   _useUtf16(): 0 | 1;
 *   _call_levenshteinDistance(
 *     source: Ptr,
 *     target: Ptr,
 *     insertion_cost: number,
 *     deletion_cost: number,
 *     substitution_cost: number,
 *     transposition_cost: number,
 *     search: boolean,
 *     restricted: boolean,
 *     damerau: boolean,
 *     outPtr: Ptr): void;
 *   _sizeof_MinCostSubstringStruct(): number;
 *   _get_MinCostSubstringStruct_distance(ptr: Ptr): number;
 *   _get_MinCostSubstringStruct_offset(ptr: Ptr): number;
 *   _get_MinCostSubstringStruct_substring(ptr: Ptr): Ptr;
 *   UTF8ToString: typeof UTF8ToString;
 *   stringToUTF8: typeof stringToUTF8;
 *   UTF16ToString: typeof UTF16ToString;
 *   stringToUTF16: typeof stringToUTF16;
 *   lengthBytesUTF8: typeof lengthBytesUTF8;
 *   lengthBytesUTF16: typeof lengthBytesUTF16;
 *   calledRun?: boolean;
 *   asm: {[k: string]: Function};
 *   quit?(status: unknown, toThrow: Error): void;
 * } & EmscriptenModule} LevenshteinModule
 */

// TODO: see if there's an emscripten build flag to disable the throw on uncaught/unhandled
// monkey patch so the emscripten binding doesn't bind to process events.
const processOn = process['on'];
process['on'] = function () {
  return this;
};

/** @type LevenshteinModule */
const binding = require('./dist/levenshtein');

process['on'] = processOn;

binding.quit = function (status, toThrow) {
  process.emitWarning(toThrow);
};

const defaultOpts = {
  insertion_cost: 1,
  deletion_cost: 1,
  substitution_cost: 1,
  transposition_cost: 1,
  search: false,
  damerau: false,
  restricted: false,
};

/** @type {{
    useUtf16: 0 | 1;
    stringToUtf: (str: string, outPtr: number, maxBytesToRead?: number) => void;
    utfToString: (ptr: number, maxBytesToRead?: number) => string;
    lengthBytes: (s: string) => number;
    paddingBytes: number;
}} */
let helpers;
function getHelpers() {
  if (helpers) {
    return helpers;
  }
  if (!initializedSync()) {
    throw new Error(
      'damerau-levenshtein wasm binding has not finished loading'
    );
  }
  const useUtf16 = binding._useUtf16();
  const stringToUtf = useUtf16 ? binding.stringToUTF16 : binding.stringToUTF8;

  const utfToString = useUtf16 ? binding.UTF16ToString : binding.UTF8ToString;

  /** @type {(s: string) => number} */
  const lengthBytes = useUtf16
    ? binding.lengthBytesUTF16
    : binding.lengthBytesUTF8;

  const paddingBytes = useUtf16 ? 2 : 1;

  helpers = {
    useUtf16,
    stringToUtf,
    utfToString,
    lengthBytes,
    paddingBytes,
  };
  return helpers;
}

/**
 * @param source {Parameters<typeof import('./damerau-levenshtein').DamerauLevenshteinDistance>[0]}
 * @param target {Parameters<typeof import('./damerau-levenshtein').DamerauLevenshteinDistance>[1]}
 * @param options {Parameters<typeof import('./damerau-levenshtein').DamerauLevenshteinDistance>[2]=}
 * @returns {ReturnType<typeof import('./damerau-levenshtein').DamerauLevenshteinDistance>}
 */
function levenshteinWrapper(source, target, options) {
  const { lengthBytes, stringToUtf, paddingBytes, utfToString } = getHelpers();
  const opts = Object.assign({}, defaultOpts, options || {});
  const sourcePtr = binding._malloc(lengthBytes(source) + paddingBytes);
  const targetPtr = binding._malloc(lengthBytes(source) + paddingBytes);
  const outPtr = binding._malloc(binding._sizeof_MinCostSubstringStruct());
  stringToUtf(source, sourcePtr);
  stringToUtf(target, targetPtr);
  binding._call_levenshteinDistance(
    sourcePtr,
    targetPtr,
    opts.insertion_cost,
    opts.deletion_cost,
    opts.substitution_cost,
    opts.transposition_cost,
    opts.search,
    opts.restricted,
    opts.damerau,
    outPtr
  );
  binding._free(sourcePtr);
  binding._free(targetPtr);
  if (!opts.search) {
    const distance = binding._get_MinCostSubstringStruct_distance(outPtr);
    binding._free(outPtr);
    return distance;
  }
  const substringPtr = binding._get_MinCostSubstringStruct_substring(outPtr);
  const substring = utfToString(substringPtr);
  const res = {
    substring,
    distance: binding._get_MinCostSubstringStruct_distance(outPtr),
    offset: binding._get_MinCostSubstringStruct_offset(outPtr),
  };
  binding._free(outPtr);
  return res;
}

function DamerauLevenshteinDistance(source, target, options) {
  return levenshteinWrapper(
    source,
    target,
    Object.assign({ transposition_cost: 1, restricted: false }, options || {}, {
      damerau: true,
    })
  );
}

function LevenshteinDistance(source, target, options) {
  return levenshteinWrapper(
    source,
    target,
    Object.assign({}, options || {}, { damerau: false })
  );
}

function initializedSync() {
  return binding.calledRun || Object.keys(binding.asm).length > 0;
}

/** @type {Promise<true> | undefined} */
let initializePromise;
function initialized() {
  if (initializedSync()) {
    return true;
  }
  if (initializePromise) {
    return initializePromise;
  }
  // eslint-disable-next-line no-undef
  initializePromise = new Promise((resolve, reject) => {
    binding.onRuntimeInitialized = function () {
      resolve(true);
    };
    binding.onAbort = function (what) {
      reject(new Error(what));
    };
  });
  return initializePromise;
}

module.exports = {
  LevenshteinDistance,
  DamerauLevenshteinDistance,
  initialized,
};
