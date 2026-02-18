<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\responses\polls\EditPollResponse;
use BedrockStarter\responses\framework\RouteResponse;

final class EditPollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $pollID,
        private readonly ?string $question,
        private readonly ?string $optionsJson
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
        return 'EditPoll';
    }

    protected static function bindFromRouteMatch(array $routeParams): self
    {
        $pollID = Request::requireRouteInt($routeParams, 'pollID');
        $question = Request::getOptionalString('question', 1, Request::MAX_SIZE_SMALL);
        $options = Request::getOptionalJsonArray('options', 2);

        if ($question === null && $options === null) {
            throw new ValidationException('Missing required parameter: question or options', 400);
        }

        $optionsJson = null;
        if ($options !== null) {
            $encoded = json_encode($options);
            if ($encoded === false) {
                throw new ValidationException('Invalid parameter: options', 400);
            }
            $optionsJson = $encoded;
        }

        return new self($pollID, $question, $optionsJson);
    }

    public function toBedrockParams(): array
    {
        $params = ['pollID' => (string)$this->pollID];

        if ($this->question !== null) {
            $params['question'] = $this->question;
        }

        if ($this->optionsJson !== null) {
            $params['options'] = $this->optionsJson;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new EditPollResponse($bedrockResponse);
    }
}
