<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\responses\polls\GetPollsResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class GetPollsRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls$#';
    private const ALLOWED_METHODS = ['GET'];

    public function __construct(private readonly ?int $limit)
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
        return 'GetPolls';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        return new self(Request::getIntStrict('limit', null, 1, 100));
    }

    public function toBedrockParams(): array
    {
        if ($this->limit === null) {
            return [];
        }

        return ['limit' => (string)$this->limit];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new GetPollsResponse($bedrockResponse);
    }
}
