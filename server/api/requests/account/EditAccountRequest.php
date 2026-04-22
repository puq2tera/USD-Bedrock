<?php

declare(strict_types=1);

namespace BedrockStarter\requests\account;

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\account\EditAccountResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class EditAccountRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/account$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $userID,
        private readonly ?string $email,
        private readonly ?string $firstName,
        private readonly ?string $lastName,
        private readonly ?string $displayName
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
        $email = Request::getOptionalString('email', 1, 256);
        $firstName = Request::getOptionalString('firstName', 1, Request::MAX_SIZE_SMALL);
        $lastName = Request::getOptionalString('lastName', 1, Request::MAX_SIZE_SMALL);
        $displayName = Request::getOptionalString('displayName', 1, 511);

        if ($email === null && $firstName === null && $lastName === null && $displayName === null) {
            throw new ValidationException('Missing required parameter: email, firstName, lastName, or displayName', 400);
        }

        return new self(
            Auth::requireAuthenticatedUserID(),
            $email,
            $firstName,
            $lastName,
            $displayName
        );
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
        if ($this->displayName !== null) {
            $params['displayName'] = $this->displayName;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditAccountResponse($bedrockResponse);
    }
}
