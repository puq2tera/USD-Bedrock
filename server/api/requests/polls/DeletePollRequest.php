<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\polls\DeletePollResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class DeletePollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)$#';
    private const ALLOWED_METHODS = ['DELETE'];

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
        return 'DeletePoll';
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
        return new DeletePollResponse($bedrockResponse);
    }
}
