<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;
final class DeletePollResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'pollID' => (string)($this->payload['pollID'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];
    }
}
