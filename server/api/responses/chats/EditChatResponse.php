<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class EditChatResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'title' => (string)($this->payload['title'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
