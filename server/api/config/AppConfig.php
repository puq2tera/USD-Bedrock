<?php

declare(strict_types=1);

namespace BedrockStarter\config;

final class AppConfig
{
    public const int POLL_MIN_OPTIONS = 2;
    public const int POLL_MAX_OPTIONS = 20;
    public const int POLL_REQUEST_MAX_OPTIONS = self::POLL_MAX_OPTIONS;
    public const string DEFAULT_JWT_ISSUER = 'bedrock-starter';
    public const string DEFAULT_JWT_AUDIENCE = 'bedrock-mobile';
    public const int DEFAULT_ACCESS_TOKEN_TTL_SECONDS = 15 * 60;
    public const int DEFAULT_REFRESH_TOKEN_TTL_SECONDS = 30 * 24 * 60 * 60;

    public static function jwtSecret(): string
    {
        return self::requireStringEnv('BEDROCK_API_JWT_SECRET');
    }

    public static function jwtIssuer(): string
    {
        return self::optionalStringEnv('BEDROCK_API_JWT_ISSUER') ?? self::DEFAULT_JWT_ISSUER;
    }

    public static function jwtAudience(): string
    {
        return self::optionalStringEnv('BEDROCK_API_JWT_AUDIENCE') ?? self::DEFAULT_JWT_AUDIENCE;
    }

    public static function accessTokenTtlSeconds(): int
    {
        return self::optionalPositiveIntEnv('BEDROCK_API_ACCESS_TOKEN_TTL_SECONDS') ?? self::DEFAULT_ACCESS_TOKEN_TTL_SECONDS;
    }

    public static function refreshTokenTtlSeconds(): int
    {
        return self::optionalPositiveIntEnv('BEDROCK_API_REFRESH_TOKEN_TTL_SECONDS') ?? self::DEFAULT_REFRESH_TOKEN_TTL_SECONDS;
    }

    private static function requireStringEnv(string $name): string
    {
        $value = self::optionalStringEnv($name);
        if ($value === null) {
            throw new \RuntimeException("Missing required configuration: {$name}");
        }

        return $value;
    }

    private static function optionalStringEnv(string $name): ?string
    {
        $value = getenv($name);
        if (!is_string($value)) {
            return null;
        }

        $trimmed = trim($value);
        return $trimmed === '' ? null : $trimmed;
    }

    private static function optionalPositiveIntEnv(string $name): ?int
    {
        $value = self::optionalStringEnv($name);
        if ($value === null) {
            return null;
        }

        $parsed = filter_var($value, FILTER_VALIDATE_INT);
        if ($parsed === false || $parsed <= 0) {
            throw new \RuntimeException("Invalid configuration: {$name}");
        }

        return (int)$parsed;
    }
}
