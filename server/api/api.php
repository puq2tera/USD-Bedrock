<?php
/**
 * Simple PHP API for Bedrock Starter
 */

declare(strict_types=1);

require __DIR__ . '/vendor/autoload.php';
use Expensify\Bedrock\Client;
use Monolog\Logger;
use Monolog\Handler\SyslogHandler;
use BedrockStarter\Log;

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

function callBedrock(string $method, array $data = []): array {
    $pluginLogger = new Logger("BedrockStarterPlugin");
    $pluginSyslogHandler = new SyslogHandler("bedrock-starter-plugin");
    $pluginLogger->pushHandler($pluginSyslogHandler);
    $client = Client::getInstance([
        'clusterName' => 'bedrock-starter',
        'mainHostConfigs' => ['127.0.0.1' => ['port' => 8888]],
        'failoverHostConfigs' => ['127.0.0.1' => ['port' => 8888]],
        'connectionTimeout' => 1,
        'readTimeout' => 300,
        'maxBlackListTimeout' => 60,
        'logger' => $pluginLogger,
        'commandPriority' => Client::PRIORITY_NORMAL,
        'bedrockTimeout' => 300,
        'writeConsistency' => 'ASYNC',
        'logParam' =>  null,
        'stats' => null,
    ]);

    try {
        Log::info("Calling bedrock method {$method}", ['data' => $data]);
        $response = $client->call($method, $data);
        if (isset($response["code"]) && $response["code"] == 200) {
            // Bedrock returns data in 'headers' for commands that set response headers
            // and in 'body' for commands that return content
            if (isset($response['headers']) && !empty($response['headers'])) {
                Log::info("Bedrock response headers received for {$method}", ['headers' => $response['headers']]);
                return $response['headers'];
            }
            if (isset($response['body']) && !empty($response['body'])) {
                Log::info("Bedrock response body received for {$method}", ['body' => $response['body']]);
                return $response['body'];
            }
            return [];
        } else {
            // Try to parse status code from error message
            $statusCode = isset($response['codeLine']) ? intval($response['codeLine']) : 500;
            if ($statusCode > 0) {
                http_response_code($statusCode);
            }

            Log::error("Received error response from Bedrock for {$method}", ['response' => $response]);
            return ['error' => $response['codeLine'] ?? 'Unknown error'];
        }
    } catch (\Exception $exception) {
        Log::error("Exception while calling Bedrock method {$method}", ['exception' => $exception->getMessage()]);
        return ["error" => "Error connecting to Bedrock", "message" => $exception->getMessage()];
    }
}

function readRequestData(): array {
    if (!empty($_POST)) {
        return $_POST;
    }

    $rawInput = file_get_contents('php://input');
    if ($rawInput === false || $rawInput === '') {
        return [];
    }

    $decoded = json_decode($rawInput, true);
    return is_array($decoded) ? $decoded : [];
}


// Simple routing
$path = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$method = $_SERVER['REQUEST_METHOD'];

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
        $name = $_GET['name'] ?? $_POST['name'] ?? 'World';

        echo json_encode(callBedrock("HelloWorld", ["name" => $name]));
        break;

    case '/api/messages':
        if ($method === 'GET') {
            $limit = isset($_GET['limit']) ? (int) $_GET['limit'] : null;
            if ($limit !== null) {
                $limit = max(1, min(100, $limit));
            }

            $params = [];
            if ($limit !== null) {
                $params['limit'] = (string) $limit;
            }

            $bedrockResponse = callBedrock("GetMessages", $params);
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
            $data = readRequestData();
            $name = trim((string) ($data['name'] ?? ''));
            $message = trim((string) ($data['message'] ?? ''));

            if ($name === '' || $message === '') {
                http_response_code(400);
                echo json_encode(['error' => 'Both name and message are required']);
                break;
            }

            echo json_encode(callBedrock("CreateMessage", [
                'name' => $name,
                'message' => $message,
            ]));
            break;
        }

        http_response_code(405);
        echo json_encode(['error' => 'Method not allowed', 'allowed' => ['GET', 'POST']]);
        break;

    default:
        http_response_code(404);
        echo json_encode(['error' => 'Endpoint not found']);
        break;
}
