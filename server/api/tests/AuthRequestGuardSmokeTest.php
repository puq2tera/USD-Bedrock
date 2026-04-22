<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\account\GetAccountRequest;
use BedrockStarter\requests\polls\GetPollRequest;
use BedrockStarter\requests\users\GetUserRequest;
use BedrockStarter\requests\users\LookupUsersRequest;

/**
 * Auth route guard smoke checks.
 * Run with: php server/api/tests/AuthRequestGuardSmokeTest.php
 */

function resetRequestState(): void
{
    putenv('BEDROCK_API_JWT_SECRET=test-secret');
    putenv('BEDROCK_API_JWT_ISSUER=bedrock-starter');
    putenv('BEDROCK_API_JWT_AUDIENCE=bedrock-mobile');

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
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 11, ['exp' => time() - 1, 'iat' => time() - 10]);
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route with expired token should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 11, ['iss' => 'wrong-issuer']);
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route with wrong issuer should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 11, ['aud' => 'wrong-audience']);
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route with wrong audience should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 11, ['typ' => 'refresh']);
    assertStatusCode(
        static fn() => GetPollRequest::tryBind('GET', '/api/polls/10'),
        401,
        'Protected route with wrong token type should be 401'
    );

    resetRequestState();
    assertStatusCode(
        static fn() => GetAccountRequest::tryBind('GET', '/api/account'),
        401,
        'Account route without token should be 401'
    );

    resetRequestState();
    assertStatusCode(
        static fn() => GetUserRequest::tryBind('GET', '/api/users/10'),
        401,
        'User lookup route without token should be 401'
    );

    resetRequestState();
    $_POST = ['userIDs' => [10, 11]];
    assertStatusCode(
        static fn() => LookupUsersRequest::tryBind('POST', '/api/users/lookup'),
        401,
        'User lookup batch route without token should be 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 11);
    $bound = GetPollRequest::tryBind('GET', '/api/polls/10');
    assertTrue($bound !== null, 'Protected route with valid token should bind');

    fwrite(STDOUT, "PASS: Auth request guard smoke tests\n");
}

run();
