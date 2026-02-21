<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\EditUserResponse;
use BedrockStarter\ValidationException;

final class EditUserRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users/(?P<userID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $userID,
        private readonly ?string $email,
        private readonly ?string $firstName,
<<<<<<< HEAD
        private readonly ?string $lastName,
        private readonly ?string $displayName
=======
        private readonly ?string $lastName
>>>>>>> origin/main
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
        return 'EditUser';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $userID = Request::requireRouteInt($routeParams, 'userID');
        $email = Request::getOptionalString('email', 1, 256);
        $firstName = Request::getOptionalString('firstName', 1, Request::MAX_SIZE_SMALL);
        $lastName = Request::getOptionalString('lastName', 1, Request::MAX_SIZE_SMALL);
<<<<<<< HEAD
        $displayName = Request::getOptionalString('displayName', 1, 511);

        if ($email === null && $firstName === null && $lastName === null && $displayName === null) {
            throw new ValidationException('Missing required parameter: email, firstName, lastName, or displayName', 400);
        }

        return new self($userID, $email, $firstName, $lastName, $displayName);
=======

        if ($email === null && $firstName === null && $lastName === null) {
            throw new ValidationException('Missing required parameter: email, firstName, or lastName', 400);
        }

        return new self($userID, $email, $firstName, $lastName);
>>>>>>> origin/main
    }

    public function toBedrockParams(): array
    {
        $params = ['userID' => (string)$this->userID];

        if ($this->email !== null) {
            $params['email'] = $this->email;
        }
        if ($this->firstName !== null) {
            $params['firstName'] = $this->firstName;
        }
        if ($this->lastName !== null) {
            $params['lastName'] = $this->lastName;
        }
<<<<<<< HEAD
        if ($this->displayName !== null) {
            $params['displayName'] = $this->displayName;
        }
=======
>>>>>>> origin/main

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditUserResponse($bedrockResponse);
    }
}
