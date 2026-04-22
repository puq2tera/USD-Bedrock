<?php

declare(strict_types=1);

namespace BedrockStarter\requests\account;

use BedrockStarter\Auth;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\account\GetAccountResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class GetAccountRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/account$#';
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
        return new self(Auth::requireAuthenticatedUserID());
    }

    public function toBedrockParams(): array
    {
        return ['userID' => (string)$this->userID];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetAccountResponse($bedrockResponse);
    }
}
