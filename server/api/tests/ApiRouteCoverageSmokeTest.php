<?php

declare(strict_types=1);

require dirname(__DIR__) . '/vendor/autoload.php';

use BedrockStarter\Auth;
use BedrockStarter\Request;
use BedrockStarter\ValidationException;
use BedrockStarter\requests\account\DeleteAccountRequest;
use BedrockStarter\requests\account\EditAccountRequest;
use BedrockStarter\requests\account\GetAccountRequest;
use BedrockStarter\requests\auth\CheckEmailExistsRequest;
use BedrockStarter\requests\auth\LogoutRequest;
use BedrockStarter\requests\auth\RefreshSessionRequest;
use BedrockStarter\requests\chats\AddChatMemberRequest;
use BedrockStarter\requests\chats\CreateChatMessageRequest;
use BedrockStarter\requests\chats\CreateChatRequest;
use BedrockStarter\requests\chats\DeleteChatMessageRequest;
use BedrockStarter\requests\chats\DeleteChatRequest;
use BedrockStarter\requests\chats\EditChatMemberRoleRequest;
use BedrockStarter\requests\chats\EditChatMessageRequest;
use BedrockStarter\requests\chats\EditChatRequest;
use BedrockStarter\requests\chats\GetChatMessagesRequest;
use BedrockStarter\requests\chats\GetChatRequest;
use BedrockStarter\requests\chats\ListChatMembersRequest;
use BedrockStarter\requests\chats\ListChatsRequest;
use BedrockStarter\requests\chats\RemoveChatMemberRequest;
use BedrockStarter\requests\framework\RouteBinder;
use BedrockStarter\requests\polls\CreatePollRequest;
use BedrockStarter\requests\polls\DeleteAllPollVotesRequest;
use BedrockStarter\requests\polls\DeletePollRequest;
use BedrockStarter\requests\polls\DeletePollVotesRequest;
use BedrockStarter\requests\polls\EditPollRequest;
use BedrockStarter\requests\polls\GetPollParticipationRequest;
use BedrockStarter\requests\polls\GetPollRequest;
use BedrockStarter\requests\polls\ListPollsRequest;
use BedrockStarter\requests\polls\SubmitPollTextResponseRequest;
use BedrockStarter\requests\polls\SubmitPollVotesRequest;
use BedrockStarter\requests\system\HelloWorldRequest;
use BedrockStarter\requests\system\StatusRequest;
use BedrockStarter\requests\users\CreateUserRequest;
use BedrockStarter\requests\users\GetUserRequest;
use BedrockStarter\requests\users\LoginUserRequest;
use BedrockStarter\requests\users\LookupUserByEmailRequest;
use BedrockStarter\requests\users\LookupUsersRequest;

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

