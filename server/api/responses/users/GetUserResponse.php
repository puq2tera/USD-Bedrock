<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class GetUserResponse implements RouteResponse
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
<<<<<<< HEAD
            'displayName' => (string)($this->payload['displayName'] ?? ''),
=======
>>>>>>> origin/main
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
        ];
    }
}
