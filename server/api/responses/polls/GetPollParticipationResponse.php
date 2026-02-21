<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class GetPollParticipationResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        foreach (['eligibleUserIDs', 'votedUserIDs', 'notVotedUserIDs'] as $field) {
            if (isset($response[$field]) && is_string($response[$field])) {
                $decoded = json_decode($response[$field], true);
                if (is_array($decoded)) {
                    $response[$field] = $decoded;
                }
            }
        }

        return $response;
    }
}
