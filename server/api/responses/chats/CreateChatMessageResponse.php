<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class CreateChatMessageResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'messageID' => (string)($this->payload['messageID'] ?? ''),
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'userID' => (string)($this->payload['userID'] ?? ''),
            'body' => (string)($this->payload['body'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
            'updatedAt' => (string)($this->payload['updatedAt'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
