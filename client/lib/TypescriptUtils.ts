export enum BedrockDateParseMode {
  Auto = "auto",
  LocalDateOnly = "localDateOnly",
  StripTimezoneOffset = "stripTimezoneOffset",
  StrictIso = "strictIso",
}

export interface BedrockParseDateOptions {
  mode?: BedrockDateParseMode;
}

export interface BedrockParseNumberOptions {
  min?: number;
  max?: number;
  integerOnly?: boolean;
  positiveOnly?: boolean;
}

export interface BedrockIsEqualOptions {
  coerceComparableValues?: boolean;
  ignoreArrayOrder?: boolean;
}

export enum BedrockBooleanParseMode {
  Loose = "loose",
  Strict = "strict",
}

export default class BedrockUtils {
  public static clone<Type>(value?: Type): Type {
    if (value === null || typeof value !== "object") {
      return value as Type;
    }

    return JSON.parse(JSON.stringify(value)) as Type;
  }

  public static isEqual(obj1?: unknown, obj2?: unknown, options?: BedrockIsEqualOptions): boolean {
    return BedrockUtils.deepEqual(obj1, obj2, new WeakMap<object, WeakSet<object>>(), options);
  }

  public static isObject(value?: unknown): value is Record<PropertyKey, unknown> {
    return value !== null && typeof value === "object" && !Array.isArray(value);
  }

  public static deepMerge<T extends object>(target: T, ...sources: object[]): T {
    if (sources.length === 0) {
      return target;
    }

    const source = BedrockUtils.clone(sources.shift());
    if (BedrockUtils.isObject(target) && BedrockUtils.isObject(source)) {
      for (const key of Reflect.ownKeys(source)) {
        const sourceValue = source[key];
        if (BedrockUtils.isObject(sourceValue)) {
          const targetValue = target[key];
          if (!BedrockUtils.isObject(targetValue)) {
            Object.assign(target, { [key]: {} });
          }

          BedrockUtils.deepMerge(target[key] as Record<PropertyKey, unknown>, sourceValue);
          continue;
        }

        Object.assign(target, { [key]: sourceValue });
      }
    }

    return BedrockUtils.deepMerge(target, ...sources);
  }

  public static isNullOrWhiteSpace(value?: unknown): boolean {
    if (value == null) {
      return true;
    }

    const parsedValue = BedrockUtils.parseString(value);
    return parsedValue == null || parsedValue.replace(/\s/gi, "").length < 1;
  }

  public static isNullOrEmpty(value?: unknown): boolean {
    if (value == null) {
      return true;
    }

    const parsedValue = BedrockUtils.parseString(value);
    return parsedValue != null ? parsedValue.length === 0 : false;
  }

  public static parseString(value?: unknown): string | null {
    if (value == null) {
      return null;
    }

    if (typeof value === "string") {
      return value;
    }

    if (typeof value === "number") {
      return Number.isFinite(value) ? value.toString() : null;
    }

    if (typeof value === "boolean" || typeof value === "bigint") {
      return value.toString();
    }

    if (value instanceof String || value instanceof Number || value instanceof Boolean) {
      return BedrockUtils.parseString(value.valueOf());
    }

    return null;
  }

  public static parseNumber(value?: unknown, options?: BedrockParseNumberOptions): number | null {
    if (value == null) {
      return null;
    }

    if (typeof value === "number") {
      return Number.isFinite(value) ? BedrockUtils.applyNumberConstraints(value, options) : null;
    }

    if (typeof value === "bigint") {
      const converted = Number(value);
      return Number.isFinite(converted) ? BedrockUtils.applyNumberConstraints(converted, options) : null;
    }

    if (typeof value === "string") {
      const normalized = value.trim();
      if (normalized === "") {
        return null;
      }

      const parsedNumber = Number(normalized);
      return Number.isFinite(parsedNumber) ? BedrockUtils.applyNumberConstraints(parsedNumber, options) : null;
    }

    if (value instanceof Number || value instanceof String || value instanceof Boolean) {
      return BedrockUtils.parseNumber(value.valueOf(), options);
    }

    return null;
  }

  public static parseInteger(value?: unknown): number | null {
    return BedrockUtils.parseNumber(value, { integerOnly: true });
  }

