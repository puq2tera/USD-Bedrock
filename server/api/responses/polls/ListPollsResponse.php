<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class ListPollsResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        if (isset($response['polls']) && is_string($response['polls'])) {
            $decoded = json_decode($response['polls'], true);
            if (is_array($decoded)) {
                $response['polls'] = $decoded;
            }
        }

        return $response;
    }
}