function assertTrue(bool $condition, string $message): void
{
    if (!$condition) {
        fwrite(STDERR, "FAIL: {$message}\n");
        exit(1);
    }
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

function run(): void
{
    $allRoutes = [
        StatusRequest::class,
        HelloWorldRequest::class,
        CreateChatRequest::class,
        ListChatsRequest::class,
        GetChatRequest::class,
        EditChatRequest::class,
        DeleteChatRequest::class,
        AddChatMemberRequest::class,
        ListChatMembersRequest::class,
        EditChatMemberRoleRequest::class,
        RemoveChatMemberRequest::class,
        CreateChatMessageRequest::class,
        GetChatMessagesRequest::class,
        EditChatMessageRequest::class,
        DeleteChatMessageRequest::class,
        CreatePollRequest::class,
        ListPollsRequest::class,
        GetPollRequest::class,
        GetPollParticipationRequest::class,
        EditPollRequest::class,
        DeletePollRequest::class,
        SubmitPollVotesRequest::class,
        DeleteAllPollVotesRequest::class,
        DeletePollVotesRequest::class,
        SubmitPollTextResponseRequest::class,
        CreateUserRequest::class,
        LoginUserRequest::class,
        CheckEmailExistsRequest::class,
        RefreshSessionRequest::class,
        LogoutRequest::class,
        GetAccountRequest::class,
        EditAccountRequest::class,
        DeleteAccountRequest::class,
        GetUserRequest::class,
        LookupUsersRequest::class,
        LookupUserByEmailRequest::class,
    ];

    // Public routes.
    resetRequestState();
    assertTrue(StatusRequest::tryBind('GET', '/api/status') !== null, 'GET /api/status should bind');

    resetRequestState();
    $_GET = ['name' => 'Frontend'];
    assertTrue(HelloWorldRequest::tryBind('GET', '/api/hello') !== null, 'GET /api/hello should bind');

    resetRequestState();
    $_POST = [
        'email' => 'person@example.com',
        'password' => 'Password1!',
        'firstName' => 'Test',
        'lastName' => 'User',
    ];
    assertTrue(CreateUserRequest::tryBind('POST', '/api/users') !== null, 'POST /api/users should bind');

    resetRequestState();
    $_POST = ['email' => 'person@example.com', 'password' => 'Password1!'];
    assertTrue(LoginUserRequest::tryBind('POST', '/api/auth/login') !== null, 'POST /api/auth/login should bind');

    resetRequestState();
    $_GET = ['email' => 'person@example.com'];
    assertTrue(CheckEmailExistsRequest::tryBind('GET', '/api/auth/email-exists') !== null, 'GET /api/auth/email-exists should bind');

    resetRequestState();
    $_POST = ['refreshToken' => 'token'];
    assertTrue(RefreshSessionRequest::tryBind('POST', '/api/auth/refresh') !== null, 'POST /api/auth/refresh should bind');

    // Authenticated routes.
    $token = 'Bearer ' . Auth::issueAccessToken(42, 77);

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['title' => 'Team Chat'];
    assertTrue(CreateChatRequest::tryBind('POST', '/api/chats') !== null, 'POST /api/chats should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_GET = ['limit' => '25', 'beforeChatID' => '100'];
    assertTrue(ListChatsRequest::tryBind('GET', '/api/chats') !== null, 'GET /api/chats should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(GetChatRequest::tryBind('GET', '/api/chats/10') !== null, 'GET /api/chats/{chatID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['title' => 'Renamed'];
    assertTrue(EditChatRequest::tryBind('PUT', '/api/chats/10') !== null, 'PUT /api/chats/{chatID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeleteChatRequest::tryBind('DELETE', '/api/chats/10') !== null, 'DELETE /api/chats/{chatID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['userID' => 11, 'role' => 'owner'];
    assertTrue(AddChatMemberRequest::tryBind('POST', '/api/chats/10/members') !== null, 'POST /api/chats/{chatID}/members should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(ListChatMembersRequest::tryBind('GET', '/api/chats/10/members') !== null, 'GET /api/chats/{chatID}/members should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['role' => 'member'];
    assertTrue(EditChatMemberRoleRequest::tryBind('PUT', '/api/chats/10/members/11') !== null, 'PUT /api/chats/{chatID}/members/{userID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(RemoveChatMemberRequest::tryBind('DELETE', '/api/chats/10/members/11') !== null, 'DELETE /api/chats/{chatID}/members/{userID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['body' => 'Hello'];
    assertTrue(CreateChatMessageRequest::tryBind('POST', '/api/chats/10/messages') !== null, 'POST /api/chats/{chatID}/messages should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_GET = ['limit' => '20', 'beforeMessageID' => '500'];
    assertTrue(GetChatMessagesRequest::tryBind('GET', '/api/chats/10/messages') !== null, 'GET /api/chats/{chatID}/messages should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['body' => 'Edited'];
    assertTrue(EditChatMessageRequest::tryBind('PUT', '/api/chats/10/messages/12') !== null, 'PUT /api/chats/{chatID}/messages/{messageID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeleteChatMessageRequest::tryBind('DELETE', '/api/chats/10/messages/12') !== null, 'DELETE /api/chats/{chatID}/messages/{messageID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['question' => 'Q?', 'type' => 'single_choice', 'options' => ['A', 'B']];
    assertTrue(CreatePollRequest::tryBind('POST', '/api/chats/10/polls') !== null, 'POST /api/chats/{chatID}/polls should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_GET = ['includeClosed' => 'true', 'beforePollID' => '50'];
    assertTrue(ListPollsRequest::tryBind('GET', '/api/chats/10/polls') !== null, 'GET /api/chats/{chatID}/polls should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(GetPollRequest::tryBind('GET', '/api/polls/50') !== null, 'GET /api/polls/{pollID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(GetPollParticipationRequest::tryBind('GET', '/api/polls/50/participation') !== null, 'GET /api/polls/{pollID}/participation should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['question' => 'Updated'];
    assertTrue(EditPollRequest::tryBind('PUT', '/api/polls/50') !== null, 'PUT /api/polls/{pollID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeletePollRequest::tryBind('DELETE', '/api/polls/50') !== null, 'DELETE /api/polls/{pollID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['optionIDs' => [1, 2]];
    assertTrue(SubmitPollVotesRequest::tryBind('POST', '/api/polls/50/votes') !== null, 'POST /api/polls/{pollID}/votes should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeleteAllPollVotesRequest::tryBind('DELETE', '/api/polls/50/votes/all') !== null, 'DELETE /api/polls/{pollID}/votes/all should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeletePollVotesRequest::tryBind('DELETE', '/api/polls/50/votes') !== null, 'DELETE /api/polls/{pollID}/votes should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['textValue' => 'My answer'];
    assertTrue(SubmitPollTextResponseRequest::tryBind('POST', '/api/polls/50/responses') !== null, 'POST /api/polls/{pollID}/responses should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['refreshToken' => 'token'];
    assertTrue(LogoutRequest::tryBind('POST', '/api/auth/logout') !== null, 'POST /api/auth/logout should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(GetAccountRequest::tryBind('GET', '/api/account') !== null, 'GET /api/account should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['firstName' => 'A'];
    assertTrue(EditAccountRequest::tryBind('PUT', '/api/account') !== null, 'PUT /api/account should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(DeleteAccountRequest::tryBind('DELETE', '/api/account') !== null, 'DELETE /api/account should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    assertTrue(GetUserRequest::tryBind('GET', '/api/users/5') !== null, 'GET /api/users/{userID} should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['userIDs' => [1, '2', 3]];
    assertTrue(LookupUsersRequest::tryBind('POST', '/api/users/lookup') !== null, 'POST /api/users/lookup should bind');

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_GET = ['email' => 'person@example.com'];
    assertTrue(LookupUserByEmailRequest::tryBind('GET', '/api/users/by-email') !== null, 'GET /api/users/by-email should bind');

    // Validation and router behavior checks.
    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['role' => 'admin'];
    assertStatusCode(
        static fn() => EditChatMemberRoleRequest::tryBind('PUT', '/api/chats/10/members/11'),
        400,
        'PUT chat member role with invalid role should fail'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_GET = ['limit' => 'abc'];
    assertStatusCode(
        static fn() => GetChatMessagesRequest::tryBind('GET', '/api/chats/10/messages'),
        400,
        'GET chat messages with non-integer limit should fail'
    );

    resetRequestState();
    $_SERVER['HTTP_AUTHORIZATION'] = $token;
    $_POST = ['question' => 'Q?', 'type' => 'free_text', 'options' => ['A']];
    assertStatusCode(
        static fn() => CreatePollRequest::tryBind('POST', '/api/chats/10/polls'),
        400,
        'POST free_text poll with non-empty options should fail'
    );

    $allowed = RouteBinder::allowedMethodsForPath('/api/chats/10/messages/12', $allRoutes);
    sort($allowed);
    assertTrue($allowed === ['DELETE', 'PUT'], '/api/chats/{chatID}/messages/{messageID} should advertise PUT/DELETE');

    $missingAllowed = RouteBinder::allowedMethodsForPath('/api/not-real', $allRoutes);
    assertTrue(count($missingAllowed) === 0, 'Unknown path should advertise no methods');

    fwrite(STDOUT, "PASS: API route coverage smoke tests\n");
}

run();
