<?php

declare(strict_types=1);

namespace BedrockStarter\config;

final class AppConfig
{
    public const int POLL_MIN_OPTIONS = 2;
    public const int POLL_MAX_OPTIONS = 20;
    public const int POLL_REQUEST_MAX_OPTIONS = self::POLL_MAX_OPTIONS;
    public const int JWT_TTL_SECONDS = 60 * 60 * 24 * 7;

    public static function jwtSecret(): string
    {
        $secret = getenv('BEDROCK_API_JWT_SECRET');
        if (is_string($secret) && trim($secret) !== '') {
            return $secret;
        }

        return 'dev-only-change-me-before-prod';
    }
}
