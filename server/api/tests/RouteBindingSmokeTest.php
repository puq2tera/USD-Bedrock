<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Request;
use BedrockStarter\requests\polls\GetPollParticipationRequest;
use BedrockStarter\requests\users\DeleteUserRequest;
use BedrockStarter\requests\users\GetUserRequest;

/**
 * Lightweight smoke checks for API route binding.
 * Run with: php server/api/tests/RouteBindingSmokeTest.php
 */

function resetRequestState(): void
{
    $_GET = [];
    $_POST = [];
    $_REQUEST = [];
    $_SERVER['REQUEST_METHOD'] = 'GET';
    $_SERVER['REQUEST_URI'] = '/';

    $reflection = new ReflectionClass(Request::class);
    $cachedData = $reflection->getProperty('cachedData');
    $cachedData->setAccessible(true);
    $cachedData->setValue(null, null);
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
    $getUser = GetUserRequest::tryBind('GET', '/api/users/42');
    assertTrue($getUser !== null, 'GET /api/users/{userID} should bind GetUserRequest');

    resetRequestState();
    $deleteUser = DeleteUserRequest::tryBind('DELETE', '/api/users/42');
    assertTrue($deleteUser !== null, 'DELETE /api/users/{userID} should bind DeleteUserRequest');

    resetRequestState();
    $_GET['requesterUserID'] = '7';
    $participation = GetPollParticipationRequest::tryBind('GET', '/api/polls/99/participation');
    assertTrue($participation !== null, 'GET /api/polls/{pollID}/participation should bind request');

    resetRequestState();
    $wrongMethod = GetPollParticipationRequest::tryBind('POST', '/api/polls/99/participation');
    assertTrue($wrongMethod === null, 'POST participation path must not bind GetPollParticipationRequest');

    fwrite(STDOUT, "PASS: Route binding smoke tests\n");
}

run();
