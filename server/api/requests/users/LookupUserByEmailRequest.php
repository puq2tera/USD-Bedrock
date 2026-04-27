<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\GetUserLookupResponse;

final class LookupUserByEmailRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users/by-email$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(private readonly string $email)
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
        return 'LookupUserByEmail';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        Auth::requireAuthenticatedContext();
        return new self(Request::requireString('email', 6, 254));
    }

    public function toBedrockParams(): array
    {
        return ['email' => $this->email];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetUserLookupResponse($bedrockResponse);
    }
}
