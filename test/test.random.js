const JSC = require('jscheck');
const { DamerauLevenshteinDistance } = require('../damerau-levenshtein.natural');
const native = require('../damerau-levenshtein.native');

const testStringPairs = [];
for (let i = 0; i < 2000; i++) {
  testStringPairs.push([JSC.string(50, 100)(), JSC.string(5, 10)()]);
}

let naturalTime;
const distances = [];
describe('natural.DamerauLevenshteinDistance', () => {
  it('should generate distances', () => {
    const t0 = Date.now();
    for (const [src, target] of testStringPairs) {
      distances.push(
        DamerauLevenshteinDistance(src, target, {
          search: true,
        }).distance
      );
    }
    naturalTime = Date.now() - t0;
  }, 30 * 1000);
});

describe('n-api LevenshteinDistance', () => {
  const nApiDistances = [];

  let nApiTime;
  it('should generate correct distances', () => {
    const t0 = Date.now();
    for (const [src, target] of testStringPairs) {
      nApiDistances.push(native(src, target, { search: true }));
    }
    nApiTime = Date.now() - t0;

    for (let i = 0, len = testStringPairs.length; i < len; i++) {
      expect(nApiDistances[i]).toBe(distances[i]);
    }
  });

  it('should be faster than natural', () => {
    const factor = naturalTime / nApiTime;
    console.log('n-api version is', factor.toFixed(1), 'times faster');
    expect(factor).toBeGreaterThan(2);
  });
});
