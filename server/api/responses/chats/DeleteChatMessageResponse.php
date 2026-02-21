<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class DeleteChatMessageResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'messageID' => (string)($this->payload['messageID'] ?? ''),
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
