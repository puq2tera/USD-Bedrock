<?php

declare(strict_types=1);

namespace BedrockStarter\requests\system;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\system\HelloWorldResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class HelloWorldRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/hello$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(private readonly string $name)
    {
    }

    public static function pathPattern(): string
    {
        return self::PATH_PATTERN;
    }

    public static function allowedMethods(): array
    {
        return self::ALLOWED_METHODS;
    }

    public static function bedrockCommand(): ?string
    {
        return 'HelloWorld';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $name = Request::getOptionalString('name', 1, Request::MAX_SIZE_SMALL);
        return new self($name ?? 'World');
    }

    public function toBedrockParams(): array
    {
        return ['name' => $this->name];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new HelloWorldResponse($bedrockResponse);
    }
}
