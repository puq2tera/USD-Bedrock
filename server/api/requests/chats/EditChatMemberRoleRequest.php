<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\EditChatMemberRoleResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class EditChatMemberRoleRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/members/(?P<userID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $actingUserID,
        private readonly int $userID,
        private readonly string $role
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
        return 'EditChatMemberRole';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('actingUserID', 1),
            Request::requireRouteInt($routeParams, 'userID'),
            ChatRole::requireKnown(Request::requireString('role', 1, Request::MAX_SIZE_SMALL))
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'chatID' => (string)$this->chatID,
            'actingUserID' => (string)$this->actingUserID,
            'userID' => (string)$this->userID,
            'role' => $this->role,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditChatMemberRoleResponse($bedrockResponse);
    }
}
