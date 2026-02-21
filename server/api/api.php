<?php
/**
 * Simple PHP API for Bedrock Starter
 */

declare(strict_types=1);

require __DIR__ . '/vendor/autoload.php';

use BedrockStarter\Log;
use BedrockStarter\Request;
use BedrockStarter\requests\chats\AddChatMemberRequest;
use BedrockStarter\requests\chats\CreateChatRequest;
use BedrockStarter\requests\chats\DeleteChatRequest;
use BedrockStarter\requests\chats\EditChatMemberRoleRequest;
use BedrockStarter\requests\chats\EditChatRequest;
use BedrockStarter\requests\chats\GetChatRequest;
use BedrockStarter\requests\chats\ListChatMembersRequest;
use BedrockStarter\requests\chats\ListChatsRequest;
use BedrockStarter\requests\chats\RemoveChatMemberRequest;
use BedrockStarter\requests\framework\RouteBinder;
use BedrockStarter\requests\chats\CreateChatMessageRequest;
use BedrockStarter\requests\chats\DeleteChatMessageRequest;
use BedrockStarter\requests\chats\EditChatMessageRequest;
use BedrockStarter\requests\chats\GetChatMessagesRequest;
use BedrockStarter\requests\polls\CreatePollRequest;
use BedrockStarter\requests\polls\DeletePollVotesRequest;
use BedrockStarter\requests\polls\DeletePollRequest;
use BedrockStarter\requests\polls\EditPollRequest;
use BedrockStarter\requests\polls\GetPollRequest;
use BedrockStarter\requests\polls\ListPollsRequest;
use BedrockStarter\requests\polls\SubmitPollTextResponseRequest;
use BedrockStarter\requests\polls\SubmitPollVotesRequest;
use BedrockStarter\requests\system\HelloWorldRequest;
use BedrockStarter\requests\system\StatusRequest;
use BedrockStarter\requests\users\CreateUserRequest;
use BedrockStarter\requests\users\DeleteUserRequest;
use BedrockStarter\requests\users\EditUserRequest;
use BedrockStarter\requests\users\GetUserRequest;
use BedrockStarter\ValidationException;

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

$path = Request::getPath();
$method = Request::getMethod();
$requestTypes = [
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
    EditPollRequest::class,
    DeletePollRequest::class,
<<<<<<< HEAD
    SubmitPollVotesRequest::class,
    DeletePollVotesRequest::class,
    SubmitPollTextResponseRequest::class,
=======
    SubmitVoteRequest::class,
>>>>>>> origin/main
    CreateUserRequest::class,
    GetUserRequest::class,
    EditUserRequest::class,
    DeleteUserRequest::class,
];

try {
    $boundRequest = RouteBinder::tryBind($method, $path, $requestTypes);
    if ($boundRequest !== null) {
        echo json_encode($boundRequest->execute()->toArray());
        exit();
    }

    $allowedMethods = RouteBinder::allowedMethodsForPath($path, $requestTypes);
    if (!empty($allowedMethods)) {
        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed', 'allowed' => $allowedMethods]);
        exit();
    }

    http_response_code(404);
    echo json_encode(['error' => 'Endpoint not found']);
} catch (ValidationException $exception) {
    http_response_code($exception->getStatusCode());
    echo json_encode(['error' => $exception->getMessage()]);
} catch (\Throwable $exception) {
    Log::error('Unhandled API exception', ['exception' => $exception->getMessage()]);
    http_response_code(500);
    echo json_encode(['error' => 'Internal server error']);
}
