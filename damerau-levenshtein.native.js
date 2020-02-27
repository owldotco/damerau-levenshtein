const binding = require('node-gyp-build')(__dirname);

const defaultOpts = {
  insertion_cost: 1,
  deletion_cost: 1,
  substitution_cost: 1,
  transposition_cost: 1,
  search: false,
  damerau: false,
  restricted: false
};

module.exports = (source, target, options) => 
  binding.levenshteinDistance(source, target, Object.assign({}, defaultOpts, options || {}));
