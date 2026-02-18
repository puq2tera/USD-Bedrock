<?php

declare(strict_types=1);

namespace BedrockStarter\requests\system;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\system\StatusResponse;

final class StatusRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/status$#';
    private const ALLOWED_METHODS = ['GET'];

    public static function pathPattern(): string
    {
        return self::PATH_PATTERN;
    }

    public static function allowedMethods(): array
    {
        return self::ALLOWED_METHODS;
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self();
    }

    public function toBedrockParams(): array
    {
        return [];
    }

    public function execute(): RouteResponse
    {
        return new StatusResponse(
            'bedrock-starter-api',
            date('c'),
            PHP_VERSION
        );
    }
}

