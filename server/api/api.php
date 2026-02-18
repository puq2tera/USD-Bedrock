<?php
/**
 * Simple PHP API for Bedrock Starter
 */

declare(strict_types=1);

require __DIR__ . '/vendor/autoload.php';

use BedrockStarter\Log;
use BedrockStarter\Request;
use BedrockStarter\requests\framework\RouteBinder;
use BedrockStarter\requests\messages\CreateMessageRequest;
use BedrockStarter\requests\messages\GetMessagesRequest;
use BedrockStarter\requests\polls\CreatePollRequest;
use BedrockStarter\requests\polls\DeletePollRequest;
use BedrockStarter\requests\polls\EditPollRequest;
use BedrockStarter\requests\polls\GetPollRequest;
use BedrockStarter\requests\polls\GetPollsRequest;
use BedrockStarter\requests\polls\SubmitVoteRequest;
use BedrockStarter\requests\system\HelloWorldRequest;
use BedrockStarter\requests\system\StatusRequest;
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
    GetMessagesRequest::class,
    CreateMessageRequest::class,
    CreatePollRequest::class,
    GetPollsRequest::class,
    GetPollRequest::class,
    EditPollRequest::class,
    DeletePollRequest::class,
    SubmitVoteRequest::class,
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
