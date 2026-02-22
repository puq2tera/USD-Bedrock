<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\chats\DeleteChatMessageResponse;

final class DeleteChatMessageRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/messages/(?P<messageID>\d+)$#';
    private const ALLOWED_METHODS = ['DELETE'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $messageID,
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
        return 'DeleteChatMessage';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireRouteInt($routeParams, 'messageID'),
            Request::requireInt('userID', 1)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'messageID' => (string)$this->messageID,
            'userID' => (string)$this->userID,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new DeleteChatMessageResponse($bedrockResponse);
    }
}
