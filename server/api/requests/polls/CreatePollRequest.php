<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\responses\polls\CreatePollResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class CreatePollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly string $question,
        private readonly int $createdBy,
        private readonly string $optionsJson
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
        return 'CreatePoll';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $question = Request::requireString('question', 1, Request::MAX_SIZE_SMALL);
        $createdBy = Request::requireInt('createdBy', 1);
        $options = Request::requireJsonArray('options', 2);
        $optionsJson = json_encode($options);
        if ($optionsJson === false) {
            throw new ValidationException('Invalid parameter: options', 400);
        }

        return new self($question, $createdBy, $optionsJson);
    }

    public function toBedrockParams(): array
    {
        return [
            'question' => $this->question,
            'createdBy' => (string)$this->createdBy,
            'options' => $this->optionsJson,
        ];
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreatePollResponse($bedrockResponse);
    }
}
