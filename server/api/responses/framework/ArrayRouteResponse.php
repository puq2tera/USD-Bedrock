<?php

declare(strict_types=1);

namespace BedrockStarter\responses\framework;

final class ArrayRouteResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return $this->payload;
    }
}

