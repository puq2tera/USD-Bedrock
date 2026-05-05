export class ApiRequestError extends Error {
  status: number;
  field: string | null;

  constructor(message: string, status: number, field?: string | null) {
    super(message);
    this.name = "ApiRequestError";
    this.status = status;
    this.field = field ?? null;
  }
}

export function getApiError(error: unknown): ApiRequestError {
  if (error instanceof ApiRequestError) {
    return error;
  }

  const fallbackMessage = error instanceof Error ? error.message : "Request failed";
  return new ApiRequestError(fallbackMessage, 500, null);
}

export function parseApiErrorResponse(data: unknown, status: number): ApiRequestError {
  const errorPayload = (typeof data === "object" && data !== null ? data : {}) as {
    error?: unknown;
    parameter?: unknown;
  };
  const parsedMessage = typeof errorPayload.error === "string" ? errorPayload.error.trim() : "";
  const parsedField = typeof errorPayload.parameter === "string" ? errorPayload.parameter.trim() : "";

  return new ApiRequestError(
    parsedMessage.length > 0 ? parsedMessage : `Request failed with status ${status}`,
    status,
    parsedField.length > 0 ? parsedField : null
  );
}
