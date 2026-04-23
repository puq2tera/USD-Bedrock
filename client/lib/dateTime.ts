export function parseDateTime(value: unknown): Date | null {
  if (value == null) {
    return null;
  }

  if (value instanceof Date) {
    return Number.isNaN(value.getTime()) ? null : value;
  }

  const parseEpoch = (raw: number): Date | null => {
    if (!Number.isFinite(raw)) {
      return null;
    }

    const absRaw = Math.abs(raw);
    // APIs can return epoch values in seconds, milliseconds, microseconds, or nanoseconds.
    // Normalize by magnitude so we always create a local Date from milliseconds.
    let asMilliseconds = raw;
    if (absRaw >= 1_000_000_000_000_000_000) {
      asMilliseconds = raw / 1_000_000;
    } else if (absRaw >= 1_000_000_000_000_000) {
      asMilliseconds = raw / 1_000;
    } else if (absRaw < 1_000_000_000_000) {
      asMilliseconds = raw * 1000;
    }
    const date = new Date(asMilliseconds);
    return Number.isNaN(date.getTime()) ? null : date;
  };

  if (typeof value === "number") {
    return parseEpoch(value);
  }

  if (typeof value !== "string") {
    return null;
  }

  const trimmed = value.trim();
  if (!trimmed) {
    return null;
  }

  if (/^-?\d+(?:\.\d+)?$/.test(trimmed)) {
    return parseEpoch(Number(trimmed));
  }

  const parsed = Date.parse(trimmed);
  if (Number.isNaN(parsed)) {
    return null;
  }

  return new Date(parsed);
}

function isSameLocalDay(left: Date, right: Date): boolean {
  return left.getFullYear() === right.getFullYear()
    && left.getMonth() === right.getMonth()
    && left.getDate() === right.getDate();
}

export function formatDateTimeForDisplay(value: unknown, fallback = "Unknown"): string {
  const date = parseDateTime(value);
  if (!date) {
    return fallback;
  }

  const now = new Date();
  const timeLabel = new Intl.DateTimeFormat(undefined, {
    hour: "numeric",
    minute: "2-digit",
  }).format(date);

  if (isSameLocalDay(date, now)) {
    return timeLabel;
  }

  if (date.getFullYear() === now.getFullYear()) {
    const dateLabel = new Intl.DateTimeFormat(undefined, {
      weekday: "long",
      month: "long",
      day: "numeric",
    }).format(date);
    return `${dateLabel}, ${timeLabel}`;
  }

  const dateLabel = new Intl.DateTimeFormat(undefined, {
    weekday: "long",
    month: "long",
    day: "numeric",
    year: "numeric",
  }).format(date);
  return `${dateLabel}, ${timeLabel}`;
}
