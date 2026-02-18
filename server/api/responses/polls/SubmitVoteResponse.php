<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;
final class SubmitVoteResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'voteID' => (string)($this->payload['voteID'] ?? ''),
            'pollID' => (string)($this->payload['pollID'] ?? ''),
            'optionID' => (string)($this->payload['optionID'] ?? ''),
            'createdAt' => (string)($this->payload['createdAt'] ?? ''),
        ];
    }
}
