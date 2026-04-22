<?php

declare(strict_types=1);

namespace BedrockStarter\responses\users;

use BedrockStarter\responses\framework\RouteResponse;

final class LookupUsersResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = [
            'resultCount' => (string)($this->payload['resultCount'] ?? '0'),
            'users' => [],
        ];

        if (isset($this->payload['users'])) {
            $decodedUsers = json_decode((string)$this->payload['users'], true);
            if (is_array($decodedUsers)) {
                $response['users'] = $decodedUsers;
            }
        }

        return $response;
    }
}