  public static parseBoolean(
    value?: unknown,
    mode: BedrockBooleanParseMode = BedrockBooleanParseMode.Loose
  ): boolean | null {
    if (value == null) {
      return null;
    }

    if (typeof value === "boolean") {
      return value;
    }

    if (typeof value === "number") {
      if (Number.isNaN(value)) {
        return null;
      }
      if (mode === BedrockBooleanParseMode.Strict) {
        return value === 1 ? true : value === 0 ? false : null;
      }
      return value !== 0;
    }

    if (typeof value === "bigint") {
      if (mode === BedrockBooleanParseMode.Strict) {
        return value === 1n ? true : value === 0n ? false : null;
      }
      return value !== 0n;
    }

    if (typeof value === "string") {
      const normalized = value.trim().toLowerCase();
      if (normalized === "") {
        return null;
      }

      if (["true", "1", "yes", "y", "on", "t", "enable", "enabled"].includes(normalized)) {
        return true;
      }

      if (["false", "0", "no", "n", "off", "f", "disable", "disabled"].includes(normalized)) {
        return false;
      }

      if (mode === BedrockBooleanParseMode.Strict) {
        return null;
      }

      const parsedNumber = Number(normalized);
      if (!Number.isNaN(parsedNumber)) {
        return parsedNumber !== 0;
      }

      return null;
    }

    if (value instanceof Boolean || value instanceof Number || value instanceof String) {
      return BedrockUtils.parseBoolean(value.valueOf(), mode);
    }

    return null;
  }

  public static parseDate(value?: unknown, options?: BedrockParseDateOptions): Date | null {
    if (value == null) {
      return null;
    }

    const mode = options?.mode ?? BedrockDateParseMode.Auto;

    if (typeof value === "number") {
      if (!Number.isFinite(value)) {
        return null;
      }

      const parsedDate = new Date(value);
      return Number.isNaN(parsedDate.getTime()) ? null : parsedDate;
    }

    if (typeof value === "string") {
      const normalized = value.trim();
      if (normalized === "") {
        return null;
      }

      if (mode === BedrockDateParseMode.StrictIso && !BedrockUtils.isIsoLikeDateString(normalized)) {
        return null;
      }

      if (mode === BedrockDateParseMode.LocalDateOnly) {
        const localDateOnlyMatch = /^(\d{4})-(\d{2})-(\d{2})$/.exec(normalized);
        if (localDateOnlyMatch) {
          const [, year, month, day] = localDateOnlyMatch;
          const parsedDate = new Date(Number(year), Number(month) - 1, Number(day));
          return Number.isNaN(parsedDate.getTime()) ? null : parsedDate;
        }
      }

      const dateToParse =
        mode === BedrockDateParseMode.StripTimezoneOffset
          ? normalized.replace(/(?:Z|[+\-]\d{2}:\d{2})$/i, "")
          : normalized;

      const parsedDate = new Date(dateToParse);
      return Number.isNaN(parsedDate.getTime()) ? null : parsedDate;
    }

    if (value instanceof Date) {
      const timestamp = value.getTime();
      return Number.isNaN(timestamp) ? null : new Date(timestamp);
    }

    if (value instanceof Number || value instanceof String) {
      return BedrockUtils.parseDate(value.valueOf(), options);
    }

    return null;
  }

  public static dedupe<T>(values?: T[] | null): T[] {
    if (!Array.isArray(values) || values.length === 0) {
      return [];
    }

    const seen = new Set<T>();
    const deduped: T[] = [];
    for (const value of values) {
      if (seen.has(value)) {
        continue;
      }

      seen.add(value);
      deduped.push(value);
    }

    return deduped;
  }

  public static parseIntervalToMilliseconds(interval?: unknown): number {
    const intervalMapping = {
      s: 1000,
      m: 1000 * 60,
      h: 1000 * 60 * 60,
      d: 1000 * 60 * 60 * 24,
      w: 1000 * 60 * 60 * 24 * 7,
      M: 1000 * 60 * 60 * 24 * 30,
      y: 1000 * 60 * 60 * 24 * 30 * 12,
    } as const;

    const parsedInterval = BedrockUtils.parseString(interval);
    if (parsedInterval == null) {
      return intervalMapping.h;
    }

    const match = /(\d+)([smhdwMy])+/.exec(parsedInterval);
    if (match) {
      const [, amount, unit] = match;
      return intervalMapping[unit as keyof typeof intervalMapping] * parseInt(amount, 10);
    }

    return intervalMapping.h;
  }

  public static parseArray<T>(value: unknown, itemParser: (x: unknown) => T | null): T[] | null {
    if (value == null || typeof itemParser !== "function") {
      return null;
    }

    let items: unknown[] | null = null;
    if (Array.isArray(value)) {
      items = value;
    } else if (typeof value === "string") {
      const normalized = value.trim();
      if (normalized === "") {
        return null;
      }

      try {
        const parsed = JSON.parse(normalized);
        if (Array.isArray(parsed)) {
          items = parsed;
        }
      } catch {
        return null;
      }
    }

    if (items == null) {
      return null;
    }

    const result: T[] = [];
    for (const item of items) {
      const parsedItem = itemParser(item);
      if (parsedItem == null) {
        return null;
      }

      result.push(parsedItem);
    }

    return result;
  }

