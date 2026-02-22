<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class GetPollResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        if (isset($response['options']) && is_string($response['options'])) {
            $decoded = json_decode($response['options'], true);
            if (is_array($decoded)) {
                $response['options'] = $decoded;
            }
        }

        if (isset($response['responses']) && is_string($response['responses'])) {
            $decoded = json_decode($response['responses'], true);
            if (is_array($decoded)) {
                $response['responses'] = $decoded;
            }
        }

        return $response;
    }
}
