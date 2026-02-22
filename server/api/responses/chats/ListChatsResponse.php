<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class ListChatsResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = [
            'resultCount' => (string)($this->payload['resultCount'] ?? '0'),
            'format' => (string)($this->payload['format'] ?? ''),
            'chats' => [],
        ];

        if (isset($this->payload['chats'])) {
            $decodedChats = json_decode((string)$this->payload['chats'], true);
            if (is_array($decodedChats)) {
                $response['chats'] = $decodedChats;
            }
        }

        return $response;
    }
}
