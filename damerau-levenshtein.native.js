const binding = require('node-gyp-build')(__dirname);

const defaultOpts = {
  insertion_cost: 1,
  deletion_cost: 1,
  substitution_cost: 1,
  transposition_cost: 1,
  search: false,
  damerau: false,
  restricted: false,
};

function levenshteinWrapper(source, target, options) {
  const opts = Object.assign({}, defaultOpts, options || {});
  const res = binding.levenshteinDistance(
    source,
    target,
    opts.insertion_cost,
    opts.deletion_cost,
    opts.substitution_cost,
    opts.transposition_cost,
    opts.search,
    opts.restricted,
    opts.damerau
  );
  return opts.search ? res : res.distance;
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

module.exports = {
  LevenshteinDistance,
  DamerauLevenshteinDistance,
  initialized: function () {
    return true;
  },
};
