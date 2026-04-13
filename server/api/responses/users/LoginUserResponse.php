<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class LoginUserResponse implements RouteResponse
{
    public function __construct(
        private readonly string $accessToken,
        private readonly string $refreshToken,
        private readonly array $user
    )
    {
    }

    public function toArray(): array
    {
        return [
            'accessToken' => $this->accessToken,
            'refreshToken' => $this->refreshToken,
            'user' => $this->user,
        ];
    }
}
