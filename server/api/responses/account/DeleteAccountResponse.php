<?php

declare(strict_types=1);

namespace BedrockStarter\responses\account;

use BedrockStarter\responses\framework\RouteResponse;

final class DeleteAccountResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'userID' => (string)($this->payload['userID'] ?? ''),
            'status' => (string)($this->payload['status'] ?? ''),
        ];
    }
}
