<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\SubmitPollVotesResponse;

final class SubmitPollVotesRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)/votes$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $pollID,
        private readonly int $userID,
        private readonly string $optionIDsJson
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
        return 'SubmitPollVotes';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $optionIDs = Request::requireJsonArray('optionIDs', 1, 20);
        $optionIDsJson = json_encode($optionIDs);
        if ($optionIDsJson === false) {
            throw new ValidationException('Invalid parameter: optionIDs', 400);
        }

        return new self(
            Request::requireRouteInt($routeParams, 'pollID'),
            Request::requireInt('userID', 1),
            $optionIDsJson
        );
    }

    public function toBedrockParams(): array
    {
        return [
            'pollID' => (string)$this->pollID,
            'userID' => (string)$this->userID,
            'optionIDs' => $this->optionIDsJson,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new SubmitPollVotesResponse($bedrockResponse);
    }
}
