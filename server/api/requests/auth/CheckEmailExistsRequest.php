<?php

declare(strict_types=1);

namespace BedrockStarter\requests\auth;

use BedrockStarter\Bedrock;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\ArrayRouteResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class CheckEmailExistsRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/auth/email-exists$#';
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
        return null;
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(Request::requireString('email', 6, 254));
    }

    public function toBedrockParams(): array
    {
        return ['email' => $this->email];
    }

    public function execute(): RouteResponse
    {
        try {
            Bedrock::call('LookupUserByEmail', $this->toBedrockParams());
            return new ArrayRouteResponse(['exists' => true]);
        } catch (ValidationException $exception) {
            if ($exception->getStatusCode() === 404) {
                return new ArrayRouteResponse(['exists' => false]);
            }

            throw $exception;
        }
    }
}
