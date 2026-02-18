<?php

declare(strict_types=1);

namespace BedrockStarter;

class Request
{
    public const MAX_SIZE_SMALL = 255;
    public const MAX_SIZE_QUERY = 1024 * 1024;

    private static ?array $cachedData = null;

    /**
     * @return mixed
     */
    private static function getRawParam(string $key)
    {
        // Query-string takes precedence over body to preserve existing endpoint behavior.
        $data = self::getData();
        return $_GET[$key] ?? $data[$key] ?? null;
    }

    /**
     * @param mixed $rawValue
     */
    private static function parseIntStrict($rawValue, string $key): int
    {
        if (is_int($rawValue)) {
            return $rawValue;
        }

        $value = trim((string)$rawValue);
        if ($value === '') {
            throw new ValidationException("Missing required parameter: {$key}", 400);
        }

        $intValue = filter_var($value, FILTER_VALIDATE_INT);
        if ($intValue === false) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }

        return (int)$intValue;
    }

    private static function validateIntBounds(int $value, string $key, ?int $min, ?int $max): int
    {
        if ($min !== null && $value < $min) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }
        if ($max !== null && $value > $max) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }
        return $value;
    }

    /**
     * Get data from POST body (supports both JSON and form data)
     */
    public static function getData(): array
    {
        if (self::$cachedData !== null) {
            return self::$cachedData;
        }

        if (!empty($_POST)) {
            self::$cachedData = $_POST;
            return self::$cachedData;
        }

        $rawInput = file_get_contents('php://input');
        if ($rawInput === false || $rawInput === '') {
            self::$cachedData = [];
            return self::$cachedData;
        }

        $decoded = json_decode($rawInput, true);
        self::$cachedData = is_array($decoded) ? $decoded : [];
        return self::$cachedData;
    }

    /**
     * Get a string parameter from GET/POST, with optional default
     */
    public static function getString(string $key, string $default = ''): string
    {
        $value = self::getRawParam($key);
        if ($value === null) {
            return $default;
        }
        return trim((string)$value);
    }

    /**
     * Get an integer parameter from GET/POST, with optional default and bounds
     */
    public static function getInt(string $key, ?int $default = null, ?int $min = null, ?int $max = null): ?int
    {
        $value = self::getRawParam($key);

        if ($value === null) {
            return $default;
        }

        $intValue = (int) $value;

        if ($min !== null) {
            $intValue = max($min, $intValue);
        }

        if ($max !== null) {
            $intValue = min($max, $intValue);
        }

        return $intValue;
    }

    /**
     * Strict integer parsing. Rejects non-integer values and out-of-range values.
     */
    public static function getIntStrict(string $key, ?int $default = null, ?int $min = null, ?int $max = null): ?int
    {
        $value = self::getRawParam($key);

        if ($value === null) {
            return $default;
        }

        $intValue = self::parseIntStrict($value, $key);
        return self::validateIntBounds($intValue, $key, $min, $max);
    }

    /**
     * Require a strict integer parameter.
     */
    public static function requireInt(string $key, ?int $min = null, ?int $max = null): int
    {
        $value = self::getIntStrict($key, null, $min, $max);
        if ($value === null) {
            throw new ValidationException("Missing required parameter: {$key}", 400);
        }
        return $value;
    }

    /**
     * Require a string parameter (throws 400 if missing or empty)
     */
    public static function requireString(string $key, int $minLength = 1, ?int $maxLength = null): string
    {
        $value = self::getString($key);
        if ($value === '' || strlen($value) < $minLength) {
            throw new ValidationException("Missing required parameter: {$key}", 400);
        }
        if ($maxLength !== null && strlen($value) > $maxLength) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }
        return $value;
    }

    /**
     * Require a boolean parameter. Accepts true/false/1/0.
     */
    public static function requireBool(string $key): bool
    {
        $value = strtolower(self::requireString($key));
        if ($value === 'true' || $value === '1') {
            return true;
        }
        if ($value === 'false' || $value === '0') {
            return false;
        }

        throw new ValidationException("Invalid parameter: {$key}", 400);
    }

    /**
     * Require a JSON array parameter.
     * Accepts either an already-decoded list from JSON body or a JSON array string.
     *
     * @return array<int, mixed>
     */
    public static function requireJsonArray(string $key, ?int $minItems = null, ?int $maxItems = null): array
    {
        $rawValue = self::getRawParam($key);
        if ($rawValue === null) {
            throw new ValidationException("Missing required parameter: {$key}", 400);
        }

        if (is_array($rawValue)) {
            $decoded = $rawValue;
        } else {
            $decoded = json_decode(trim((string)$rawValue), true);
        }

        if (!is_array($decoded) || !array_is_list($decoded)) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }

        $itemCount = count($decoded);
        if ($minItems !== null && $itemCount < $minItems) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }
        if ($maxItems !== null && $itemCount > $maxItems) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }

        return $decoded;
    }

    public static function hasParam(string $key): bool
    {
        $data = self::getData();
        return array_key_exists($key, $_GET) || array_key_exists($key, $data);
    }

    public static function getOptionalString(string $key, int $minLength = 1, ?int $maxLength = null): ?string
    {
        if (!self::hasParam($key)) {
            return null;
        }

        $value = self::getString($key);
        if ($value === '') {
            return null;
        }

        if (strlen($value) < $minLength) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }
        if ($maxLength !== null && strlen($value) > $maxLength) {
            throw new ValidationException("Invalid parameter: {$key}", 400);
        }

        return $value;
    }

    /**
     * @return array<int, mixed>|null
     */
    public static function getOptionalJsonArray(string $key, ?int $minItems = null, ?int $maxItems = null): ?array
    {
        if (!self::hasParam($key)) {
            return null;
        }

        return self::requireJsonArray($key, $minItems, $maxItems);
    }

    /**
     * Require a route parameter as a non-empty string.
     */
    public static function requireRouteString(array $routeParams, string $key, int $minLength = 1, ?int $maxLength = null): string
    {
        $rawValue = $routeParams[$key] ?? null;
        if ($rawValue === null) {
            throw new ValidationException("Missing required route parameter: {$key}", 400);
        }

        $value = trim((string)$rawValue);
        if ($value === '' || strlen($value) < $minLength) {
            throw new ValidationException("Invalid route parameter: {$key}", 400);
        }
        if ($maxLength !== null && strlen($value) > $maxLength) {
            throw new ValidationException("Invalid route parameter: {$key}", 400);
        }

        return $value;
    }

    /**
     * Require a positive integer route parameter by its regex named capture.
     */
    public static function requireRouteInt(array $routeParams, string $key, int $min = 1): int
    {
        $rawValue = $routeParams[$key] ?? null;
        if ($rawValue === null) {
            throw new ValidationException("Missing required route parameter: {$key}", 400);
        }

        $value = trim((string)$rawValue);
        if ($value === '' || !ctype_digit($value)) {
            throw new ValidationException("Invalid route parameter: {$key}", 400);
        }

        $intValue = (int)$value;
        if ($intValue < $min) {
            throw new ValidationException("Invalid route parameter: {$key}", 400);
        }

        return $intValue;
    }

    /**
     * Get request method
     */
    public static function getMethod(): string
    {
        return $_SERVER['REQUEST_METHOD'];
    }

    /**
     * Get request path
     */
    public static function getPath(): string
    {
        return parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH) ?? '/';
    }
}
