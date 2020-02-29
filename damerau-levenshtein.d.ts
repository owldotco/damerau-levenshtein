declare function LevenshteinDistance(source: string, target: string, options?: any): number;

export interface DamerauLevenshteinDistanceOptions {
    /** @default 1 */
    insertion_cost?: number;
    /** @default 1 */
    deletion_cost?: number;
    /** @default 1 */
    substitution_cost?: number;
    /** @default 1 */
    transposition_cost?: number;
    /** @default false */
    search?: boolean;
    /** @default false */
    restricted?: boolean;
}

interface SubstringDistanceResult {
    substring: string;
    distance: number;
    offset: number;
}

/**
 * Returns the Damerau-Levenshtein distance between strings. Counts the distance
 * between two strings by returning the number of edit operations required to
 * convert `source` into `target`.
 *
 * Valid edit operations are:
 *  - transposition, insertion, deletion, and substitution
 */
export function DamerauLevenshteinDistance(
    source: string,
    target: string,
    options: DamerauLevenshteinDistanceOptions & { search: true }
): SubstringDistanceResult;

export function DamerauLevenshteinDistance(
    source: string,
    target: string,
    options?: DamerauLevenshteinDistanceOptions & { search?: false }
): number;

export function DamerauLevenshteinDistance(
    source: string,
    target: string,
    options: DamerauLevenshteinDistanceOptions & { search: boolean }
): number | SubstringDistanceResult;