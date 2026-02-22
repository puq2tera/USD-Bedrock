<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\RemoveChatMemberResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class RemoveChatMemberRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/members/(?P<userID>\d+)$#';
    private const ALLOWED_METHODS = ['DELETE'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $actingUserID,
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
        return 'RemoveChatMember';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('actingUserID', 1),
            Request::requireRouteInt($routeParams, 'userID')
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'actingUserID' => (string)$this->actingUserID,
            'userID' => (string)$this->userID,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new RemoveChatMemberResponse($bedrockResponse);
    }
}
