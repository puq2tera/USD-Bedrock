<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\SubmitPollTextResponseResponse;

final class SubmitPollTextResponseRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)/responses$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $pollID,
        private readonly int $userID,
        private readonly string $textValue
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
        return 'SubmitPollTextResponse';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'pollID'),
            Request::requireInt('userID', 1),
            Request::requireString('textValue', 1, Request::MAX_SIZE_QUERY)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'pollID' => (string)$this->pollID,
            'userID' => (string)$this->userID,
            'textValue' => $this->textValue,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new SubmitPollTextResponseResponse($bedrockResponse);
    }
}
