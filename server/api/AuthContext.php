<?php

declare(strict_types=1);

namespace BedrockStarter;

final class AuthContext
{
    /**
     * @param array<string, mixed> $claims
     */
    public function __construct(
        public readonly int $userID,
        public readonly int $sessionID,
        public readonly array $claims
    ) {
    }
}
