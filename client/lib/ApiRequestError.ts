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
