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

        if (isset($response['options'])) {
            $decoded = json_decode((string)$response['options'], true);
            if (is_array($decoded)) {
                $response['options'] = $decoded;
            }
        }

        return $response;
    }
}

