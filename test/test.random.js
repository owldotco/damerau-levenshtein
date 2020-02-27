const JSC = require('jscheck');
const { LevenshteinDistance } = require('natural');
const native = require('../index');

const testStringPairs = [];
for (let i = 0; i < 1000; i++) {
  testStringPairs.push([JSC.string(50, 100)(), JSC.string(5, 10)()]);
}

let naturalTime;
const distances = [];
describe('natural.LevenshteinDistance', () => {
  it('should generate distances', () => {
    const t0 = Date.now();
    for (const [src, target] of testStringPairs) {
      distances.push(
        LevenshteinDistance(src, target, {
          search: true,
        }).distance
      );
    }
    naturalTime = Date.now() - t0;
  });
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
