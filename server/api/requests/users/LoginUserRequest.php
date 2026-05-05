<?php

declare(strict_types=1);

namespace BedrockStarter\requests\users;

use BedrockStarter\Auth;
use BedrockStarter\Bedrock;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
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
        return null;
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireString('email', 1, 256),
            // Accept any non-empty password here so incorrect credentials consistently surface as auth failures.
            Request::requireString('password', 1, 128)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'email' => $this->email,
            'password' => $this->password,
        ];
    }

    public function execute(): RouteResponse
    {
        $bedrockResponse = Bedrock::call('LoginUser', $this->toBedrockParams());
        $userID = isset($bedrockResponse['userID']) ? (int)$bedrockResponse['userID'] : 0;
        if ($userID <= 0) {
            throw new ValidationException('Unauthorized', 401);
        }

        $refreshToken = Auth::generateRefreshToken();
        $sessionResponse = Bedrock::call('CreateUserSession', [
            'userID' => (string)$userID,
            'refreshTokenHash' => Auth::hashRefreshToken($refreshToken),
            'expiresAt' => (string)$this->refreshTokenExpiresAt(),
            'userAgent' => Auth::extractUserAgent() ?? '',
        ]);
        $sessionID = isset($sessionResponse['sessionID']) ? (int)$sessionResponse['sessionID'] : 0;
        if ($sessionID <= 0) {
            throw new ValidationException('Unauthorized', 401);
        }

        return new LoginUserResponse(
            Auth::issueAccessToken($userID, $sessionID),
            $refreshToken,
            [
                'userID' => (string)$bedrockResponse['userID'],
                'email' => (string)($bedrockResponse['email'] ?? ''),
                'firstName' => (string)($bedrockResponse['firstName'] ?? ''),
                'lastName' => (string)($bedrockResponse['lastName'] ?? ''),
                'displayName' => (string)($bedrockResponse['displayName'] ?? ''),
            ]
        );
    }

    private function refreshTokenExpiresAt(): int
    {
        return (int)(microtime(true) * 1000000) + (\BedrockStarter\config\AppConfig::refreshTokenTtlSeconds() * 1000000);
    }
}
