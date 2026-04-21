<?php

declare(strict_types=1);

namespace BedrockStarter\config;

final class Env
{
    public static function loadFromFile(string $path): void
    {
        if (!is_file($path) || !is_readable($path)) {
            return;
        }

        $lines = file($path, FILE_IGNORE_NEW_LINES);
        if (!is_array($lines)) {
            return;
        }

        foreach ($lines as $line) {
            $trimmed = trim($line);
            if ($trimmed === '' || str_starts_with($trimmed, '#')) {
                continue;
            }

            if (str_starts_with($trimmed, 'export ')) {
                $trimmed = trim(substr($trimmed, 7));
            }

            $separatorPos = strpos($trimmed, '=');
            if ($separatorPos === false) {
                continue;
            }

            $name = trim(substr($trimmed, 0, $separatorPos));
            $value = trim(substr($trimmed, $separatorPos + 1));
            if ($name === '' || !preg_match('/^[A-Z0-9_]+$/', $name)) {
                continue;
            }

            $value = self::normalizeValue($value);
            putenv("{$name}={$value}");
            $_ENV[$name] = $value;
            $_SERVER[$name] = $value;
        }
    }

    private static function normalizeValue(string $value): string
    {
        $length = strlen($value);
        if ($length < 2) {
            return $value;
        }

        $first = $value[0];
        $last = $value[$length - 1];
        if (($first === '"' && $last === '"') || ($first === "'" && $last === "'")) {
            return substr($value, 1, -1);
        }

        return $value;
    }
}
