<?php

declare(strict_types=1);

namespace BedrockStarter\responses\system;

use BedrockStarter\responses\framework\RouteResponse;
final class HelloWorldResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
        return [
            'message' => (string)($this->payload['message'] ?? ''),
            'from' => (string)($this->payload['from'] ?? ''),
            'timestamp' => (string)($this->payload['timestamp'] ?? ''),
            'plugin_name' => (string)($this->payload['plugin_name'] ?? ''),
            'plugin_version' => (string)($this->payload['plugin_version'] ?? ''),
        ];
    }
}
