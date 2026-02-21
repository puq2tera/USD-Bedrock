<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class DeleteUserResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'userID' => (string)($this->payload['userID'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
