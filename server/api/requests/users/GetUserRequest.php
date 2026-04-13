<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\GetUserResponse;

final class GetUserRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users/(?P<userID>\d+)$#';
    private const ALLOWED_METHODS = ['GET'];

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
        return 'GetUser';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $routeUserID = Request::requireRouteInt($routeParams, 'userID');
        $authContext = Auth::requireAuthenticatedContext();
        // Profile routes are intentionally self-only. Allowing a caller-controlled route ID here
        // would turn a valid bearer token into cross-user data access.
        if ($routeUserID !== $authContext->userID) {
            throw new ValidationException('Forbidden', 403);
        }

        return new self($routeUserID);
    }

    public function toBedrockParams(): array
    {
        return ['userID' => (string)$this->userID];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetUserResponse($bedrockResponse);
    }
}
