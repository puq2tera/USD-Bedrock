<?php
/**
 * Simple PHP API for Bedrock Starter
 */

declare(strict_types=1);

require __DIR__ . '/vendor/autoload.php';

use BedrockStarter\Bedrock;
use BedrockStarter\Request;

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Simple routing
$path = Request::getPath();
$method = Request::getMethod();

switch ($path) {
    case '/api/status':
        $response = [
            'status' => 'ok',
            'service' => 'bedrock-starter-api',
            'timestamp' => date('c'),
            'php_version' => PHP_VERSION
        ];

        echo json_encode($response, JSON_PRETTY_PRINT);
        break;

    case '/api/hello':
        $name = Request::getString('name', 'World');
        echo json_encode(Bedrock::call("HelloWorld", ["name" => $name]));
        break;

    case '/api/messages':
        if ($method === 'GET') {
            $limit = Request::getInt('limit', null, 1, 100);

            $params = [];
            if ($limit !== null) {
                $params['limit'] = (string) $limit;
            }

            $bedrockResponse = Bedrock::call("GetMessages", $params);
            if (isset($bedrockResponse['messages'])) {
                $decodedMessages = json_decode($bedrockResponse['messages'], true);
                if (is_array($decodedMessages)) {
                    $bedrockResponse['messages'] = $decodedMessages;
                }
            }

            echo json_encode($bedrockResponse);
            break;
        }

        if ($method === 'POST') {
            $name = Request::requireString('name');
            $message = Request::requireString('message');
            echo json_encode(Bedrock::call("CreateMessage", [
                'name' => $name,
                'message' => $message,
            ]));
            break;
        }

        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed', 'allowed' => ['GET', 'POST']]);
        break;

    case '/api/polls':
        if ($method === 'POST') {
            $question = Request::requireString('question');
            $options = Request::requireString('options');

            echo json_encode(Bedrock::call("CreatePoll", [
                'question' => $question,
                'options' => $options,
            ]));
            break;
        }

        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed', 'allowed' => ['POST']]);
        break;

    default:
        http_response_code(404);
        echo json_encode(['error' => 'Endpoint not found']);
        break;
}
