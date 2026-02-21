<?php

declare(strict_types=1);

namespace BedrockStarter\requests\chats;

use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\chats\ListChatsResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class ListChatsRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(
        private readonly int $userID,
        private readonly ?int $limit,
        private readonly ?int $beforeChatID
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
        return 'ListChats';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(
            Request::requireInt('userID', 1),
            // Validate page size early so callers cannot request unbounded chat lists.
            // If omitted, command layer uses the default page size.
            Request::getIntStrict('limit', null, 1, 100),
            // Cursor pagination: return chats with chatID lower than beforeChatID.
            Request::getIntStrict('beforeChatID', null, 1)
        );
    }

    public function toBedrockParams(): array
    {
        $params = [
            'userID' => (string)$this->userID,
        ];

        if ($this->limit !== null) {
            $params['limit'] = (string)$this->limit;
        }
        if ($this->beforeChatID !== null) {
            $params['beforeChatID'] = (string)$this->beforeChatID;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new ListChatsResponse($bedrockResponse);
    }
}
