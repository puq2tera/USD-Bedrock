import { authFetch } from "../auth";
import { ApiRequestError, getApiError, parseApiErrorResponse } from "../ApiRequestError";
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
    throw parseApiErrorResponse(data, response.status);
  }

  return data as T;
}
