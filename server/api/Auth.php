<?php

declare(strict_types=1);

namespace BedrockStarter;

use BedrockStarter\config\AppConfig;

final class Auth
{
    public static function issueToken(int $userID): string
    {
        $now = time();
        $payload = [
            'sub' => $userID,
            'iat' => $now,
            'exp' => $now + AppConfig::JWT_TTL_SECONDS,
        ];

        return self::encodeHS256($payload, AppConfig::jwtSecret());
    }

    public static function requireAuthenticatedUserID(): int
    {
        $token = self::extractBearerToken();
        if ($token === null) {
            throw new ValidationException('Unauthorized', 401);
        }

        $decoded = self::decodeHS256($token, AppConfig::jwtSecret());
        if ($decoded === null) {
            throw new ValidationException('Unauthorized', 401);
        }

        $userID = isset($decoded['sub']) ? (int)$decoded['sub'] : 0;
        if ($userID <= 0) {
            throw new ValidationException('Unauthorized', 401);
        }

        return $userID;
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

        if ($header === '') {
            return null;
        }

        if (!preg_match('/^Bearer\\s+(.+)$/i', trim($header), $matches)) {
            return null;
        }

        return trim((string)$matches[1]);
    }

    /**
     * @param array<string, mixed> $payload
     */
    private static function encodeHS256(array $payload, string $secret): string
    {
        $header = ['alg' => 'HS256', 'typ' => 'JWT'];
        $headerB64 = self::b64UrlEncode((string)json_encode($header));
        $payloadB64 = self::b64UrlEncode((string)json_encode($payload));
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
        if (!is_array($header) || !is_array($payload)) {
            return null;
        }
        if (($header['alg'] ?? '') !== 'HS256') {
            return null;
        }

        $expectedSig = hash_hmac('sha256', "{$headerB64}.{$payloadB64}", $secret, true);
        if (!hash_equals($expectedSig, $sig)) {
            return null;
        }

        $exp = isset($payload['exp']) ? (int)$payload['exp'] : 0;
        if ($exp <= 0 || time() >= $exp) {
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
