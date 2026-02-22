<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\AddChatMemberResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class AddChatMemberRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/members$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $actingUserID,
        private readonly int $userID,
        private readonly ?string $role
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
        return 'AddChatMember';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $role = Request::getOptionalString('role', 1, Request::MAX_SIZE_SMALL);
        if ($role !== null) {
            $role = ChatRole::requireKnown($role);
        }

        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('actingUserID', 1),
            Request::requireInt('userID', 1),
            $role
        );
    }

    public function toBedrockParams(): array
    {
        $params = [
            'chatID' => (string)$this->chatID,
            'actingUserID' => (string)$this->actingUserID,
            'userID' => (string)$this->userID,
        ];

        if ($this->role !== null) {
            $params['role'] = $this->role;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new AddChatMemberResponse($bedrockResponse);
    }
}
