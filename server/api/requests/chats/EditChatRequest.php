<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\EditChatResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class EditChatRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $userID,
        private readonly string $title
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
        return 'EditChat';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('userID', 1),
            Request::requireString('title', 1, Request::MAX_SIZE_SMALL)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'userID' => (string)$this->userID,
            'title' => $this->title,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditChatResponse($bedrockResponse);
    }
}
