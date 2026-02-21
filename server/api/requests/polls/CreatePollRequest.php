<?php

declare(strict_types=1);

namespace BedrockStarter\requests\polls;

use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\framework\RouteBoundRequestBase;
use BedrockStarter\responses\framework\RouteResponse;
use BedrockStarter\responses\polls\CreatePollResponse;

final class CreatePollRequest extends RouteBoundRequestBase
{
    private const PATH_PATTERN = '#^/api/chats/(?P<chatID>\d+)/polls$#';
    private const ALLOWED_METHODS = ['POST'];

    public function __construct(
        private readonly int $chatID,
        private readonly int $creatorUserID,
        private readonly string $question,
<<<<<<< HEAD
        private readonly string $type,
        private readonly bool $allowChangeVote,
        private readonly bool $isAnonymous,
        private readonly ?int $expiresAt,
        private readonly ?string $optionsJson
=======
        private readonly int $createdBy,
        private readonly string $optionsJson
>>>>>>> origin/main
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
        $chatID = Request::requireRouteInt($routeParams, 'chatID');
        $creatorUserID = Request::requireInt('creatorUserID', 1);
        $question = Request::requireString('question', 1, Request::MAX_SIZE_SMALL);
<<<<<<< HEAD
        $type = Request::requireString('type', 1, 64);

        $allowChangeVote = Request::hasParam('allowChangeVote') ? Request::requireBool('allowChangeVote') : false;
        $isAnonymous = Request::hasParam('isAnonymous') ? Request::requireBool('isAnonymous') : false;
        $expiresAt = Request::hasParam('expiresAt') ? Request::requireInt('expiresAt', 1) : null;

        $optionsJson = null;
        if (strtolower($type) !== 'free_text') {
            $options = Request::requireJsonArray('options', 2, 20);
            $encoded = json_encode($options);
            if ($encoded === false) {
                throw new ValidationException('Invalid parameter: options', 400);
            }
            $optionsJson = $encoded;
        } elseif (Request::hasParam('options')) {
            $options = Request::requireJsonArray('options', 0, 20);
            if (!empty($options)) {
                throw new ValidationException('Invalid parameter: options', 400);
            }
        }

        return new self(
            $chatID,
            $creatorUserID,
            $question,
            $type,
            $allowChangeVote,
            $isAnonymous,
            $expiresAt,
            $optionsJson
        );
=======
        $createdBy = Request::requireInt('createdBy', 1);
        $options = Request::requireJsonArray('options', 2);
        $optionsJson = json_encode($options);
        if ($optionsJson === false) {
            throw new ValidationException('Invalid parameter: options', 400);
        }

        return new self($question, $createdBy, $optionsJson);
>>>>>>> origin/main
    }

    public function toBedrockParams(): array
    {
        $params = [
            'chatID' => (string)$this->chatID,
            'creatorUserID' => (string)$this->creatorUserID,
            'question' => $this->question,
<<<<<<< HEAD
            'type' => $this->type,
            'allowChangeVote' => $this->allowChangeVote ? 'true' : 'false',
            'isAnonymous' => $this->isAnonymous ? 'true' : 'false',
=======
            'createdBy' => (string)$this->createdBy,
            'options' => $this->optionsJson,
>>>>>>> origin/main
        ];

        if ($this->expiresAt !== null) {
            $params['expiresAt'] = (string)$this->expiresAt;
        }

        if ($this->optionsJson !== null) {
            $params['options'] = $this->optionsJson;
        }

        return $params;
    }

    public function transformResponse(array $bedrockResponse): RouteResponse
    {
        return new CreatePollResponse($bedrockResponse);
    }
}
