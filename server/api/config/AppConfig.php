<?php

declare(strict_types=1);

namespace BedrockStarter\config;

final class AppConfig
{
    public const int POLL_MIN_OPTIONS = 2;
    public const int POLL_MAX_OPTIONS = 20;
    public const int POLL_REQUEST_MAX_OPTIONS = self::POLL_MAX_OPTIONS;
}
