<?php

declare(strict_types=1);

namespace BedrockStarter\responses\polls;

use BedrockStarter\responses\framework\RouteResponse;

final class EditPollResponse implements RouteResponse
{
    public function __construct(private readonly array $payload)
    {
    }

    public function toArray(): array
    {
<<<<<<< HEAD
        return $this->payload;
=======
        $response = [
            'pollID' => (string)($this->payload['pollID'] ?? ''),
            'createdBy' => (string)($this->payload['createdBy'] ?? ''),
            'result' => (string)($this->payload['result'] ?? ''),
        ];

        if (isset($this->payload['optionCount'])) {
            $response['optionCount'] = (string)$this->payload['optionCount'];
        }

        return $response;
>>>>>>> origin/main
    }
}
