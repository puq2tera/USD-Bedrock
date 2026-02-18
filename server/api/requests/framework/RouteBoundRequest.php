<?php

declare(strict_types=1);

namespace BedrockStarter\requests\framework;

use BedrockStarter\responses\framework\RouteResponse;

interface RouteBoundRequest
{
    // Regex pattern describing the route path this request class owns.
    // Kept static so routing/method checks can happen without object construction.
    public static function pathPattern(): string;

    /**
     * HTTP methods this request class accepts for its path.
     * Used to build deterministic 405 responses when path matches but method does not.
     *
     * @return array<int, string>
     */
    public static function allowedMethods(): array;

    // True when the incoming path belongs to this request type.
    public static function pathMatches(string $path): bool;

    // True when this request type supports the incoming HTTP method.
    public static function methodAllowed(string $method): bool;

    // Binds raw route/body/query input into a typed request object.
    // Returns null when method/path do not belong to this request type.
    public static function tryBind(string $method, string $path): ?self;

    // Name of the Bedrock command this request maps to.
    // Return null for requests that are handled entirely in API layer (for example, /api/status).
    public static function bedrockCommand(): ?string;

    // Converts typed request properties into Bedrock command parameters.
    public function toBedrockParams(): array;

    // Converts raw Bedrock output into a typed API response object.
    // Override for endpoint-specific decoding/normalization.
    public function transformResponse(array $bedrockResponse): RouteResponse;

    // Executes the request and returns a typed response model ready for JSON serialization.
    public function execute(): RouteResponse;
}
