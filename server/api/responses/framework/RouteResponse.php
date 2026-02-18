<?php

declare(strict_types=1);

namespace BedrockStarter\responses\framework;

interface RouteResponse
{
    // Encodes the canonical wire shape for API output.
    public function toArray(): array;
}
