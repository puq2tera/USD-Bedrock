<?php

declare(strict_types=1);

namespace BedrockStarter\requests\messages;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\messages\CreateMessageResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class CreateMessageRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/messages$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $userID,
        private readonly string $name,
        private readonly string $message
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
        return 'CreateMessage';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireInt('userID', 1),
            Request::requireString('name', 1, Request::MAX_SIZE_SMALL),
            Request::requireString('message', 1, Request::MAX_SIZE_QUERY)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'userID' => (string)$this->userID,
            'name' => $this->name,
            'message' => $this->message,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreateMessageResponse($bedrockResponse);
    }
}
