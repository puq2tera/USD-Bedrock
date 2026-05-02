<?php

declare(strict_types=1);

namespace BedrockStarter;

class ValidationException extends \RuntimeException
{
    public function __construct(
        string $message,
        private readonly int $statusCode = 400,
        private readonly ?string $errorCode = null,
        private readonly ?string $parameter = null
    )
    {
        parent::__construct($message);
    }

    public function getStatusCode(): int
    {
        return $this->statusCode;
    }

    public function getErrorCode(): ?string
    {
        return $this->errorCode;
    }

    public function getParameter(): ?string
    {
        return $this->parameter;
    }
}
