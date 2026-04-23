<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Request;
use BedrockStarter\Auth;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\account\DeleteAccountRequest;
use BedrockStarter\requests\account\EditAccountRequest;
use BedrockStarter\requests\account\GetAccountRequest;
use BedrockStarter\requests\auth\CheckEmailExistsRequest;
use BedrockStarter\requests\auth\LogoutRequest;
use BedrockStarter\requests\auth\RefreshSessionRequest;
use BedrockStarter\requests\framework\RouteBinder;
use BedrockStarter\requests\polls\GetPollParticipationRequest;
use BedrockStarter\requests\users\GetUserRequest;
use BedrockStarter\requests\users\LoginUserRequest;
use BedrockStarter\requests\users\LookupUsersRequest;
use BedrockStarter\requests\users\LookupUserByEmailRequest;

/**
 * Lightweight smoke checks for API route binding.
 * Run with: php server/api/tests/RouteBindingSmokeTest.php
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
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $getUser = GetUserRequest::tryBind('GET', '/api/users/42');
    assertTrue($getUser !== null, 'GET /api/users/{userID} should bind GetUserRequest');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $getOtherUser = GetUserRequest::tryBind('GET', '/api/users/7');
    assertTrue($getOtherUser !== null, 'GET /api/users/{otherUserID} should bind GetUserRequest');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $getAccount = GetAccountRequest::tryBind('GET', '/api/account');
    assertTrue($getAccount !== null, 'GET /api/account should bind GetAccountRequest');

    resetRequestState();
    $_POST = ['firstName' => 'Updated'];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $editAccount = EditAccountRequest::tryBind('PUT', '/api/account');
    assertTrue($editAccount !== null, 'PUT /api/account should bind EditAccountRequest');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $deleteAccount = DeleteAccountRequest::tryBind('DELETE', '/api/account');
    assertTrue($deleteAccount !== null, 'DELETE /api/account should bind DeleteAccountRequest');

    resetRequestState();
    $_POST = ['firstName' => 'Other'];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $legacyEditSelf = RouteBinder::tryBind('PUT', '/api/users/42', [
        GetAccountRequest::class,
        EditAccountRequest::class,
        DeleteAccountRequest::class,
        GetUserRequest::class,
        LookupUsersRequest::class,
    ]);
    assertTrue($legacyEditSelf === null, 'PUT /api/users/{selfID} should not bind after account cutover');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $legacyDeleteSelf = RouteBinder::tryBind('DELETE', '/api/users/42', [
        GetAccountRequest::class,
        EditAccountRequest::class,
        DeleteAccountRequest::class,
        GetUserRequest::class,
        LookupUsersRequest::class,
    ]);
    assertTrue($legacyDeleteSelf === null, 'DELETE /api/users/{selfID} should not bind after account cutover');

    resetRequestState();
    $_POST = ['userIDs' => [42, 7, 99]];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $lookupUsers = LookupUsersRequest::tryBind('POST', '/api/users/lookup');
    assertTrue($lookupUsers !== null, 'POST /api/users/lookup should bind LookupUsersRequest');

    resetRequestState();
    $_POST = ['userIDs' => ['abc']];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    assertStatusCode(
        static fn() => LookupUsersRequest::tryBind('POST', '/api/users/lookup'),
        400,
        'POST /api/users/lookup with invalid IDs should fail validation'
    );

    resetRequestState();
    $_GET = ['email' => 'person@example.com'];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $lookupByEmail = LookupUserByEmailRequest::tryBind('GET', '/api/users/by-email');
    assertTrue($lookupByEmail !== null, 'GET /api/users/by-email should bind LookupUserByEmailRequest');

    resetRequestState();
    $_POST = ['email' => 'person@example.com', 'password' => 'Password1!'];
    $login = LoginUserRequest::tryBind('POST', '/api/auth/login');
    assertTrue($login !== null, 'POST /api/auth/login should bind LoginUserRequest');

    resetRequestState();
    $_GET = ['email' => 'person@example.com'];
    $emailExists = CheckEmailExistsRequest::tryBind('GET', '/api/auth/email-exists');
    assertTrue($emailExists !== null, 'GET /api/auth/email-exists should bind CheckEmailExistsRequest');

    resetRequestState();
    $_POST = ['refreshToken' => 'refresh-token'];
    $refresh = RefreshSessionRequest::tryBind('POST', '/api/auth/refresh');
    assertTrue($refresh !== null, 'POST /api/auth/refresh should bind RefreshSessionRequest');

    resetRequestState();
    $_POST = ['refreshToken' => 'refresh-token'];
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(42, 90);
    $logout = LogoutRequest::tryBind('POST', '/api/auth/logout');
    assertTrue($logout !== null, 'POST /api/auth/logout should bind LogoutRequest');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = 'Bearer ' . Auth::issueAccessToken(7, 91);
    $participation = GetPollParticipationRequest::tryBind('GET', '/api/polls/99/participation');
    assertTrue($participation !== null, 'GET /api/polls/{pollID}/participation should bind request');

    resetRequestState();
    $wrongMethod = GetPollParticipationRequest::tryBind('POST', '/api/polls/99/participation');
    assertTrue($wrongMethod === null, 'POST participation path must not bind GetPollParticipationRequest');

    fwrite(STDOUT, "PASS: Route binding smoke tests\n");
}

run();
