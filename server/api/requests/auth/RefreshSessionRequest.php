<?php

declare(strict_types=1);

namespace BedrockStarter\requests\auth;

use BedrockStarter\Auth;
use BedrockStarter\Bedrock;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\config\AppConfig;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\auth\RefreshTokenResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class RefreshSessionRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/auth/refresh$#';
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
        return new self(Request::requireString('refreshToken', 1, Request::MAX_SIZE_QUERY));
    }

    public function toBedrockParams(): array
    {
        return [];
    }

    public function execute(): RouteResponse
    {
        $refreshTokenHash = Auth::hashRefreshToken($this->refreshToken);
        $session = $this->requireValidRefreshSession(
            $this->normalizeUnauthorized(static fn(): array => Bedrock::call('GetUserSessionByRefreshTokenHash', [
                'refreshTokenHash' => $refreshTokenHash,
            ]))
        );

        $newRefreshToken = Auth::generateRefreshToken();
        $rotated = $this->normalizeUnauthorized(static fn(): array => Bedrock::call('RotateUserSession', [
            'sessionID' => (string)$session['sessionID'],
            'refreshTokenHash' => Auth::hashRefreshToken($newRefreshToken),
            'expiresAt' => (string)$this->refreshTokenExpiresAt(),
            'userAgent' => Auth::extractUserAgent() ?? '',
        ]));

        $newSessionID = isset($rotated['sessionID']) ? (int)$rotated['sessionID'] : 0;
        if ($newSessionID <= 0) {
            throw new ValidationException('Unauthorized', 401);
        }

        return new RefreshTokenResponse(
            Auth::issueAccessToken($session['userID'], $newSessionID),
            $newRefreshToken
        );
    }

    /**
     * @param array<string, mixed> $session
     * @return array{sessionID:int,userID:int,expiresAt:int,revokedAt:?int,replacedBySessionID:?int}
     */
    private function requireValidRefreshSession(array $session): array
    {
        $sessionID = (int)($session['sessionID'] ?? 0);
        $userID = (int)($session['userID'] ?? 0);
        $expiresAt = (int)($session['expiresAt'] ?? 0);
        $revokedAt = isset($session['revokedAt']) ? (int)$session['revokedAt'] : null;
        $replacedBySessionID = isset($session['replacedBySessionID']) ? (int)$session['replacedBySessionID'] : null;

        if ($sessionID <= 0 ||
            $userID <= 0 ||
            $expiresAt <= $this->nowMicroseconds() ||
            $revokedAt !== null ||
            $replacedBySessionID !== null) {
            throw new ValidationException('Unauthorized', 401);
        }

        return [
            'sessionID' => $sessionID,
            'userID' => $userID,
            'expiresAt' => $expiresAt,
            'revokedAt' => $revokedAt,
            'replacedBySessionID' => $replacedBySessionID,
        ];
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

    private function nowMicroseconds(): int
    {
        return (int)(microtime(true) * 1000000);
    }

    private function refreshTokenExpiresAt(): int
    {
        return $this->nowMicroseconds() + (AppConfig::refreshTokenTtlSeconds() * 1000000);
    }
}
