<?php

declare(strict_types=1);

namespace BedrockStarter\responses\chats;

use BedrockStarter\responses\framework\RouteResponse;

final class ListChatMembersResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = [
            'resultCount' => (string)($this->payload['resultCount'] ?? '0'),
            'format' => (string)($this->payload['format'] ?? ''),
            'members' => [],
        ];

        if (isset($this->payload['members'])) {
            $decodedMembers = json_decode((string)$this->payload['members'], true);
            if (is_array($decodedMembers)) {
                $response['members'] = $decodedMembers;
            }
        }

        return $response;
    }
}
