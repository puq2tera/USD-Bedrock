<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class GetUserLookupResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'userID' => (string)($this->payload['userID'] ?? ''),
            'firstName' => (string)($this->payload['firstName'] ?? ''),
            'lastName' => (string)($this->payload['lastName'] ?? ''),
            'displayName' => (string)($this->payload['displayName'] ?? ''),
        ];
    }
}
