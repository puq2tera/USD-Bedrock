<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\CreateUserResponse;

final class CreateUserRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly string $email,
        private readonly string $firstName,
        private readonly string $lastName
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
        return 'CreateUser';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireString('email', 1, 256),
            Request::requireString('firstName', 1, Request::MAX_SIZE_SMALL),
            Request::requireString('lastName', 1, Request::MAX_SIZE_SMALL)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'email' => $this->email,
            'firstName' => $this->firstName,
            'lastName' => $this->lastName,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreateUserResponse($bedrockResponse);
    }
}
