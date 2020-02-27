const binding = require('node-gyp-build')(__dirname);

module.exports = binding.levenshteinDistance;

if (module === require.main) {
  console.log('result:', binding.levenshteinDistance('foo', 'bar'));
}
