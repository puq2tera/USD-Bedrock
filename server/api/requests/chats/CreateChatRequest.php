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
<<<<<<< HEAD:server/api/requests/chats/CreateChatRequest.php
        private readonly int $creatorUserID,
        private readonly string $title
=======
        private readonly int $userID,
        private readonly string $name,
        private readonly string $message
>>>>>>> origin/main:server/api/requests/messages/CreateMessageRequest.php
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
<<<<<<< HEAD:server/api/requests/chats/CreateChatRequest.php
            Request::requireInt('creatorUserID', 1),
            Request::requireString('title', 1, Request::MAX_SIZE_SMALL)
=======
            Request::requireInt('userID', 1),
            Request::requireString('name', 1, Request::MAX_SIZE_SMALL),
            Request::requireString('message', 1, Request::MAX_SIZE_QUERY)
>>>>>>> origin/main:server/api/requests/messages/CreateMessageRequest.php
        );
    }

    public function toBedrockParams(): array
    {
        return [
<<<<<<< HEAD:server/api/requests/chats/CreateChatRequest.php
            'creatorUserID' => (string)$this->creatorUserID,
            'title' => $this->title,
=======
            'userID' => (string)$this->userID,
            'name' => $this->name,
            'message' => $this->message,
>>>>>>> origin/main:server/api/requests/messages/CreateMessageRequest.php
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreateChatResponse($bedrockResponse);
    }
}
