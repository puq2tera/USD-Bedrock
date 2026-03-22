<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\LoginUserResponse;

final class LoginUserRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/auth/login$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly string $email,
        private readonly string $password
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
        return 'LoginUser';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireString('email', 1, 256),
            Request::requireString('password', 8, 128)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'email' => $this->email,
            'password' => $this->password,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new LoginUserResponse($bedrockResponse);
    }
}
