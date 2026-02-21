<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\config\AppConfig;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\EditPollResponse;

final class EditPollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/polls/(?P<pollID>\d+)$#';
    private const ALLOWED_METHODS = ['PUT'];

    public function __construct(
        private readonly int $pollID,
        private readonly int $actorUserID,
        private readonly ?string $question,
        private readonly ?bool $allowChangeVote,
        private readonly ?bool $isAnonymous,
        private readonly ?string $status,
        private readonly ?string $expiresAt,
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
        $actorUserID = Request::requireInt('actorUserID', 1);

        $question = Request::getOptionalString('question', 1, Request::MAX_SIZE_SMALL);
        $allowChangeVote = Request::hasParam('allowChangeVote') ? Request::requireBool('allowChangeVote') : null;
        $isAnonymous = Request::hasParam('isAnonymous') ? Request::requireBool('isAnonymous') : null;
        $status = Request::getOptionalString('status', 1, 64);

        $expiresAt = null;
        if (Request::hasParam('expiresAt')) {
            $rawExpiresAt = trim(Request::getString('expiresAt'));
            // For partial updates, we send literal "null" to mean "clear expiresAt".
            // If expiresAt is omitted entirely, command layer treats it as "leave unchanged".
            if ($rawExpiresAt === '' || strtolower($rawExpiresAt) === 'null') {
                $expiresAt = 'null';
            } else {
                $expiresAt = (string)Request::requireInt('expiresAt', 1);
            }
        }

        $optionsJson = null;
        if (Request::hasParam('options')) {
            $options = Request::requireJsonArray('options', 0, AppConfig::POLL_REQUEST_MAX_OPTIONS);
            $encoded = json_encode($options);
            if ($encoded === false) {
                throw new ValidationException('Invalid parameter: options', 400);
            }
            $optionsJson = $encoded;
        }

        if ($question === null && $allowChangeVote === null && $isAnonymous === null
            && $status === null && $expiresAt === null && $optionsJson === null) {
            // Reject empty edit requests: at least one editable field must be provided.
            throw new ValidationException('Missing required parameter: at least one editable field', 400);
        }

        return new self(
            $pollID,
            $actorUserID,
            $question,
            $allowChangeVote,
            $isAnonymous,
            $status,
            $expiresAt,
            $optionsJson
        );
    }

    public function toBedrockParams(): array
    {
        $params = [
            'pollID' => (string)$this->pollID,
            'actorUserID' => (string)$this->actorUserID,
        ];

        if ($this->question !== null) {
            $params['question'] = $this->question;
        }
        if ($this->allowChangeVote !== null) {
            $params['allowChangeVote'] = $this->allowChangeVote ? 'true' : 'false';
        }
        if ($this->isAnonymous !== null) {
            $params['isAnonymous'] = $this->isAnonymous ? 'true' : 'false';
        }
        if ($this->status !== null) {
            $params['status'] = $this->status;
        }
        if ($this->expiresAt !== null) {
            $params['expiresAt'] = $this->expiresAt;
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
