<?php

declare(strict_types=1);

namespace BedrockStarter\responses\auth;

use BedrockStarter\responses\framework\RouteResponse;

final class LogoutResponse implements RouteResponse
{
    public function toArray(): array
    {
        return ['ok' => true];
    }
}
