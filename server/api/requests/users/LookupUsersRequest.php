<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\users\LookupUsersResponse;

final class LookupUsersRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/users/lookup$#';
    private const ALLOWED_METHODS = ['POST'];

    /**
     * @param array<int, int> $userIDs
     */
    public function __construct(private readonly array $userIDs)
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
        return 'LookupUsers';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        Auth::requireAuthenticatedUserID();

        $rawUserIDs = Request::requireJsonArray('userIDs', 1, 500);
        $normalizedUserIDs = [];
        foreach ($rawUserIDs as $rawUserID) {
            if (!is_int($rawUserID) && !is_string($rawUserID)) {
                throw new ValidationException('Invalid parameter: userIDs', 400);
            }

            $userID = filter_var((string)$rawUserID, FILTER_VALIDATE_INT);
            if ($userID === false || $userID <= 0) {
                throw new ValidationException('Invalid parameter: userIDs', 400);
            }

            $normalizedUserIDs[] = $userID;
        }

        return new self(array_values(array_unique($normalizedUserIDs)));
    }

    public function toBedrockParams(): array
    {
        return [
            'userIDs' => json_encode($this->userIDs, JSON_THROW_ON_ERROR),
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new LookupUsersResponse($bedrockResponse);
    }
}
