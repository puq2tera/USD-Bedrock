<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\ListPollsResponse;

final class ListPollsRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/polls$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $requesterUserID,
        private readonly bool $includeClosed
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
        return 'ListPolls';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('requesterUserID', 1),
            Request::hasParam('includeClosed') ? Request::requireBool('includeClosed') : true
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'requesterUserID' => (string)$this->requesterUserID,
            'includeClosed' => $this->includeClosed ? 'true' : 'false',
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new ListPollsResponse($bedrockResponse);
    }
}
