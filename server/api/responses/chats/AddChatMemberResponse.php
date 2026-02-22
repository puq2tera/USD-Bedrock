<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class AddChatMemberResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'userID' => (string)($this->payload['userID'] ?? ''),
            'role' => (string)($this->payload['role'] ?? ''),
            'joinedAt' => (string)($this->payload['joinedAt'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
