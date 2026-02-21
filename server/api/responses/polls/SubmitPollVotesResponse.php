<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class SubmitPollVotesResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        if (isset($response['optionIDs']) && is_string($response['optionIDs'])) {
            $decoded = json_decode($response['optionIDs'], true);
            if (is_array($decoded)) {
                $response['optionIDs'] = $decoded;
            }
        }

        return $response;
    }
}