  public static parseEnumValue<T extends string | number>(value: unknown, enumType: Record<string, T>): T | null {
    if (value == null) {
      return null;
    }

    const enumValues = Object.values(enumType).filter(
      (enumValue: T): enumValue is T => typeof enumValue === "string" || typeof enumValue === "number"
    );

    const resolvedValue =
      value instanceof String || value instanceof Number || value instanceof Boolean ? value.valueOf() : value;

    if (enumValues.includes(resolvedValue as T)) {
      return resolvedValue as T;
    }

    if (typeof resolvedValue === "string") {
      const trimmedValue = resolvedValue.trim();
      if (trimmedValue === "") {
        return null;
      }

      if (enumValues.includes(trimmedValue as T)) {
        return trimmedValue as T;
      }

      const numericValue = BedrockUtils.parseNumber(trimmedValue);
      if (numericValue != null && enumValues.includes(numericValue as T)) {
        return numericValue as T;
      }
    }

    return null;
  }

  public static isNumeric(value?: unknown): boolean {
    return BedrockUtils.parseNumber(value) != null;
  }

  public static isNumber(value?: unknown): value is number {
    return typeof value === "number" && Number.isFinite(value);
  }

  public static isInteger(value?: unknown): boolean {
    return BedrockUtils.parseInteger(value) != null;
  }

  public static isDate(value?: unknown): boolean {
    return BedrockUtils.parseDate(value) != null;
  }

  private static applyNumberConstraints(value: number, options?: BedrockParseNumberOptions): number | null {
    if (options?.integerOnly === true && !Number.isInteger(value)) {
      return null;
    }

    if (options?.positiveOnly === true && value <= 0) {
      return null;
    }

    if (options?.min != null && Number.isFinite(options.min) && value < options.min) {
      return null;
    }

    if (options?.max != null && Number.isFinite(options.max) && value > options.max) {
      return null;
    }

    return value;
  }

  private static isIsoLikeDateString(value: string): boolean {
    return /^\d{4}-\d{2}-\d{2}(?:[Tt ]\d{2}:\d{2}(?::\d{2}(?:\.\d{1,7})?)?(?:Z|[+\-]\d{2}:\d{2})?)?$/.test(value);
  }

