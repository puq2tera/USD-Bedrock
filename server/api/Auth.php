<?php

declare(strict_types=1);

namespace BedrockStarter;

use BedrockStarter\config\AppConfig;

final class Auth
{
    /**
     * @param array<string, mixed> $claimOverrides
     */
    public static function issueAccessToken(int $userID, int $sessionID, array $claimOverrides = []): string
    {
        $now = time();
        $payload = array_merge([
            'sub' => $userID,
            'sid' => $sessionID,
            'iat' => $now,
            'exp' => $now + AppConfig::accessTokenTtlSeconds(),
            'iss' => AppConfig::jwtIssuer(),
            'aud' => AppConfig::jwtAudience(),
            'typ' => 'access',
        ], $claimOverrides);

        return self::encodeHS256($payload, AppConfig::jwtSecret());
    }

    public static function generateRefreshToken(): string
    {
        return self::b64UrlEncode(random_bytes(32));
    }

    public static function hashRefreshToken(string $refreshToken): string
    {
        return hash('sha256', $refreshToken);
    }

    public static function requireAuthenticatedContext(): AuthContext
    {
        $token = self::extractBearerToken();
        if ($token === null) {
            throw new ValidationException('Unauthorized', 401);
        }

        return self::validateAccessToken($token);
    }

    public static function requireAuthenticatedUserID(): int
    {
        return self::requireAuthenticatedContext()->userID;
    }

    public static function extractUserAgent(): ?string
    {
        $userAgent = $_SERVER['HTTP_USER_AGENT'] ?? null;
        if (!is_string($userAgent)) {
            return null;
        }

        $trimmed = trim($userAgent);
        return $trimmed === '' ? null : $trimmed;
    }

    public static function accessTokenExpiresAt(string $token): int
    {
        $parts = explode('.', $token);
        if (count($parts) !== 3) {
            throw new ValidationException('Unauthorized', 401);
        }

        $payloadJson = self::b64UrlDecode($parts[1]);
        $payload = is_string($payloadJson) ? json_decode($payloadJson, true) : null;
        $exp = is_array($payload) ? (int)($payload['exp'] ?? 0) : 0;
        if ($exp <= 0) {
            throw new ValidationException('Unauthorized', 401);
        }

        return $exp;
    }

    private static function extractBearerToken(): ?string
    {
        $header = '';
        if (isset($_SERVER['HTTP_AUTHORIZATION'])) {
            $header = (string)$_SERVER['HTTP_AUTHORIZATION'];
        } elseif (function_exists('getallheaders')) {
            $headers = getallheaders();
            $header = (string)($headers['Authorization'] ?? $headers['authorization'] ?? '');
        }

        if ($header === '' || preg_match('/^Bearer\s+(.+)$/i', trim($header), $matches) !== 1) {
            return null;
        }

        $token = trim((string)$matches[1]);
        return $token === '' ? null : $token;
    }

    private static function validateAccessToken(string $jwt): AuthContext
    {
        $payload = self::decodeHS256($jwt, AppConfig::jwtSecret());
        if ($payload === null) {
            throw new ValidationException('Unauthorized', 401);
        }

        $userID = isset($payload['sub']) ? (int)$payload['sub'] : 0;
        $sessionID = isset($payload['sid']) ? (int)$payload['sid'] : 0;
        $issuer = (string)($payload['iss'] ?? '');
        $audience = (string)($payload['aud'] ?? '');
        $type = (string)($payload['typ'] ?? '');
        $expiresAt = isset($payload['exp']) ? (int)$payload['exp'] : 0;

        if ($userID <= 0 ||
            $sessionID <= 0 ||
            $issuer !== AppConfig::jwtIssuer() ||
            $audience !== AppConfig::jwtAudience() ||
            $type !== 'access' ||
            $expiresAt <= 0 ||
            time() >= $expiresAt) {
            throw new ValidationException('Unauthorized', 401);
        }

        return new AuthContext($userID, $sessionID, $payload);
    }

    /**
     * @param array<string, mixed> $payload
     */
    private static function encodeHS256(array $payload, string $secret): string
    {
        $header = ['alg' => 'HS256', 'typ' => 'JWT'];
        $headerB64 = self::b64UrlEncode((string)json_encode($header, JSON_THROW_ON_ERROR));
        $payloadB64 = self::b64UrlEncode((string)json_encode($payload, JSON_THROW_ON_ERROR));
        $sig = hash_hmac('sha256', "{$headerB64}.{$payloadB64}", $secret, true);
        $sigB64 = self::b64UrlEncode($sig);
        return "{$headerB64}.{$payloadB64}.{$sigB64}";
    }

    /**
     * @return array<string, mixed>|null
     */
    private static function decodeHS256(string $jwt, string $secret): ?array
    {
        $parts = explode('.', $jwt);
        if (count($parts) !== 3) {
            return null;
        }

        [$headerB64, $payloadB64, $sigB64] = $parts;
        $headerJson = self::b64UrlDecode($headerB64);
        $payloadJson = self::b64UrlDecode($payloadB64);
        $sig = self::b64UrlDecode($sigB64);
        if ($headerJson === null || $payloadJson === null || $sig === null) {
            return null;
        }

        $header = json_decode($headerJson, true);
        $payload = json_decode($payloadJson, true);
        if (!is_array($header) || !is_array($payload) || ($header['alg'] ?? '') !== 'HS256') {
            return null;
        }

        $expectedSig = hash_hmac('sha256', "{$headerB64}.{$payloadB64}", $secret, true);
        if (!hash_equals($expectedSig, $sig)) {
            return null;
        }

        return $payload;
    }

    private static function b64UrlEncode(string $raw): string
    {
        return rtrim(strtr(base64_encode($raw), '+/', '-_'), '=');
    }

    private static function b64UrlDecode(string $raw): ?string
    {
        $padLen = (4 - (strlen($raw) % 4)) % 4;
        $decoded = base64_decode(strtr($raw . str_repeat('=', $padLen), '-_', '+/'), true);
        return is_string($decoded) ? $decoded : null;
    }
}
