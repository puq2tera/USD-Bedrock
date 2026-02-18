<?php

declare(strict_types=1);

namespace BedrockStarter\responses\system;

use BedrockStarter\responses\framework\RouteResponse;
final class StatusResponse implements RouteResponse
{
    public function __construct(
        private readonly string $service,
        private readonly string $timestamp,
        private readonly string $phpVersion
    ) {
    }

    public function toArray(): array
    {
        return [
            'status' => 'ok',
            'service' => $this->service,
            'timestamp' => $this->timestamp,
            'php_version' => $this->phpVersion,
        ];
    }
}

