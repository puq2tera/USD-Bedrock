<?php

declare(strict_types=1);

namespace BedrockStarter\responses\messages;

use BedrockStarter\responses\framework\RouteResponse;
final class CreateMessageResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'result' => (string)($this->payload['result'] ?? ''),
            'messageID' => (string)($this->payload['messageID'] ?? ''),
            'userID' => (string)($this->payload['userID'] ?? ''),
            'name' => (string)($this->payload['name'] ?? ''),
            'message' => (string)($this->payload['message'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
        ];
    }
}
