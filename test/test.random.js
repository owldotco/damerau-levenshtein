const JSC = require('jscheck');
const natural = require('../damerau-levenshtein.natural');
const native = require('../damerau-levenshtein.native');

const opts = [];
for (const insertion_cost of [0.5, 1]) {
  for (const deletion_cost of [0.5, 1]) {
    for (const substitution_cost of [0.5, 1]) {
      for (const transposition_cost of [0.5, 1]) {
        for (const search of [true, false]) {
          for (const damerau of [true, false]) {
            for (const restricted of [true, false]) {
              opts.push({
                insertion_cost,
                deletion_cost,
                substitution_cost,
                transposition_cost,
                search,
                damerau,
                restricted,
              });
            }
          }
        }
      }
    }
  }
}

const maxTimeTotal = 30 * 1000;
const minTimePerOpts = 100;
const maxTimePerOpts = Math.max(minTimePerOpts, maxTimeTotal / opts.length);

const speedupFactors = [];
let totalNaturalTime = 0;
let totalNapiTime = 0;
let totalSamples = 0;

const longStrGenerator = JSC.string(50, 100, JSC.character(32, 1 << 24));
const shortStrGenerator = JSC.string(5, 15, JSC.character(32, 1 << 24));

const mismatches = [];

afterAll(() => {
  let totalFactors = 0;
  for (const f of speedupFactors) totalFactors += f;

  console.log({
    totalFactors,
    'speedupFactors.length': speedupFactors.length,
    totalNaturalTime,
    totalNapiTime,
    totalSamples,
  });

  console.log(
    'Mean speedup factor:',
    (totalFactors / speedupFactors.length).toFixed(1)
  );
  console.log(
    'Overall speedup factor:',
    (totalNaturalTime / totalNapiTime).toFixed(1)
  );
  console.log('Total samples:', totalSamples);

  if (mismatches.length > 0) {
    console.error('Failing options:', mismatches);
  }
});

for (const options of opts) {
  describe(`LevenshteinDistance with options: ${JSON.stringify(
    options
  )}`, () => {
    const testStringPairs = [];
    for (let i = 0; i < 500; i++) {
      testStringPairs.push([longStrGenerator(), shortStrGenerator()]);
    }

    let naturalTime;
    const naturalDistances = [];
    const nApiDistances = [];

    it(
      'should generate distances with natural',
      () => {
        const t0 = Date.now();
        for (
          let i = 0, len = testStringPairs.length;
          i < len && Date.now() - t0 < maxTimePerOpts;
          i++
        ) {
          const [src, target] = testStringPairs[i];
          naturalDistances.push(
            natural.LevenshteinDistance(src, target, options)
          );
        }
        naturalTime = Date.now() - t0;
        totalNaturalTime += naturalTime;
        totalSamples += naturalDistances.length;
      },
      maxTimePerOpts + 500
    );

    let nApiTime;
    it(
      'should generate correct distances with native code',
      () => {
        const t0 = Date.now();
        for (let i = 0, len = naturalDistances.length; i < len; i++) {
          const [src, target] = testStringPairs[i];
          nApiDistances.push(native.LevenshteinDistance(src, target, options));
        }
        nApiTime = Date.now() - t0;
        totalNapiTime += nApiTime;

        for (let i = 0, len = naturalDistances.length; i < len; i++) {
          const expected = naturalDistances[i];
          const actual = nApiDistances[i];
          try {
            expect(expected).toEqual(actual);
          } catch (err) {
            mismatches.push({ options, expected, actual });
            throw err;
          }
        }
      },
      maxTimePerOpts + 500
    );

    it('should be faster than natural in native mode', () => {
      const factor = naturalTime / nApiTime;
      speedupFactors.push(factor);
      expect(factor).toBeGreaterThan(2);
    });
  });
}