  private static deepEqual(
    first: unknown,
    second: unknown,
    visitedPairs: WeakMap<object, WeakSet<object>>,
    options?: BedrockIsEqualOptions
  ): boolean {
    if (Object.is(first, second)) {
      return true;
    }

    if (first == null || second == null) {
      return false;
    }

    const coercedPair = BedrockUtils.tryCoerceComparableValues(first, second, options);
    if (coercedPair != null) {
      [first, second] = coercedPair;
      if (Object.is(first, second)) {
        return true;
      }
    }

    if (typeof first !== typeof second) {
      return false;
    }

    if (typeof first !== "object") {
      return false;
    }

    const firstObject = first as object;
    const secondObject = second as object;
    if (BedrockUtils.hasVisitedPair(firstObject, secondObject, visitedPairs)) {
      return true;
    }
    BedrockUtils.markVisitedPair(firstObject, secondObject, visitedPairs);

    if (Array.isArray(first) || Array.isArray(second)) {
      if (!Array.isArray(first) || !Array.isArray(second) || first.length !== second.length) {
        return false;
      }

      if (options?.ignoreArrayOrder === true) {
        const remainingValues = Array.from(second.values());
        for (const firstValue of first.values()) {
          const matchIndex = remainingValues.findIndex((secondValue) =>
            BedrockUtils.deepEqual(firstValue, secondValue, new WeakMap<object, WeakSet<object>>(), options)
          );
          if (matchIndex < 0) {
            return false;
          }
          remainingValues.splice(matchIndex, 1);
        }

        return remainingValues.length === 0;
      }

      for (let index = 0; index < first.length; index += 1) {
        if (!BedrockUtils.deepEqual(first[index], second[index], visitedPairs, options)) {
          return false;
        }
      }

      return true;
    }

    if (first instanceof Date || second instanceof Date) {
      if (!(first instanceof Date) || !(second instanceof Date)) {
        return false;
      }

      const firstTime = first.getTime();
      const secondTime = second.getTime();
      return firstTime === secondTime || (Number.isNaN(firstTime) && Number.isNaN(secondTime));
    }

    if (first instanceof RegExp || second instanceof RegExp) {
      return first instanceof RegExp && second instanceof RegExp && first.source === second.source && first.flags === second.flags;
    }

    if (first instanceof Map || second instanceof Map) {
      if (!(first instanceof Map) || !(second instanceof Map) || first.size !== second.size) {
        return false;
      }

      const firstMap = first as Map<unknown, unknown>;
      const secondMap = second as Map<unknown, unknown>;
      const remainingEntries = Array.from(secondMap.entries());
      for (const [firstKey, firstValue] of firstMap.entries()) {
        const matchIndex = remainingEntries.findIndex(([secondKey, secondValue]: [unknown, unknown]) => {
          return (
            BedrockUtils.deepEqual(firstKey, secondKey, visitedPairs, options) &&
            BedrockUtils.deepEqual(firstValue, secondValue, visitedPairs, options)
          );
        });

        if (matchIndex < 0) {
          return false;
        }

        remainingEntries.splice(matchIndex, 1);
      }

      return remainingEntries.length === 0;
    }

    if (first instanceof Set || second instanceof Set) {
      if (!(first instanceof Set) || !(second instanceof Set) || first.size !== second.size) {
        return false;
      }

      const firstSet = first as Set<unknown>;
      const secondSet = second as Set<unknown>;
      const remainingValues = Array.from(secondSet.values());
      for (const firstValue of firstSet.values()) {
        const matchIndex = remainingValues.findIndex((secondValue: unknown) =>
          BedrockUtils.deepEqual(firstValue, secondValue, visitedPairs, options)
        );
        if (matchIndex < 0) {
          return false;
        }
        remainingValues.splice(matchIndex, 1);
      }

      return remainingValues.length === 0;
    }

    if (ArrayBuffer.isView(first) || ArrayBuffer.isView(second)) {
      if (!ArrayBuffer.isView(first) || !ArrayBuffer.isView(second)) {
        return false;
      }

      if (first.constructor !== second.constructor || first.byteLength !== second.byteLength) {
        return false;
      }

      if (first instanceof DataView && second instanceof DataView) {
        for (let index = 0; index < first.byteLength; index += 1) {
          if (first.getUint8(index) !== second.getUint8(index)) {
            return false;
          }
        }
        return true;
      }

      const firstBytes = new Uint8Array(first.buffer, first.byteOffset, first.byteLength);
      const secondBytes = new Uint8Array(second.buffer, second.byteOffset, second.byteLength);
      for (let index = 0; index < firstBytes.length; index += 1) {
        if (firstBytes[index] !== secondBytes[index]) {
          return false;
        }
      }
      return true;
    }

    if (Object.getPrototypeOf(first) !== Object.getPrototypeOf(second)) {
      return false;
    }

    const firstKeys = Reflect.ownKeys(first as object);
    const secondKeys = Reflect.ownKeys(second as object);
    if (firstKeys.length !== secondKeys.length) {
      return false;
    }

    for (const key of firstKeys) {
      if (!secondKeys.includes(key)) {
        return false;
      }

      const firstValue = (first as Record<PropertyKey, unknown>)[key];
      const secondValue = (second as Record<PropertyKey, unknown>)[key];
      if (!BedrockUtils.deepEqual(firstValue, secondValue, visitedPairs, options)) {
        return false;
      }
    }

    return true;
  }

  private static tryCoerceComparableValues(
    first: unknown,
    second: unknown,
    options?: BedrockIsEqualOptions
  ): [unknown, unknown] | null {
    if (options?.coerceComparableValues !== true) {
      return null;
    }

    if (typeof first === "object" || typeof second === "object") {
      return null;
    }

    const coercionStrategies: Array<(value: unknown) => unknown | null> = [
      (value) => BedrockUtils.parseNumber(value),
      (value) => BedrockUtils.parseBoolean(value, BedrockBooleanParseMode.Strict),
      (value) => {
        const parsedDate = BedrockUtils.parseDate(value);
        return parsedDate == null ? null : parsedDate.getTime();
      },
      (value) => BedrockUtils.parseString(value),
    ];

    for (const strategy of coercionStrategies) {
      const parsedFirst = strategy(first);
      const parsedSecond = strategy(second);
      if (parsedFirst != null && parsedSecond != null) {
        return [parsedFirst, parsedSecond];
      }
    }

    return null;
  }

  private static hasVisitedPair(first: object, second: object, visitedPairs: WeakMap<object, WeakSet<object>>): boolean {
    const pairedObjects = visitedPairs.get(first);
    return pairedObjects != null && pairedObjects.has(second);
  }

  private static markVisitedPair(first: object, second: object, visitedPairs: WeakMap<object, WeakSet<object>>): void {
    let pairedObjects = visitedPairs.get(first);
    if (pairedObjects == null) {
      pairedObjects = new WeakSet<object>();
      visitedPairs.set(first, pairedObjects);
    }

    pairedObjects.add(second);
  }
}
