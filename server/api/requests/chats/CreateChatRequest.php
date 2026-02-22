<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\CreateChatResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class CreateChatRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $creatorUserID,
        private readonly string $title
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
        return 'CreateChat';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireInt('creatorUserID', 1),
            Request::requireString('title', 1, Request::MAX_SIZE_SMALL)
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'creatorUserID' => (string)$this->creatorUserID,
            'title' => $this->title,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreateChatResponse($bedrockResponse);
    }
}
