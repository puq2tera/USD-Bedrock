<?php

declare(strict_types=1);

namespace BedrockStarter\responses\auth;

use BedrockStarter\responses\framework\RouteResponse;

final class RefreshTokenResponse implements RouteResponse
{
    public function __construct(
        private readonly string $accessToken,
        private readonly string $refreshToken
    ) {
    }

    public function toArray(): array
    {
        return [
            'accessToken' => $this->accessToken,
            'refreshToken' => $this->refreshToken,
        ];
    }
}
