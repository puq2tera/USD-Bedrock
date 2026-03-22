<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\Auth;
use BedrockStarter\responses\framework\RouteResponse;

final class LoginUserResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $userID = isset($this->payload['userID']) ? (int)$this->payload['userID'] : 0;
        if ($userID <= 0) {
            return $this->payload;
        }

        return [
            'token' => Auth::issueToken($userID),
            'user' => [
                'userID' => (string)$this->payload['userID'],
                'email' => (string)($this->payload['email'] ?? ''),
                'firstName' => (string)($this->payload['firstName'] ?? ''),
                'lastName' => (string)($this->payload['lastName'] ?? ''),
                'displayName' => (string)($this->payload['displayName'] ?? ''),
            ],
        ];
    }
}
