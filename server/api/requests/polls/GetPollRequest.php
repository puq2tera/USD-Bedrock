<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\polls\GetPollResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class GetPollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(private readonly int $pollID)
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
        return 'GetPoll';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(Request::requireRouteInt($routeParams, 'pollID'));
    }

    public function toBedrockParams(): array
    {
        return ['pollID' => (string)$this->pollID];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetPollResponse($bedrockResponse);
    }
}
