<?php

declare(strict_types=1);

namespace BedrockStarter\requests\framework;

use BedrockStarter\ValidationException;

final class RouteBinder
{
    /**
     * Attempts to bind the incoming request into one of the provided request classes.
     * Returns the first successful bind (request type order is therefore significant).
     *
     * @param array<int, class-string<RouteBoundRequest>> $requestTypes
     */
    public static function tryBind(string $method, string $path, array $requestTypes): ?RouteBoundRequest
    {
        // Ordering is intentional: first matching request type wins when patterns overlap.
        foreach ($requestTypes as $requestType) {
            self::assertRouteBoundRequest($requestType);

            $bound = $requestType::tryBind($method, $path);
            if ($bound !== null) {
                return $bound;
            }
        }

        return null;
    }

    /**
     * Collects all methods supported by request classes whose path pattern matches the input path.
     * Used by router to generate accurate 405 payloads.
     *
     * @param array<int, class-string<RouteBoundRequest>> $requestTypes
     * @return array<int, string>
     */
    public static function allowedMethodsForPath(string $path, array $requestTypes): array
    {
        // Aggregate allowed methods across all types that match the path so callers can return
        // a complete and stable 405 payload.
        $methods = [];
        foreach ($requestTypes as $requestType) {
            self::assertRouteBoundRequest($requestType);
            if ($requestType::pathMatches($path)) {
                $methods = array_merge($methods, $requestType::allowedMethods());
            }
        }

        $methods = array_values(array_unique(array_map(
            static fn(string $method): string => strtoupper($method),
            $methods
        )));
        sort($methods);
        return $methods;
    }

    /**
     * @param class-string $requestType
     */
    private static function assertRouteBoundRequest(string $requestType): void
    {
        if (!is_subclass_of($requestType, RouteBoundRequest::class)) {
            throw new ValidationException("Invalid route binding class: {$requestType}", 500);
        }
    }
}
