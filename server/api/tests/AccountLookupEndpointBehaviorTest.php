<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\account\DeleteAccountRequest;
use BedrockStarter\requests\account\EditAccountRequest;
use BedrockStarter\requests\account\GetAccountRequest;
use BedrockStarter\requests\framework\RouteBinder;
use BedrockStarter\requests\users\GetUserRequest;
use BedrockStarter\requests\users\LookupUsersRequest;
use BedrockStarter\requests\users\LookupUserByEmailRequest;

/**
 * Endpoint behavior checks for phase-1 account/user lookup contract.
 * Run with: php server/api/tests/AccountLookupEndpointBehaviorTest.php
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
    $phase1RequestTypes = [
        GetAccountRequest::class,
        EditAccountRequest::class,
        DeleteAccountRequest::class,
        GetUserRequest::class,
        LookupUsersRequest::class,
        LookupUserByEmailRequest::class,
    ];

    // Success: account and lookup endpoints bind and parse expected params.
    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $getAccount = GetAccountRequest::tryBind('GET', '/api/account');
    assertTrue($getAccount !== null, 'GET /api/account should bind with auth token');
    assertTrue($getAccount->toBedrockParams()['userID'] === '42', 'GET /api/account should map authenticated userID');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $getUser = GetUserRequest::tryBind('GET', '/api/users/7');
    assertTrue($getUser !== null, 'GET /api/users/{userID} should bind with auth token');
    assertTrue($getUser->toBedrockParams()['userID'] === '7', 'GET /api/users/{userID} should map route userID');

    resetRequestState();
    $_POST = ['userIDs' => [7, 8, 9]];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $lookupUsers = LookupUsersRequest::tryBind('POST', '/api/users/lookup');
    assertTrue($lookupUsers !== null, 'POST /api/users/lookup should bind with auth token');

    resetRequestState();
    $_GET = ['email' => 'person@example.com'];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $lookupByEmail = LookupUserByEmailRequest::tryBind('GET', '/api/users/by-email');
    assertTrue($lookupByEmail !== null, 'GET /api/users/by-email should bind with auth token');

    // Forbidden-style auth failures (request guard currently returns 401 for these).
    resetRequestState();
    assertStatusCode(
        static fn() => GetAccountRequest::tryBind('GET', '/api/account'),
        401,
        'GET /api/account without auth should fail with 401'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90, ['typ' => 'refresh']);
    assertStatusCode(
        static fn() => GetUserRequest::tryBind('GET', '/api/users/7'),
        401,
        'GET /api/users/{userID} with refresh token should fail with 401'
    );

    // 404 contract checks: unrelated paths should not bind and should not advertise allowed methods.
    resetRequestState();
    $notFoundBind = RouteBinder::tryBind('GET', '/api/account/profile', $phase1RequestTypes);
    assertTrue($notFoundBind === null, 'Unknown phase-1 endpoint path should not bind');
    $notFoundAllowed = RouteBinder::allowedMethodsForPath('/api/account/profile', $phase1RequestTypes);
    assertTrue(count($notFoundAllowed) === 0, 'Unknown phase-1 endpoint path should have no allowed methods');

    // Method mismatch check (maps to API 405 path handling when run through api.php).
    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $wrongMethod = RouteBinder::tryBind('POST', '/api/account', $phase1RequestTypes);
    assertTrue($wrongMethod === null, 'POST /api/account should not bind');
    $allowedMethods = RouteBinder::allowedMethodsForPath('/api/account', $phase1RequestTypes);
    sort($allowedMethods);
    assertTrue($allowedMethods === ['DELETE', 'GET', 'PUT'], '/api/account should advertise GET/PUT/DELETE');

    fwrite(STDOUT, "PASS: Account and lookup endpoint behavior checks\n");
}

run();
