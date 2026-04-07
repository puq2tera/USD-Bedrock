<?php

declare(strict_types=1);

namespace BedrockStarter\requests\auth;

use BedrockStarter\Auth;
use BedrockStarter\Bedrock;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\auth\LogoutResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class LogoutRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/auth/logout$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(private readonly string $refreshToken)
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
        Auth::requireAuthenticatedContext();
        return new self(Request::requireString('refreshToken', 1, Request::MAX_SIZE_QUERY));
    }

    public function toBedrockParams(): array
    {
        return [];
    }

    public function execute(): RouteResponse
    {
        $authContext = Auth::requireAuthenticatedContext();
        $session = $this->normalizeUnauthorized(static fn(): array => Bedrock::call('GetUserSessionByRefreshTokenHash', [
            'refreshTokenHash' => Auth::hashRefreshToken($this->refreshToken),
        ]));

        $sessionID = (int)($session['sessionID'] ?? 0);
        $userID = (int)($session['userID'] ?? 0);
        $expiresAt = (int)($session['expiresAt'] ?? 0);
        $revokedAt = isset($session['revokedAt']) ? (int)$session['revokedAt'] : null;
        $replacedBySessionID = isset($session['replacedBySessionID']) ? (int)$session['replacedBySessionID'] : null;

        if ($sessionID <= 0 ||
            $userID !== $authContext->userID ||
            $sessionID !== $authContext->sessionID ||
            $expiresAt <= (int)(microtime(true) * 1000000) ||
            $revokedAt !== null ||
            $replacedBySessionID !== null) {
            throw new ValidationException('Unauthorized', 401);
        }

        $this->normalizeUnauthorized(static fn(): array => Bedrock::call('RevokeUserSession', [
            'sessionID' => (string)$sessionID,
        ]));

        return new LogoutResponse();
    }

    /**
     * @param callable(): array<string, mixed> $fn
     * @return array<string, mixed>
     */
    private function normalizeUnauthorized(callable $fn): array
    {
        try {
            return $fn();
        } catch (ValidationException $exception) {
            if (in_array($exception->getStatusCode(), [404, 409], true)) {
                throw new ValidationException('Unauthorized', 401);
            }

            throw $exception;
        }
    }
}
