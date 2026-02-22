<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\ValidationException;

final class ChatRole
{
    public static function requireKnown(string $role): string
    {
        if ($role !== 'owner' && $role !== 'member') {
            throw new ValidationException('Invalid parameter: role', 400);
        }

        return $role;
    }
}
