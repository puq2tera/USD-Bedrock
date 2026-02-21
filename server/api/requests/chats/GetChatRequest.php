<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\GetChatResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class GetChatRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $userID
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
        return 'GetChat';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('userID', 1)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'userID' => (string)$this->userID,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetChatResponse($bedrockResponse);
    }
}
