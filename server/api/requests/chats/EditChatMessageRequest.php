<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\chats\EditChatMessageResponse;

final class EditChatMessageRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/messages/(?P<messageID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $messageID,
        private readonly int $userID,
        private readonly string $body
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
        return 'EditChatMessage';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireRouteInt($routeParams, 'messageID'),
            Request::requireInt('userID', 1),
            Request::requireString('body', 1, Request::MAX_SIZE_QUERY)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'messageID' => (string)$this->messageID,
            'userID' => (string)$this->userID,
            'body' => $this->body,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditChatMessageResponse($bedrockResponse);
    }
}
