<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\chats\GetChatMessagesResponse;

final class GetChatMessagesRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/messages$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $userID,
        private readonly ?int $limit,
        private readonly ?int $beforeMessageID
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
        return 'GetChatMessages';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireRouteInt($routeParams, 'chatID'),
            Request::requireInt('userID', 1),
            // Validate limit early so clients cannot request unbounded message pages.
            // When limit is omitted, command layer applies the default page size.
            Request::getIntStrict('limit', null, 1, 100),
            // Cursor pagination: return messages with IDs lower than beforeMessageID.
            Request::getIntStrict('beforeMessageID', null, 1)
        );
    }

    public function toBedrockParams(): array
    {
        $params = [
            'chatID' => (string)$this->chatID,
            'userID' => (string)$this->userID,
        ];

        if ($this->limit !== null) {
            $params['limit'] = (string)$this->limit;
        }
        if ($this->beforeMessageID !== null) {
            $params['beforeMessageID'] = (string)$this->beforeMessageID;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetChatMessagesResponse($bedrockResponse);
    }
}
