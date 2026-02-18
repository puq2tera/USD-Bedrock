<?php

declare(strict_types=1);

namespace BedrockStarter\requests\framework;

use BedrockStarter\Bedrock;
use BedrockStarter\responses\framework\ArrayRouteResponse;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\ValidationException;

abstract class RouteBoundRequestBase implements RouteBoundRequest
{
    public static function pathMatches(string $path): bool
    {
        // Keep path matching centralized so every request type uses identical regex semantics.
        return preg_match(static::pathPattern(), $path) === 1;
    }

    public static function methodAllowed(string $method): bool
    {
        return in_array(strtoupper($method), static::normalizedAllowedMethods(), true);
    }

    public static function tryBind(string $method, string $path): ?self
    {
        // Method check first avoids doing potentially expensive regex work for unsupported verbs.
        if (!static::methodAllowed($method)) {
            return null;
        }

        if (preg_match(static::pathPattern(), $path, $matches) !== 1) {
            return null;
        }

        return static::bindFromRouteMatch($matches);
    }

    public static function bedrockCommand(): ?string
    {
        // Default for API-only request types; Bedrock-backed requests override this.
        return null;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        // Default pass-through response object for endpoints that do not need custom shaping.
        return new ArrayRouteResponse($bedrockResponse);
    }

    public function execute(): RouteResponse
    {
        $command = static::bedrockCommand();
        if ($command === null || $command === '') {
            throw new ValidationException('Route request is missing bedrock command', 500);
        }

        // All Bedrock-backed requests share this execution path; endpoint-specific shaping is
        // delegated to transformResponse().
        return $this->transformResponse(Bedrock::call($command, $this->toBedrockParams()));
    }

    /**
     * Constructs the typed request instance from regex route captures (+ Request helpers for body/query).
     * Called only after method/path ownership has already been verified by tryBind().
     *
     * @param array<string, mixed> $routeParams
     */
    abstract protected static function bindFromRouteMatch(array $routeParams): self;

    /**
     * @return array<int, string>
     */
    private static function normalizedAllowedMethods(): array
    {
        return array_values(array_unique(array_map(
            static fn(string $method): string => strtoupper($method),
            static::allowedMethods()
        )));
    }
}
