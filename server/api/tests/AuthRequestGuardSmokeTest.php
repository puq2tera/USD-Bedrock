<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\polls\GetPollRequest;

/**
 * Auth route guard smoke checks.
 * Run with: php server/api/tests/AuthRequestGuardSmokeTest.php
 */

function resetRequestState(): void
{
    $_GET = [];
    $_POST = [];
    $_REQUEST = [];
    $_SERVER['REQUEST_METHOD'] = 'GET';
    $_SERVER['REQUEST_URI'] = '/';
    $_SERVER['HTTP_AUTHORIZATION'] = '';

    $reflection = new ReflectionClass(Request::class);
    $cachedData = $reflection->getProperty('cachedData');
    $cachedData->setAccessible(true);
    $cachedData->setValue(null, null);
}

function assertStatusCode(callable $fn, int $expectedCode, string $message): void
{
    try {
        $fn();
    } catch (ValidationException $e) {
        if ($e->getStatusCode() === $expectedCode) {
            return;
        }

        fwrite(STDERR, "FAIL: {$message} (got {$e->getStatusCode()})\n");
        exit(1);
    }

    fwrite(STDERR, "FAIL: {$message} (no exception)\n");
    exit(1);
}

function assertTrue(bool $condition, string $message): void
{
    if (!$condition) {
        fwrite(STDERR, "FAIL: {$message}\n");
        exit(1);
    }
}

function run(): void
{
    resetRequestState();
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route without token should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer invalid-token';
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route with invalid token should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueToken(7);
    $bound = GetPollRequest::tryBind('GET', '/api/polls/10');
    assertTrue($bound !== null, 'Protected route with valid token should bind');

    fwrite(STDOUT, "PASS: Auth request guard smoke tests\n");
}

run();
