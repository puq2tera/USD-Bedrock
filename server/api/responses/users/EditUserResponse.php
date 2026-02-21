<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class EditUserResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'userID' => (string)($this->payload['userID'] ?? ''),
            'email' => (string)($this->payload['email'] ?? ''),
            'firstName' => (string)($this->payload['firstName'] ?? ''),
            'lastName' => (string)($this->payload['lastName'] ?? ''),
            'displayName' => (string)($this->payload['displayName'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
