<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class GetChatMessagesResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = [
            'resultCount' => (string)($this->payload['resultCount'] ?? '0'),
            'format' => (string)($this->payload['format'] ?? ''),
            'messages' => [],
        ];

        if (isset($this->payload['nextBeforeMessageID'])) {
            $response['nextBeforeMessageID'] = (string)$this->payload['nextBeforeMessageID'];
        }

        if (isset($this->payload['messages'])) {
            $decodedMessages = json_decode((string)$this->payload['messages'], true);
            if (is_array($decodedMessages)) {
                $response['messages'] = $decodedMessages;
            }
        }

        return $response;
    }
}
