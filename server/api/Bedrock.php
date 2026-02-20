<?php

declare(strict_types=1);

namespace BedrockStarter;

use Expensify\Bedrock\Client;
use Monolog\Handler\SyslogHandler;
use Monolog\Logger;

class Bedrock
{
    private static ?Client $instance = null;

    /**
     * Get singleton Bedrock client instance
     */
    public static function getInstance(): Client
    {
        if (self::$instance === null) {
            $pluginLogger = new Logger("BedrockStarterPlugin");
            $pluginLogger->pushHandler(new SyslogHandler("bedrock-starter-plugin"));

            self::$instance = Client::getInstance([
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
                'logParam' => null,
                'stats' => null,
            ]);
        }

        return self::$instance;
    }

    /**
     * Call a Bedrock command and return the response
     */
    public static function call(string $method, array $data = []): array
    {
        $client = self::getInstance();

        try {
            Log::info("Calling Bedrock command {$method}", ['data' => $data]);
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
            }
            $codeLine = (string)($response['codeLine'] ?? 'Unknown Bedrock error');
            $statusCode = intval($codeLine);
            if ($statusCode < 400 || $statusCode > 599) {
                $statusCode = 502;
            }

            $headers = isset($response['headers']) && is_array($response['headers']) ? $response['headers'] : [];
            $errorCode = isset($headers['errorCode']) ? (string)$headers['errorCode'] : '';
            $bodyError = self::extractErrorFromBody($response['body'] ?? null);

            $errorMessage = $codeLine;
            if ($bodyError !== '' && stripos($errorMessage, $bodyError) === false) {
                $errorMessage .= " - {$bodyError}";
            }

            Log::error("Received error response from Bedrock for {$method}", [
                'statusCode' => $statusCode,
                'errorMessage' => $errorMessage,
                'errorCode' => $errorCode,
                'response' => $response,
            ]);
            throw new ValidationException($errorMessage, $statusCode);
        } catch (ValidationException $exception) {
            throw $exception;
        } catch (\Throwable $exception) {
            Log::error("Exception while calling Bedrock method {$method}", ['exception' => $exception->getMessage()]);
            throw new ValidationException('Error connecting to Bedrock', 502);
        }
    }

    private static function extractErrorFromBody(mixed $rawBody): string
    {
        if ($rawBody === null) {
            return '';
        }

        if (is_array($rawBody)) {
            return isset($rawBody['error']) ? (string)$rawBody['error'] : '';
        }

        if (!is_string($rawBody) || trim($rawBody) === '') {
            return '';
        }

        $decoded = json_decode($rawBody, true);
        if (is_array($decoded) && isset($decoded['error'])) {
            return (string)$decoded['error'];
        }

        return '';
    }
}
