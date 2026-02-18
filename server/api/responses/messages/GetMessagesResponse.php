<?php

declare(strict_types=1);

namespace BedrockStarter\responses\messages;

use BedrockStarter\responses\framework\RouteResponse;
final class GetMessagesResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        if (isset($response['messages'])) {
            $decodedMessages = json_decode((string)$response['messages'], true);
            if (is_array($decodedMessages)) {
                $response['messages'] = $decodedMessages;
            }
        }

        return $response;
    }
}

