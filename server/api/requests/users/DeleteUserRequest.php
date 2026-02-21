<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\DeleteUserResponse;

final class DeleteUserRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users/(?P<userID>\d+)$#';
    private const ALLOWED_METHODS = ['DELETE'];

    public function __construct(private readonly int $userID)
    {
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
        return 'DeleteUser';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(Request::requireRouteInt($routeParams, 'userID'));
    }

    public function toBedrockParams(): array
    {
        return ['userID' => (string)$this->userID];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new DeleteUserResponse($bedrockResponse);
    }
}
