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
<<<<<<< HEAD:server/api/responses/chats/DeleteChatMessageResponse.php
            'chatID' => (string)($this->payload['chatID'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
=======
            'userID' => (string)($this->payload['userID'] ?? ''),
            'name' => (string)($this->payload['name'] ?? ''),
            'message' => (string)($this->payload['message'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
>>>>>>> origin/main:server/api/responses/messages/CreateMessageResponse.php
        ];
    }
}
