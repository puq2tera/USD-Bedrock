import { authFetch } from "../auth";
import { ApiRequestError, getApiError } from "../ApiRequestError";
import { API_BASE } from "./constants";

export { ApiRequestError, getApiError };

export async function request<T>(path: string, options?: RequestInit): Promise<T> {
  const headers: Record<string, string> = {
    "Content-Type": "application/json",
    ...(options?.headers as Record<string, string> | undefined),
  };

  const response = await authFetch(`${API_BASE}${path}`, {
    ...options,
    headers,
  });

  const data = await response.json();
  if (!response.ok) {
    const message = typeof data?.error === "string" && data.error.trim().length > 0
      ? data.error
      : `Request failed with status ${response.status}`;
    const field = typeof data?.parameter === "string" && data.parameter.trim().length > 0
      ? data.parameter
      : null;
    throw new ApiRequestError(message, response.status, field);
  }

  return data as T;
}
