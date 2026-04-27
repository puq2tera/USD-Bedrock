import TypescriptUtils from "../TypescriptUtils";
import { request } from "./http";
import { normalizeUserIdentity, parseUserIdentity } from "./parsers";
import { cacheIdentity } from "./identity";
import type { AccountProfile, UpdateAccountInput } from "./types";

function parseAccount(data: Record<string, unknown>): AccountProfile {
  const identity = parseUserIdentity(data);
  if (!identity) {
    throw new Error("Invalid account response");
  }

  const normalized = normalizeUserIdentity(identity);
  cacheIdentity(normalized);

  return {
    ...normalized,
    email: TypescriptUtils.parseString(data.email) ?? "",
    createdAt: TypescriptUtils.parseString(data.createdAt) ?? "",
  };
}

export async function getAccount(): Promise<AccountProfile> {
  const data = await request<Record<string, unknown>>("/api/account");
  return parseAccount(data);
}

export async function updateAccount(input: UpdateAccountInput): Promise<AccountProfile> {
  const payload: Record<string, string> = {};
  if (!TypescriptUtils.isNullOrWhiteSpace(input.email)) {
    payload.email = TypescriptUtils.parseString(input.email)?.trim() ?? "";
  }
  if (!TypescriptUtils.isNullOrWhiteSpace(input.firstName)) {
    payload.firstName = TypescriptUtils.parseString(input.firstName)?.trim() ?? "";
  }
  if (!TypescriptUtils.isNullOrWhiteSpace(input.lastName)) {
    payload.lastName = TypescriptUtils.parseString(input.lastName)?.trim() ?? "";
  }
  if (!TypescriptUtils.isNullOrWhiteSpace(input.displayName)) {
    payload.displayName = TypescriptUtils.parseString(input.displayName)?.trim() ?? "";
  }

  if (Object.keys(payload).length < 1) {
    throw new Error("At least one field is required");
  }

  const data = await request<Record<string, unknown>>("/api/account", {
    method: "PUT",
    body: JSON.stringify(payload),
  });

  return parseAccount(data);
}

export async function deleteAccount(): Promise<void> {
  await request("/api/account", { method: "DELETE" });
}
