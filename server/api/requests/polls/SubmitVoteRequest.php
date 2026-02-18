<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\SubmitVoteResponse;

final class SubmitVoteRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)/vote$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $pollID,
        private readonly int $optionID
    ) {
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
        return 'SubmitVote';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'pollID'),
            Request::requireInt('optionID', 1)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'pollID' => (string)$this->pollID,
            'optionID' => (string)$this->optionID,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new SubmitVoteResponse($bedrockResponse);
    }
}
