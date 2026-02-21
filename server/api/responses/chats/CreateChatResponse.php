<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class CreateChatResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'createdByUserID' => (string)($this->payload['createdByUserID'] ?? ''),
            'title' => (string)($this->payload['title'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
