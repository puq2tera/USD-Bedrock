<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;
final class CreatePollResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'pollID' => (string)($this->payload['pollID'] ?? ''),
            'question' => (string)($this->payload['question'] ?? ''),
            'optionCount' => (string)($this->payload['optionCount'] ?? '0'),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
        ];
    }
}
