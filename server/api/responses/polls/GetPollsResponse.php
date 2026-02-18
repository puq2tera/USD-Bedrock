<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class GetPollsResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        $response = $this->payload;

        if (isset($response['polls'])) {
            $decoded = json_decode((string)$response['polls'], true);
            if (is_array($decoded)) {
                $response['polls'] = $decoded;
            }
        }

        return $response;
    }
}
